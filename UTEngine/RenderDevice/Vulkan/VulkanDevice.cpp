
#include "Precomp.h"
#include "VulkanDevice.h"
#include "VulkanObjects.h"
#include <algorithm>
#include <set>
#include <string>
#include <mutex>

#ifdef WIN32
VulkanDevice::VulkanDevice(HWND window, int vk_device, bool vk_debug, std::function<void(const char* typestr, const std::string& msg)> printLogCallback) : window(window), vk_device(vk_device), vk_debug(vk_debug), printLogCallback(printLogCallback)
{
	try
	{
		initVolk();
		createInstance();
		if (window)
			createSurface();
		selectPhysicalDevice();
		selectFeatures();
		createDevice();
		createAllocator();
	}
	catch (...)
	{
		releaseResources();
		throw;
	}
}

VulkanDevice::~VulkanDevice()
{
	releaseResources();
}
#else

// To do: port to unix

#endif

void VulkanDevice::selectFeatures()
{
	enabledDeviceFeatures.samplerAnisotropy = physicalDevice.features.samplerAnisotropy;
	enabledDeviceFeatures.fragmentStoresAndAtomics = physicalDevice.features.fragmentStoresAndAtomics;
	enabledDeviceFeatures.depthClamp = physicalDevice.features.depthClamp;
	enabledDeviceFeatures.shaderClipDistance = physicalDevice.features.shaderClipDistance;
}

bool VulkanDevice::checkRequiredFeatures(const VkPhysicalDeviceFeatures &f)
{
	return
		f.samplerAnisotropy == VK_TRUE &&
		f.fragmentStoresAndAtomics == VK_TRUE &&
		f.depthClamp == VK_TRUE;
}

void VulkanDevice::selectPhysicalDevice()
{
	availableDevices = getPhysicalDevices(instance);
	if (availableDevices.empty())
		throw std::runtime_error("No Vulkan devices found. Either the graphics card has no vulkan support or the driver is too old.");

	supportedDevices.clear();

	for (size_t idx = 0; idx < availableDevices.size(); idx++)
	{
		const auto &info = availableDevices[idx];

		if (!checkRequiredFeatures(info.features))
			continue;

		std::set<std::string> requiredExtensionSearch(enabledDeviceExtensions.begin(), enabledDeviceExtensions.end());
		for (const auto &ext : info.extensions)
			requiredExtensionSearch.erase(ext.extensionName);
		if (!requiredExtensionSearch.empty())
			continue;

		VulkanCompatibleDevice dev;
		dev.device = &availableDevices[idx];

		// Figure out what can present
		#ifndef WIN32
		bool window = true;
		#endif
		if (window)
		{
			for (int i = 0; i < (int)info.queueFamilies.size(); i++)
			{
				VkBool32 presentSupport = false;
				VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(info.device, i, surface, &presentSupport);
				if (result == VK_SUCCESS && info.queueFamilies[i].queueCount > 0 && presentSupport)
				{
					dev.presentFamily = i;
					break;
				}
			}
		}

		// The vulkan spec states that graphics and compute queues can always do transfer.
		// Furthermore the spec states that graphics queues always can do compute.
		// Last, the spec makes it OPTIONAL whether the VK_QUEUE_TRANSFER_BIT is set for such queues, but they MUST support transfer.
		//
		// In short: pick the first graphics queue family for everything.
		for (int i = 0; i < (int)info.queueFamilies.size(); i++)
		{
			const auto &queueFamily = info.queueFamilies[i];
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				dev.graphicsFamily = i;
				break;
			}
		}

		if (dev.graphicsFamily != -1 && (!window || dev.presentFamily != -1))
		{
			supportedDevices.push_back(dev);
		}
	}

	if (supportedDevices.empty())
		throw std::runtime_error("No Vulkan device supports the minimum requirements of this application");

	// The device order returned by Vulkan can be anything. Prefer discrete > integrated > virtual gpu > cpu > other
	std::stable_sort(supportedDevices.begin(), supportedDevices.end(), [&](const auto &a, const auto b) {

		// Sort by GPU type first. This will ensure the "best" device is most likely to map to vk_device 0
		static const int typeSort[] = { 4, 1, 0, 2, 3 };
		int sortA = a.device->properties.deviceType < 5 ? typeSort[a.device->properties.deviceType] : (int)a.device->properties.deviceType;
		int sortB = b.device->properties.deviceType < 5 ? typeSort[b.device->properties.deviceType] : (int)b.device->properties.deviceType;
		if (sortA != sortB)
			return sortA < sortB;

		// Then sort by the device's unique ID so that vk_device uses a consistent order
		int sortUUID = memcmp(a.device->properties.pipelineCacheUUID, b.device->properties.pipelineCacheUUID, VK_UUID_SIZE);
		return sortUUID < 0;
	});

	size_t selected = vk_device;
	if (selected >= supportedDevices.size())
		selected = 0;

	// Enable optional extensions we are interested in, if they are available on this device
	for (const auto &ext : supportedDevices[selected].device->extensions)
	{
		for (const auto &opt : optionalDeviceExtensions)
		{
			if (strcmp(ext.extensionName, opt) == 0)
			{
				enabledDeviceExtensions.push_back(opt);
			}
		}
	}

	physicalDevice = *supportedDevices[selected].device;
	graphicsFamily = supportedDevices[selected].graphicsFamily;
	presentFamily = supportedDevices[selected].presentFamily;
}

bool VulkanDevice::supportsDeviceExtension(const char *ext) const
{
	return std::find(enabledDeviceExtensions.begin(), enabledDeviceExtensions.end(), ext) != enabledDeviceExtensions.end();
}

void VulkanDevice::createAllocator()
{
	VmaAllocatorCreateInfo allocinfo = {};
	if (supportsDeviceExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) && supportsDeviceExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME))
		allocinfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	allocinfo.physicalDevice = physicalDevice.device;
	allocinfo.device = device;
	allocinfo.preferredLargeHeapBlockSize = 64 * 1024 * 1024;
	if (vmaCreateAllocator(&allocinfo, &allocator) != VK_SUCCESS)
		throw std::runtime_error("Unable to create allocator");
}

void VulkanDevice::createDevice()
{
	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<int> neededFamilies;
	neededFamilies.insert(graphicsFamily);
	if (presentFamily != -1)
		neededFamilies.insert(presentFamily);

	for (int index : neededFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = index;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &enabledDeviceFeatures;
	deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = 0;

	VkResult result = vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &device);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Could not create vulkan device");

	volkLoadDevice(device);

	vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
	if (presentFamily != -1)
		vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
}

#ifdef WIN32
void VulkanDevice::createSurface()
{
	VkWin32SurfaceCreateInfoKHR windowCreateInfo = {};
	windowCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	windowCreateInfo.hwnd = window;
	windowCreateInfo.hinstance = GetModuleHandle(nullptr);

	VkResult result = vkCreateWin32SurfaceKHR(instance, &windowCreateInfo, nullptr, &surface);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Could not create vulkan surface");
}
#endif

void VulkanDevice::createInstance()
{
	availableLayers = getAvailableLayers();
	extensions = getExtensions();
	enabledExtensions = getPlatformExtensions();

	std::string debugLayer = "VK_LAYER_LUNARG_standard_validation";
	bool wantDebugLayer = vk_debug;
	bool debugLayerFound = false;
	for (const VkLayerProperties &layer : availableLayers)
	{
		if (layer.layerName == debugLayer && wantDebugLayer)
		{
			enabledValidationLayers.push_back(debugLayer.c_str());
			enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			debugLayerFound = true;
		}
	}

	// Enable optional instance extensions we are interested in
	for (const auto &ext : extensions)
	{
		for (const auto &opt : optionalExtensions)
		{
			if (strcmp(ext.extensionName, opt) == 0)
			{
				enabledExtensions.push_back(opt);
			}
		}
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "UT99";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "UT99";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	createInfo.enabledLayerCount = (uint32_t)enabledValidationLayers.size();
	createInfo.ppEnabledLayerNames = enabledValidationLayers.data();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Could not create vulkan instance");

	volkLoadInstance(instance);

	if (debugLayerFound)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = this;
		result = vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
		if (result != VK_SUCCESS)
			throw std::runtime_error("vkCreateDebugUtilsMessengerEXT failed");

		debugLayerActive = true;
	}
}

VkBool32 VulkanDevice::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
{
	VulkanDevice *device = (VulkanDevice*)userData;

	static std::mutex mtx;
	static std::set<std::string> seenMessages;
	static int totalMessages;

	std::unique_lock<std::mutex> lock(mtx);

	std::string msg = callbackData->pMessage;

	bool found = seenMessages.find(msg) != seenMessages.end();
	if (!found)
	{
		if (totalMessages < 20)
		{
			totalMessages++;
			seenMessages.insert(msg);

			const char *typestr;
			if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				typestr = "vulkan error";
			}
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				typestr = "vulkan warning";
			}
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			{
				typestr = "vulkan info";
			}
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			{
				typestr = "vulkan verbose";
			}
			else
			{
				typestr = "vulkan";
			}

			if (device->printLogCallback)
				device->printLogCallback(typestr, msg);
		}
	}

	return VK_FALSE;
}

std::vector<VkLayerProperties> VulkanDevice::getAvailableLayers()
{
	uint32_t layerCount;
	VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	return availableLayers;
}

std::vector<VkExtensionProperties> VulkanDevice::getExtensions()
{
	uint32_t extensionCount = 0;
	VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	return extensions;
}

std::vector<VulkanPhysicalDevice> VulkanDevice::getPhysicalDevices(VkInstance instance)
{
	uint32_t deviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (result == VK_ERROR_INITIALIZATION_FAILED) // Some drivers return this when a card does not support vulkan
		return {};
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkEnumeratePhysicalDevices failed");
	if (deviceCount == 0)
		return {};

	std::vector<VkPhysicalDevice> devices(deviceCount);
	result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkEnumeratePhysicalDevices failed (2)");

	std::vector<VulkanPhysicalDevice> devinfo(deviceCount);
	for (size_t i = 0; i < devices.size(); i++)
	{
		auto &dev = devinfo[i];
		dev.device = devices[i];

		VkPhysicalDeviceProperties2 props = {};
		VkPhysicalDeviceRayTracingPropertiesNV rayprops = {};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = &rayprops;
		rayprops.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
		vkGetPhysicalDeviceProperties2(dev.device, &props);
		dev.properties = props.properties;
		dev.rayTracingProperties = rayprops;
		dev.rayTracingProperties.pNext = nullptr;

		vkGetPhysicalDeviceMemoryProperties(dev.device, &dev.memoryProperties);
		vkGetPhysicalDeviceFeatures(dev.device, &dev.features);

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(dev.device, &queueFamilyCount, nullptr);
		dev.queueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(dev.device, &queueFamilyCount, dev.queueFamilies.data());

		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(dev.device, nullptr, &deviceExtensionCount, nullptr);
		dev.extensions.resize(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(dev.device, nullptr, &deviceExtensionCount, dev.extensions.data());
	}
	return devinfo;
}

#ifdef WIN32
std::vector<const char *> VulkanDevice::getPlatformExtensions()
{
	return
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};
}
#endif

void VulkanDevice::initVolk()
{
	if (volkInitialize() != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to find Vulkan");
	}
	auto iver = volkGetInstanceVersion();
	if (iver == 0)
	{
		throw std::runtime_error("Vulkan not supported");
	}
}

void VulkanDevice::releaseResources()
{
	if (device)
		vkDeviceWaitIdle(device);

	if (allocator)
		vmaDestroyAllocator(allocator);
	allocator = VK_NULL_HANDLE;

	if (device)
		vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;

	if (surface)
		vkDestroySurfaceKHR(instance, surface, nullptr);
	surface = VK_NULL_HANDLE;

	if (debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	if (instance)
		vkDestroyInstance(instance, nullptr);
	instance = VK_NULL_HANDLE;
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
