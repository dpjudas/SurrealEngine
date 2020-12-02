#pragma once

#include "VulkanObjects.h"

class SceneSamplers
{
public:
	SceneSamplers(VulkanDevice *device);

	std::unique_ptr<VulkanSampler> samplers[4];
};
