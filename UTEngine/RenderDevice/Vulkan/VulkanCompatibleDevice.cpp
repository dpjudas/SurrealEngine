
#include "Precomp.h"
#include "VulkanCompatibleDevice.h"
#include "VulkanSurface.h"

std::vector<VulkanCompatibleDevice> VulkanCompatibleDevice::FindDevices(const std::shared_ptr<VulkanInstance>& instance, const std::shared_ptr<VulkanSurface>& surface)
{
	std::vector<std::string> RequiredDeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<std::string> OptionalDeviceExtensions =
	{
		VK_EXT_HDR_METADATA_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_RAY_QUERY_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME
#endif
	};

	std::vector<VulkanCompatibleDevice> supportedDevices;

	for (size_t idx = 0; idx < instance->PhysicalDevices.size(); idx++)
	{
		const auto& info = instance->PhysicalDevices[idx];

		// Check if all required extensions are there
		std::set<std::string> requiredExtensionSearch(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());
		for (const auto& ext : info.Extensions)
			requiredExtensionSearch.erase(ext.extensionName);
		if (!requiredExtensionSearch.empty())
			continue;

		// Check if all required features are there
		if (info.Features.Features.samplerAnisotropy != VK_TRUE ||
			info.Features.Features.fragmentStoresAndAtomics != VK_TRUE)
			continue;

		VulkanCompatibleDevice dev;
		dev.Device = &instance->PhysicalDevices[idx];
		dev.EnabledDeviceExtensions = RequiredDeviceExtensions;

		// Enable optional extensions we are interested in, if they are available on this device
		for (const auto& ext : dev.Device->Extensions)
		{
			for (const auto& opt : OptionalDeviceExtensions)
			{
				if (ext.extensionName == opt)
				{
					dev.EnabledDeviceExtensions.push_back(opt);
				}
			}
		}

		// Enable optional features we are interested in, if they are available on this device
		auto& enabledFeatures = dev.EnabledFeatures;
		auto& deviceFeatures = dev.Device->Features;
		enabledFeatures.Features.samplerAnisotropy = deviceFeatures.Features.samplerAnisotropy;
		enabledFeatures.Features.fragmentStoresAndAtomics = deviceFeatures.Features.fragmentStoresAndAtomics;
		enabledFeatures.Features.depthClamp = deviceFeatures.Features.depthClamp;
		enabledFeatures.Features.shaderClipDistance = deviceFeatures.Features.shaderClipDistance;
		enabledFeatures.BufferDeviceAddress.bufferDeviceAddress = deviceFeatures.BufferDeviceAddress.bufferDeviceAddress;
		enabledFeatures.AccelerationStructure.accelerationStructure = deviceFeatures.AccelerationStructure.accelerationStructure;
		enabledFeatures.RayQuery.rayQuery = deviceFeatures.RayQuery.rayQuery;
		enabledFeatures.DescriptorIndexing.runtimeDescriptorArray = deviceFeatures.DescriptorIndexing.runtimeDescriptorArray;
		enabledFeatures.DescriptorIndexing.descriptorBindingPartiallyBound = deviceFeatures.DescriptorIndexing.descriptorBindingPartiallyBound;
		enabledFeatures.DescriptorIndexing.descriptorBindingSampledImageUpdateAfterBind = deviceFeatures.DescriptorIndexing.descriptorBindingSampledImageUpdateAfterBind;
		enabledFeatures.DescriptorIndexing.descriptorBindingVariableDescriptorCount = deviceFeatures.DescriptorIndexing.descriptorBindingVariableDescriptorCount;

		// Figure out which queue can present
		if (surface)
		{
			for (int i = 0; i < (int)info.QueueFamilies.size(); i++)
			{
				VkBool32 presentSupport = false;
				VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(info.Device, i, surface->Surface, &presentSupport);
				if (result == VK_SUCCESS && info.QueueFamilies[i].queueCount > 0 && presentSupport)
				{
					dev.PresentFamily = i;
					break;
				}
			}
		}

		// The vulkan spec states that graphics and compute queues can always do transfer.
		// Furthermore the spec states that graphics queues always can do compute.
		// Last, the spec makes it OPTIONAL whether the VK_QUEUE_TRANSFER_BIT is set for such queues, but they MUST support transfer.
		//
		// In short: pick the first graphics queue family for everything.
		for (int i = 0; i < (int)info.QueueFamilies.size(); i++)
		{
			const auto& queueFamily = info.QueueFamilies[i];
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				dev.GraphicsFamily = i;
				dev.GraphicsTimeQueries = queueFamily.timestampValidBits != 0;
				break;
			}
		}

		// Only use device if we found the required graphics and present queues
		if (dev.GraphicsFamily != -1 && (!surface || dev.PresentFamily != -1))
		{
			supportedDevices.push_back(dev);
		}
	}

	// The device order returned by Vulkan can be anything. Prefer discrete > integrated > virtual gpu > cpu > other
	std::stable_sort(supportedDevices.begin(), supportedDevices.end(), [&](const auto& a, const auto b) {

		// Sort by GPU type first. This will ensure the "best" device is most likely to map to vk_device 0
		static const int typeSort[] = { 4, 1, 0, 2, 3 };
		int sortA = a.Device->Properties.deviceType < 5 ? typeSort[a.Device->Properties.deviceType] : (int)a.Device->Properties.deviceType;
		int sortB = b.Device->Properties.deviceType < 5 ? typeSort[b.Device->Properties.deviceType] : (int)b.Device->Properties.deviceType;
		if (sortA != sortB)
			return sortA < sortB;

		// Then sort by the device's unique ID so that vk_device uses a consistent order
		int sortUUID = memcmp(a.Device->Properties.pipelineCacheUUID, b.Device->Properties.pipelineCacheUUID, VK_UUID_SIZE);
		return sortUUID < 0;
		});

	return supportedDevices;
}

VulkanCompatibleDevice VulkanCompatibleDevice::SelectDevice(const std::shared_ptr<VulkanInstance>& instance, const std::shared_ptr<VulkanSurface>& surface, int vk_device)
{
	if (instance->PhysicalDevices.empty())
		VulkanError("No Vulkan devices found. The graphics card may have no vulkan support or the driver may be too old.");

	std::vector<VulkanCompatibleDevice> supportedDevices = FindDevices(instance, surface);
	if (supportedDevices.empty())
		VulkanError("No Vulkan device found supports the minimum requirements of this application");

	size_t selected = vk_device;
	if (selected >= supportedDevices.size())
		selected = 0;
	return supportedDevices[selected];
}
