#pragma once

#include <zvulkan/vulkanbuilders.h>
#include "../core/widget.h"

#include "sdl2nativehandle.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include "win32nativehandle.h"
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include "x11nativehandle.h"
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include "waylandnativehandle.h"
#endif

inline VulkanInstanceBuilder CreateZVulkanInstanceBuilder(Widget* widget)
{
	if (DisplayBackend::Get()->IsSDL2())
	{
		auto handle = (SDL2NativeHandle*)widget->GetNativeHandle();
		return VulkanInstanceBuilder().RequireExtensions(handle->VulkanGetInstanceExtensions());
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	if (DisplayBackend::Get()->IsWin32())
	{
		return VulkanInstanceBuilder().RequireWin32Surface();
	}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
	if (DisplayBackend::Get()->IsX11())
	{
		return VulkanInstanceBuilder().RequireX11Surface();
	}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	if (DisplayBackend::Get()->IsWayland())
	{
		return VulkanInstanceBuilder().RequireWaylandSurface();
	}
#endif

	VulkanError("Could not create vulkan instance for widget");
	return {};
}

inline std::shared_ptr<VulkanSurface> CreateZVulkanSurface(Widget* widget, std::shared_ptr<VulkanInstance> instance)
{
	if (DisplayBackend::Get()->IsSDL2())
	{
		auto handle = (SDL2NativeHandle*)widget->GetNativeHandle();
		return std::make_shared<VulkanSurface>(instance, handle->VulkanCreateSurface(instance->Instance));
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	if (DisplayBackend::Get()->IsWin32())
	{
		auto handle = (Win32NativeHandle*)widget->GetNativeHandle();
		return std::make_shared<VulkanSurface>(instance, handle->hwnd);
	}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
	if (DisplayBackend::Get()->IsX11())
	{
		auto handle = (X11NativeHandle*)widget->GetNativeHandle();
		return std::make_shared<VulkanSurface>(instance, handle->display, handle->window);
	}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	if (DisplayBackend::Get()->IsWayland())
	{
		auto handle = (WaylandNativeHandle*)widget->GetNativeHandle();
		return std::make_shared<VulkanSurface>(instance, handle->display, handle->surface);
	}
#endif

	VulkanError("Could not create vulkan surface for widget");
	return {};
}
