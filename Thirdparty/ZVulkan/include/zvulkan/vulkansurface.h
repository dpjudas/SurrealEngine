#pragma once

#include "vulkaninstance.h"

class VulkanSurface
{
public:
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, VkSurfaceKHR surface);
	~VulkanSurface();

	std::shared_ptr<VulkanInstance> Instance;
	VkSurfaceKHR Surface = VK_NULL_HANDLE;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, HWND window);
	HWND Window = 0;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, Display* display, Window window);
	Display* X11Display = nullptr;
	Window X11Window = 0;
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	VulkanSurface(std::shared_ptr<VulkanInstance> instance, wl_display* display, wl_surface* surface);
	wl_display* WLDisplay = nullptr;
	wl_surface* WLSurface = nullptr;
#endif
};
