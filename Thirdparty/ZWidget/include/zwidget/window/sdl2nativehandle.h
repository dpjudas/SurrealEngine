#pragma once

#include <string>
#include <vector>

struct SDL_Window;

#ifndef VULKAN_H_

#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)

#endif

class SDL2NativeHandle
{
public:
	SDL_Window* window = nullptr;

	std::vector<std::string> VulkanGetInstanceExtensions();
	VkSurfaceKHR VulkanCreateSurface(VkInstance instance);
};
