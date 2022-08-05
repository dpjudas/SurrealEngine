#pragma once

#include "VulkanInstance.h"

class VulkanSurface
{
public:
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, HWND window);
	~VulkanSurface();

	std::shared_ptr<VulkanInstance> Instance;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	HWND Window = 0;
};
