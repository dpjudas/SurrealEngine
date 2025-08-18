#pragma once

#include <zvulkan/vulkanobjects.h>

class VulkanRenderDevice;

class SamplerManager
{
public:
	SamplerManager(VulkanRenderDevice* renderer);

	void CreateSceneSamplers();

	std::unique_ptr<VulkanSampler> Samplers[4];

	std::unique_ptr<VulkanSampler> PPNearestRepeat;
	std::unique_ptr<VulkanSampler> PPLinearClamp;

	FLOAT LODBias = 0.0f;

private:
	VulkanRenderDevice* renderer = nullptr;
};
