
#include "Precomp.h"
#include "DescriptorSetManager.h"
#include "VulkanRenderDevice.h"
#include <zvulkan/vulkanbuilders.h>
#include "CachedTexture.h"
#include "UObject/ULevel.h"

DescriptorSetManager::DescriptorSetManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	CreateBindlessSceneDescriptorSet();
	CreateSceneDescriptorSetLayout();
	CreatePresentDescriptorSetLayout();
	CreatePresentDescriptorSet();
}

DescriptorSetManager::~DescriptorSetManager()
{
}

VulkanDescriptorSet* DescriptorSetManager::GetTextureDescriptorSet(uint32_t PolyFlags, CachedTexture* tex, CachedTexture* lightmap, CachedTexture* macrotex, CachedTexture* detailtex, bool clamp)
{
	uint32_t samplermode = 0;
	if (PolyFlags & PF_NoSmooth) samplermode |= 1;
	if (clamp) samplermode |= 2;

	auto& descriptorSet = TextureDescriptorSets[{ tex, lightmap, detailtex, macrotex, samplermode }];
	if (!descriptorSet)
	{
		if (SceneDescriptorPoolSetsLeft == 0)
		{
			SceneDescriptorPool.push_back(DescriptorPoolBuilder()
				.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 * 4)
				.MaxSets(1000)
				.DebugName("SceneDescriptorPool")
				.Create(renderer->Device.get()));
			SceneDescriptorPoolSetsLeft = 1000;
		}

		descriptorSet = SceneDescriptorPool.back()->allocate(SceneDescriptorSetLayout.get());
		SceneDescriptorPoolSetsLeft--;

		WriteDescriptors writes;
		int i = 0;
		for (CachedTexture* texture : { tex, lightmap, macrotex, detailtex })
		{
			VulkanSampler* sampler = (i == 0) ? renderer->Samplers->Samplers[samplermode].get() : renderer->Samplers->Samplers[0].get();

			if (texture)
				writes.AddCombinedImageSampler(descriptorSet.get(), i++, texture->imageView.get(), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			else
				writes.AddCombinedImageSampler(descriptorSet.get(), i++, renderer->Textures->NullTextureView.get(), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		writes.Execute(renderer->Device.get());
	}
	return descriptorSet.get();
}

void DescriptorSetManager::ClearCache()
{
	TextureDescriptorSets.clear();

	SceneDescriptorPool.clear();
	SceneDescriptorPoolSetsLeft = 0;

	WriteBindless = WriteDescriptors();
	NextBindlessIndex = 0;
}

int DescriptorSetManager::GetTextureArrayIndex(uint32_t PolyFlags, CachedTexture* tex, bool clamp)
{
	if (!tex)
		return 0;

	uint32_t samplermode = 0;
	if (PolyFlags & PF_NoSmooth) samplermode |= 1;
	if (clamp) samplermode |= 2;

	int index = tex->BindlessIndex[samplermode];
	if (index != -1)
		return index;

	index = NextBindlessIndex++;

	VulkanSampler* sampler = renderer->Samplers->Samplers[samplermode].get();
	WriteBindless.AddCombinedImageSampler(SceneBindlessDescriptorSet.get(), 0, index, tex->imageView.get(), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	tex->BindlessIndex[samplermode] = index;
	return index;
}

void DescriptorSetManager::UpdateBindlessDescriptorSet()
{
	WriteBindless.Execute(renderer->Device.get());
	WriteBindless = WriteDescriptors();
}

void DescriptorSetManager::CreateBindlessSceneDescriptorSet()
{
	if (!renderer->SupportsBindless)
		return;

	SceneBindlessDescriptorPool = DescriptorPoolBuilder()
		.Flags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MaxBindlessTextures)
		.MaxSets(MaxBindlessTextures)
		.DebugName("SceneBindlessDescriptorPool")
		.Create(renderer->Device.get());

	SceneBindlessDescriptorSetLayout = DescriptorSetLayoutBuilder()
		.Flags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)
		.AddBinding(
			0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			MaxBindlessTextures,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
		.DebugName("SceneBindlessDescriptorSetLayout")
		.Create(renderer->Device.get());

	SceneBindlessDescriptorSet = SceneBindlessDescriptorPool->allocate(SceneBindlessDescriptorSetLayout.get(), MaxBindlessTextures);
}

void DescriptorSetManager::CreateSceneDescriptorSetLayout()
{
	SceneDescriptorSetLayout = DescriptorSetLayoutBuilder()
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
		.DebugName("SceneDescriptorSetLayout")
		.Create(renderer->Device.get());
}

void DescriptorSetManager::CreatePresentDescriptorSetLayout()
{
	PresentDescriptorSetLayout = DescriptorSetLayoutBuilder()
		.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
		.DebugName("PresentDescriptorSetLayout")
		.Create(renderer->Device.get());
}

void DescriptorSetManager::CreatePresentDescriptorSet()
{
	PresentDescriptorPool = DescriptorPoolBuilder()
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
		.MaxSets(1)
		.DebugName("PresentDescriptorPool")
		.Create(renderer->Device.get());
	PresentDescriptorSet = PresentDescriptorPool->allocate(PresentDescriptorSetLayout.get());
}
