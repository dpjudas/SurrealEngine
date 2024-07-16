
#include "vulkansurface.h"
#include "vulkaninstance.h"

#if defined(WIN32)
#define NOMINMAX
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#endif

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, VkSurfaceKHR surface) : Instance(std::move(instance)), Surface(surface)
{
}

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(Instance->Instance, Surface, nullptr);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, HWND window) : Instance(std::move(instance)), Window(window)
{
	VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hwnd = window;
	createInfo.hinstance = GetModuleHandle(nullptr);

	VkResult result = vkCreateWin32SurfaceKHR(Instance->Instance, &createInfo, nullptr, &Surface);
	if (result != VK_SUCCESS)
		VulkanError("Could not create vulkan surface");
}

#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, Display* display, Window window) : Instance(std::move(instance)), X11Display(display), X11Window(window)
{
	VkWaylandSurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR };
	createInfo.display = display;
	createInfo.window = window;

	VkResult result = vkCreateXlibSurfaceKHR(Instance->Instance, &createInfo, nullptr, &Surface);
	if (result != VK_SUCCESS)
		VulkanError("Could not create vulkan surface");
}

#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, wl_display* display, wl_surface* surface) : Instance(std::move(instance)), WLDisplay(display), WLWindow(window)
{
	VkWaylandSurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
	createInfo.display = display;
	createInfo.window = window;

	VkResult result = vkCreateWaylandSurfaceKHR(Instance->Instance, &createInfo, nullptr, &Surface);
	if (result != VK_SUCCESS)
		VulkanError("Could not create vulkan surface");
}

#endif
