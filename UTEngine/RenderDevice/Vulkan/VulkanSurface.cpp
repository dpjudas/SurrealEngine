
#include "Precomp.h"
#include "VulkanSurface.h"
#include "VulkanInstance.h"

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

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(Instance->Instance, Surface, nullptr);
}

#elif defined(VK_USE_PLATFORM_XLIB_KHR)

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, Display* disp, Window wind) : Instance(std::move(instance)), disp(disp), wind(wind)
{
	VkXlibSurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR };
	createInfo.dpy = disp;
	createInfo.window = wind;

	VkResult result = vkCreateXlibSurfaceKHR(Instance->Instance, &createInfo, nullptr, &Surface);
	if (result != VK_SUCCESS)
		VulkanError("Could not create vulkan surface");
}

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(Instance->Instance, Surface, nullptr);
}

#endif
