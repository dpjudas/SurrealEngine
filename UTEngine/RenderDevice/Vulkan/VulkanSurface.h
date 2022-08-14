#pragma once

#include "VulkanInstance.h"

#ifdef VK_USE_PLATFORM_WIN32_KHR

class VulkanSurface
{
public:
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, HWND window);
	~VulkanSurface();

	std::shared_ptr<VulkanInstance> Instance;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	HWND Window = 0;
};

#elif defined(VK_USE_PLATFORM_XLIB_KHR)

#include <X11/Xlib.h>

class VulkanSurface
{
public:
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, Display* disp, Window wind);
	~VulkanSurface();

	std::shared_ptr<VulkanInstance> Instance;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;
	Display* disp = nullptr;
	Window wind;
};

#endif
