
#include "gpuqueryset.h"
#include "gpudevice.h"
#include "vulkanbuilders.h"

GPUQuerySet::GPUQuerySet(GPUDevice* device, const GPUQuerySetDesc& desc) : device(device), desc(desc)
{
	VkQueryPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	poolInfo.queryType = (desc.type == GPUQuerySetType::occlusion) ? VK_QUERY_TYPE_OCCLUSION : VK_QUERY_TYPE_TIMESTAMP;
	poolInfo.queryCount = desc.count;
	poolInfo.pipelineStatistics = 0;

	VkQueryPool vkqueryPool;
	VkResult result = vkCreateQueryPool(device->getVulkanDevice()->device, &poolInfo, nullptr, &vkqueryPool);
	device->getVulkanDevice()->CheckVulkanError(result, "Could not create query pool");
	queryPool = std::make_unique<VulkanQueryPool>(device->getVulkanDevice(), vkqueryPool);
	if (!desc.label.empty())
		queryPool->SetDebugName(desc.label.c_str());
}

void GPUQuerySet::destroy()
{
}
