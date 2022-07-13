#pragma once

#include <functional>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef __APPLE__
#define VK_USE_PLATFORM_MACOS_MVK
#define VK_USE_PLATFORM_METAL_EXT
#endif

#ifdef WIN32
#include <Windows.h>
#undef min
#undef max
#endif

#include "volk/volk.h"
#include "vk_mem_alloc/vk_mem_alloc.h"
#include "ShaderCompiler/glslang/Public/ShaderLang.h"
#include "ShaderCompiler/spirv/GlslangToSpv.h"

class VulkanSwapChain;
class VulkanSemaphore;
class VulkanFence;

class VulkanPhysicalDevice
{
public:
	VkPhysicalDevice device = VK_NULL_HANDLE;

	std::vector<VkExtensionProperties> extensions;
	std::vector<VkQueueFamilyProperties> queueFamilies;
	VkPhysicalDeviceProperties properties = {};
	VkPhysicalDeviceFeatures features = {};
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
};

class VulkanCompatibleDevice
{
public:
	VulkanPhysicalDevice *device = nullptr;
	int graphicsFamily = -1;
	int presentFamily = -1;
};

class VulkanDevice
{
public:
#ifdef WIN32
	VulkanDevice(HWND window, int vk_device = 0, bool vk_debug = false, std::function<void(const char* typestr, const std::string& msg)> printLogCallback = {});
#else
	VulkanDevice(std::function<std::pair<Display*,Window>(VkPhysicalDevice physDevice, uint32_t queueFamilyIndex)> createSurfaceWindow, int vk_device = 0, bool vk_debug = false, std::function<void(const char* typestr, const std::string& msg)> printLogCallback = {});
#endif
	~VulkanDevice();

	void setDebugObjectName(const char *name, uint64_t handle, VkObjectType type)
	{
		if (!debugLayerActive) return;

		VkDebugUtilsObjectNameInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.objectHandle = handle;
		info.objectType = type;
		info.pObjectName = name;
		vkSetDebugUtilsObjectNameEXT(device, &info);
	}

#ifdef WIN32
	HWND window;
#else
	std::function<std::pair<Display*,Window>(VkPhysicalDevice physDevice, uint32_t queueFamilyIndex)> createSurfaceWindow;
#endif

	// Instance setup
	std::vector<VkLayerProperties> availableLayers;
	std::vector<VkExtensionProperties> extensions;
	std::vector<const char *> enabledExtensions;
	std::vector<const char *> optionalExtensions = { VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME };
	std::vector<const char*> enabledValidationLayers;
	uint32_t apiVersion = {};

	// Device setup
	VkPhysicalDeviceFeatures enabledDeviceFeatures = {};
	std::vector<const char *> enabledDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> optionalDeviceExtensions =
	{
		VK_EXT_HDR_METADATA_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_RAY_QUERY_EXTENSION_NAME
	};
	VulkanPhysicalDevice physicalDevice;
	bool debugLayerActive = false;

	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;

	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;

	int graphicsFamily = -1;
	int presentFamily = -1;

	// Physical device info
	std::vector<VulkanPhysicalDevice> availableDevices;
	std::vector<VulkanCompatibleDevice> supportedDevices;

	static void initVolk();

	bool supportsDeviceExtension(const char* ext) const;

private:
	int vk_device;
	bool vk_debug;
	std::function<void(const char* typestr, const std::string& msg)> printLogCallback;

	void createInstance();
	void createSurface();
	void selectPhysicalDevice();
	void selectFeatures();
	void createDevice();
	void createAllocator();
	void releaseResources();

	static bool checkRequiredFeatures(const VkPhysicalDeviceFeatures &f);

	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	static std::vector<VkLayerProperties> getAvailableLayers();
	static std::vector<VkExtensionProperties> getExtensions();
	static std::vector<const char *> getPlatformExtensions();
	static std::vector<VulkanPhysicalDevice> getPhysicalDevices(VkInstance instance);
};
