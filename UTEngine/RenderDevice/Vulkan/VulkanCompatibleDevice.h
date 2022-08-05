#pragma once

#include "VulkanInstance.h"

class VulkanSurface;

class VulkanCompatibleDevice
{
public:
	VulkanPhysicalDevice* Device = nullptr;

	int GraphicsFamily = -1;
	int PresentFamily = -1;

	bool GraphicsTimeQueries = false;

	std::vector<std::string> EnabledDeviceExtensions;
	VulkanDeviceFeatures EnabledFeatures;

	static std::vector<VulkanCompatibleDevice> FindDevices(const std::shared_ptr<VulkanInstance>& instance, const std::shared_ptr<VulkanSurface>& surface);
	static VulkanCompatibleDevice SelectDevice(const std::shared_ptr<VulkanInstance>& instance, const std::shared_ptr<VulkanSurface>& surface, int vk_device);
};
