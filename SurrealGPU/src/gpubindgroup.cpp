
#include "gpubindgroup.h"
#include "gpudevice.h"
#include "gpubuffer.h"
#include "gputexture.h"
#include "vulkanbuilders.h"

GPUBindGroup::GPUBindGroup(GPUDevice* device, const GPUBindGroupDesc& desc) : device(device), desc(desc)
{
	descriptorSet = desc.layout->pool->allocate(desc.layout->layout.get());

	struct WriteExtra
	{
		VkDescriptorImageInfo imageInfo;
		VkDescriptorBufferInfo bufferInfo;
		VkBufferView bufferView;
		VkWriteDescriptorSetAccelerationStructureKHR accelStruct;
	};

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<std::unique_ptr<WriteExtra>> writeExtras;

	int index = 0;
	const auto& layoutBindings = desc.layout->desc.bindings;
	for (const GPUBindGroupBinding& binding : desc.bindings)
	{
		const GPUBindGroupLayoutBinding& bindingInfo = layoutBindings[index++];
		if (bindingInfo.type == GPUBindGroupLayoutBindingType::buffer)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = binding.resource.buffer.buffer->buffer->buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = binding.resource.buffer.size;

			auto extra = std::make_unique<WriteExtra>();
			extra->bufferInfo = bufferInfo;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet->set;
			descriptorWrite.dstBinding = binding.bindingIndex;
			descriptorWrite.dstArrayElement = 0;
			if (bindingInfo.buffer.type == GPUBindGroupLayoutBufferType::uniform)
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			else if (bindingInfo.buffer.type == GPUBindGroupLayoutBufferType::readOnlyStorage)
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			else if (bindingInfo.buffer.type == GPUBindGroupLayoutBufferType::storage)
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &extra->bufferInfo;
			writes.push_back(descriptorWrite);
			writeExtras.push_back(std::move(extra));
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::combinedTextureSampler)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageView = binding.resource.textureView->view->view;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = binding.resource.sampler->sampler->sampler;

			auto extra = std::make_unique<WriteExtra>();
			extra->imageInfo = imageInfo;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet->set;
			descriptorWrite.dstBinding = binding.bindingIndex;
			descriptorWrite.dstArrayElement = binding.arrayIndex;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &extra->imageInfo;
			writes.push_back(descriptorWrite);
			writeExtras.push_back(std::move(extra));
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::texture)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageView = binding.resource.textureView->view->view;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			auto extra = std::make_unique<WriteExtra>();
			extra->imageInfo = imageInfo;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet->set;
			descriptorWrite.dstBinding = binding.bindingIndex;
			descriptorWrite.dstArrayElement = binding.arrayIndex;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &extra->imageInfo;
			writes.push_back(descriptorWrite);
			writeExtras.push_back(std::move(extra));
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::sampler)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.sampler = binding.resource.sampler->sampler->sampler;

			auto extra = std::make_unique<WriteExtra>();
			extra->imageInfo = imageInfo;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet->set;
			descriptorWrite.dstBinding = binding.bindingIndex;
			descriptorWrite.dstArrayElement = binding.arrayIndex;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &extra->imageInfo;
			writes.push_back(descriptorWrite);
			writeExtras.push_back(std::move(extra));
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::storageTexture)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageView = binding.resource.textureView->view->view;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			auto extra = std::make_unique<WriteExtra>();
			extra->imageInfo = imageInfo;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet->set;
			descriptorWrite.dstBinding = binding.bindingIndex;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &extra->imageInfo;
			writes.push_back(descriptorWrite);
			writeExtras.push_back(std::move(extra));
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::externalTexture)
		{
		}
	}

	if (!writes.empty())
		vkUpdateDescriptorSets(device->getVulkanDevice()->device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

/////////////////////////////////////////////////////////////////////////////

GPUBindGroupLayout::GPUBindGroupLayout(GPUDevice* device, const GPUBindGroupLayoutDesc& desc) : device(device), desc(desc)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT };
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorBindingFlags> bindingFlags;

	// layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	for (const GPUBindGroupLayoutBinding& bindingInfo : desc.bindings)
	{
		// AddBinding(int index, VkDescriptorType type, int arrayCount, VkShaderStageFlags stageFlags, VkDescriptorBindingFlags flags)

		VkDescriptorBindingFlags flags = 0; // VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		VkDescriptorSetLayoutBinding binding = { };
		binding.binding = bindingInfo.bindingIndex;
		binding.descriptorCount = bindingInfo.arrayCount;

		// To do: stageFlags needs to know where we want to use the binding

		if (bindingInfo.type == GPUBindGroupLayoutBindingType::buffer)
		{
			if (bindingInfo.buffer.type == GPUBindGroupLayoutBufferType::uniform)
			{
				if (bindingInfo.buffer.hasDynamicOffset)
				{
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					binding.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else
				{
					binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					binding.stageFlags = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
			}
			else
			{
				if (bindingInfo.buffer.hasDynamicOffset)
				{
					binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
					binding.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else
				{
					binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					binding.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
			}
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::combinedTextureSampler)
		{
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::texture)
		{
			binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			binding.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::sampler)
		{
			binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			binding.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::storageTexture)
		{
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			binding.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (bindingInfo.type == GPUBindGroupLayoutBindingType::externalTexture)
		{
			continue;
		}

		binding.pImmutableSamplers = nullptr;
		bindings.push_back(binding);
		bindingFlags.push_back(flags);

		bindingFlagsInfo.bindingCount = (uint32_t)bindings.size();
		bindingFlagsInfo.pBindingFlags = bindingFlags.data();

		if (flags != 0)
			layoutInfo.pNext = &bindingFlagsInfo;
	}

	layoutInfo.bindingCount = (uint32_t)bindings.size();
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout vklayout;
	VkResult result = vkCreateDescriptorSetLayout(device->getVulkanDevice()->device, &layoutInfo, nullptr, &vklayout);
	device->getVulkanDevice()->CheckVulkanError(result, "Could not create descriptor set layout");
	layout = std::make_unique<VulkanDescriptorSetLayout>(device->getVulkanDevice(), vklayout);
	if (!desc.label.empty())
		layout->SetDebugName(desc.label.c_str());

	// To do: deal with this
	DescriptorPoolBuilder poolBuilder;
	pool = poolBuilder.Create(device->getVulkanDevice());
}
