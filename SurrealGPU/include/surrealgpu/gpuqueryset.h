#pragma once

#include <string>
#include "vulkanobjects.h"

class GPUDevice;

enum class GPUQuerySetType
{
	occlusion,
	timestamp
};

class GPUQuerySetDesc
{
public:
	int count = 0;
	GPUQuerySetType type = {};
	std::string label;
};

class GPUQuerySet
{
public:
	GPUQuerySet(GPUDevice* device, const GPUQuerySetDesc& desc);
	void destroy();

	GPUDevice* device = nullptr;
	GPUQuerySetDesc desc;

	std::unique_ptr<VulkanQueryPool> queryPool;
};
