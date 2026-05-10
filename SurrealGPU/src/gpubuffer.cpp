
#include "gpubuffer.h"
#include "gpucommands.h"
#include "gpudevice.h"
#include "vulkanbuilders.h"

GPUBuffer::GPUBuffer(GPUDevice* device, const GPUBufferDesc& desc) : device(device), desc(desc)
{
	VkDeviceSize minAlignment = 0;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = std::max(desc.size, (size_t)16);

	// To do: apply this based on desc.usage
	bufferInfo.usage =
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.flags = 0;

	// Memory type:
	//allocInfo.requiredFlags = requiredFlags;
	//allocInfo.preferredFlags = preferredFlags;
	//allocInfo.memoryTypeBits = memoryTypeBits;

	VkBuffer vkbuffer;
	VmaAllocation allocation;

	if (minAlignment == 0)
	{
		VkResult result = vmaCreateBuffer(device->getVulkanDevice()->allocator, &bufferInfo, &allocInfo, &vkbuffer, &allocation, nullptr);
		device->getVulkanDevice()->CheckVulkanError(result, "Could not allocate memory for vulkan buffer");
	}
	else
	{
		VkResult result = vmaCreateBufferWithAlignment(device->getVulkanDevice()->allocator, &bufferInfo, &allocInfo, minAlignment, &vkbuffer, &allocation, nullptr);
		device->getVulkanDevice()->CheckVulkanError(result, "Could not allocate memory for vulkan buffer");
	}

	buffer = std::make_unique<VulkanBuffer>(device->getVulkanDevice(), vkbuffer, allocation, (size_t)bufferInfo.size);
	if (!desc.label.empty())
		buffer->SetDebugName(desc.label.c_str());
}

GPUPromise GPUBuffer::mapAsync()
{
	return {};
}

void GPUBuffer::unmap()
{
}

void GPUBuffer::destroy()
{
}
