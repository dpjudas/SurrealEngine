
#include "Precomp.h"
#include "VulkanDescriptorSet.h"
#include "VulkanRenderDevice.h"
#include "VulkanSampler.h"
#include "VulkanBuilders.h"
#include "VulkanTexture.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRaytrace.h"
#include "VulkanLight.h"
#include "UObject/ULevel.h"

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	CreateTextureSetLayout();
	CreateLightSet();
	CreateNullTexture();
}

VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
{
}

VulkanDescriptorSet* VulkanDescriptorSetManager::GetTextureSet(uint32_t PolyFlags, VulkanTexture* tex, VulkanTexture* lightmap, VulkanTexture* macrotex, VulkanTexture* detailtex, bool clamp)
{
	uint32_t samplermode = 0;
	if (PolyFlags & PF_NoSmooth) samplermode |= 1;
	if (clamp) samplermode |= 2;

	auto& descriptorSet = TextureDescriptorSets[{ tex, lightmap, detailtex, macrotex, samplermode }];
	if (!descriptorSet)
	{
		if (TextureSetPoolSetsLeft == 0)
		{
			DescriptorPoolBuilder builder;
			builder.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 * 4);
			builder.setMaxSets(1000);
			TextureSetPool.push_back(builder.create(renderer->Device));
			TextureSetPool.back()->SetDebugName("TextureSetPool");
			TextureSetPoolSetsLeft = 1000;
		}

		descriptorSet = TextureSetPool.back()->allocate(TextureSetLayout.get());
		descriptorSet->SetDebugName("MaterialDescriptorSet");
		TextureSetPoolSetsLeft--;

		WriteDescriptors writes;
		int i = 0;
		for (VulkanTexture* texture : { tex, lightmap, macrotex, detailtex })
		{
			VulkanSampler* sampler = (i == 0) ? renderer->Samplers->samplers[samplermode].get() : renderer->Samplers->samplers[0].get();

			if (texture)
				writes.addCombinedImageSampler(descriptorSet.get(), i++, texture->imageView.get(), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			else
				writes.addCombinedImageSampler(descriptorSet.get(), i++, NullTextureView.get(), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		writes.updateSets(renderer->Device);
	}
	return descriptorSet.get();
}

void VulkanDescriptorSetManager::ClearTextureDescriptors()
{
	TextureDescriptorSets.clear();
	TextureSetPool.clear();
	TextureSetPoolSetsLeft = 0;
}

void VulkanDescriptorSetManager::UpdateLightSet()
{
	WriteDescriptors writes;
	writes.addAccelerationStructure(LightSet.get(), 0, renderer->Raytrace->GetAccelStruct());
	writes.addBuffer(LightSet.get(), 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, renderer->Lights->Lights.get());
	writes.addBuffer(LightSet.get(), 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, renderer->Lights->SurfaceLights.get());
	writes.updateSets(renderer->Device);
}

void VulkanDescriptorSetManager::CreateLightSet()
{
	DescriptorSetLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	builder.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	LightSetLayout = builder.create(renderer->Device);

	DescriptorPoolBuilder poolbuilder;
	poolbuilder.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
	poolbuilder.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);
	poolbuilder.setMaxSets(1);
	LightSetPool = poolbuilder.create(renderer->Device);
	LightSetPool->SetDebugName("LightSetPool");

	LightSet = LightSetPool->allocate(LightSetLayout.get());
}

void VulkanDescriptorSetManager::CreateTextureSetLayout()
{
	DescriptorSetLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	builder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	builder.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	TextureSetLayout = builder.create(renderer->Device);
}

void VulkanDescriptorSetManager::CreateNullTexture()
{
	auto cmdbuffer = renderer->Commands->GetTransferCommands();

	ImageBuilder imgbuilder;
	imgbuilder.setFormat(VK_FORMAT_R8G8B8A8_UNORM);
	imgbuilder.setSize(1, 1);
	imgbuilder.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	NullTexture = imgbuilder.create(renderer->Device);

	ImageViewBuilder viewbuilder;
	viewbuilder.setImage(NullTexture.get(), VK_FORMAT_R8G8B8A8_UNORM);
	NullTextureView = viewbuilder.create(renderer->Device);

	BufferBuilder builder;
	builder.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	builder.setSize(4);
	auto stagingbuffer = builder.create(renderer->Device);
	auto data = (uint32_t*)stagingbuffer->Map(0, 4);
	data[0] = 0xffffffff;
	stagingbuffer->Unmap();

	PipelineBarrier imageTransition0;
	imageTransition0.addImage(NullTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
	imageTransition0.execute(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { 1, 1, 1 };
	cmdbuffer->copyBufferToImage(stagingbuffer->buffer, NullTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	PipelineBarrier imageTransition1;
	imageTransition1.addImage(NullTexture.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	imageTransition1.execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	renderer->Commands->FrameDeleteList->buffers.push_back(std::move(stagingbuffer));
}
