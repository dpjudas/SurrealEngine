
#include "Precomp.h"
#include "VulkanSurface.h"
#include "VulkanInstance.h"

VulkanSurface::VulkanSurface(std::shared_ptr<VulkanInstance> instance, HWND window) : Instance(std::move(instance))
{
	VkWin32SurfaceCreateInfoKHR windowCreateInfo = {};
	windowCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	windowCreateInfo.hwnd = window;
	windowCreateInfo.hinstance = GetModuleHandle(nullptr);

	VkResult result = vkCreateWin32SurfaceKHR(Instance->Instance, &windowCreateInfo, nullptr, &Surface);
	if (result != VK_SUCCESS)
		VulkanError("Could not create vulkan surface");
}

VulkanSurface::~VulkanSurface()
{
	vkDestroySurfaceKHR(Instance->Instance, Surface, nullptr);
}
