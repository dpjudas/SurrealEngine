#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_USE_PLATFORM_METAL_EXT
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include "volk/volk.h"
#include "vk_mem_alloc/vk_mem_alloc.h"
#include "ShaderCompiler/glslang/Public/ShaderLang.h"
#include "ShaderCompiler/spirv/GlslangToSpv.h"

#include <functional>

class VulkanDeviceFeatures
{
public:
	VkPhysicalDeviceFeatures Features = {};
	VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddress = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
	VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	VkPhysicalDeviceRayQueryFeaturesKHR RayQuery = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
	VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
};

class VulkanPhysicalDevice
{
public:
	VkPhysicalDevice Device = VK_NULL_HANDLE;
	std::vector<VkExtensionProperties> Extensions;
	std::vector<VkQueueFamilyProperties> QueueFamilies;
	VkPhysicalDeviceProperties Properties = {};
	VkPhysicalDeviceMemoryProperties MemoryProperties = {};
	VulkanDeviceFeatures Features;
};

class VulkanInstance
{
public:
	VulkanInstance(bool wantDebugLayer);
	~VulkanInstance();

	std::vector<const char*> RequiredExtensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
	};
	std::vector<const char*> OptionalExtensions =
	{
		VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};
	std::vector<uint32_t> ApiVersionsToTry = { VK_API_VERSION_1_2, VK_API_VERSION_1_1, VK_API_VERSION_1_0 };

	std::vector<VkLayerProperties> AvailableLayers;
	std::vector<VkExtensionProperties> AvailableExtensions;

	std::vector<const char*> EnabledValidationLayers;
	std::vector<const char*> EnabledExtensions;

	std::vector<VulkanPhysicalDevice> PhysicalDevices;

	uint32_t ApiVersion = {};
	VkInstance Instance = VK_NULL_HANDLE;

	bool DebugLayerActive = false;

private:
	bool WantDebugLayer = false;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	void CreateInstance();
	void ReleaseResources();

	static void InitVolk();
	static std::vector<VkLayerProperties> GetAvailableLayers();
	static std::vector<VkExtensionProperties> GetExtensions();
	static std::vector<VulkanPhysicalDevice> GetPhysicalDevices(VkInstance instance, uint32_t apiVersion);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	static std::vector<std::string> SplitString(const std::string& s, const std::string& seperator);
};

std::string VkResultToString(VkResult result);

void VulkanPrintLog(const char* typestr, const std::string& msg);
void VulkanError(const char* text);

inline void CheckVulkanError(VkResult result, const char* text)
{
	if (result >= VK_SUCCESS) return;
	VulkanError((text + std::string(": ") + VkResultToString(result)).c_str());
}
