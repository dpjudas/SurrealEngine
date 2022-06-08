#pragma once

#include "VulkanObjects.h"

class VulkanSamplerManager
{
public:
	VulkanSamplerManager(VulkanDevice *device);

	std::unique_ptr<VulkanSampler> samplers[4];
};
