#pragma once

#include "VulkanObjects.h"

class VulkanRenderDevice;

class SamplerManager
{
public:
	SamplerManager(VulkanRenderDevice* renderer);

	std::unique_ptr<VulkanSampler> Samplers[4];

	std::unique_ptr<VulkanSampler> PPNearestRepeat;
	std::unique_ptr<VulkanSampler> PPLinearClamp;

private:
	VulkanRenderDevice* renderer = nullptr;
};
