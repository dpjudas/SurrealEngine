#pragma once

#include "VulkanDevice.h"

class VulkanSemaphore;
class VulkanFence;

class VulkanSwapChain
{
public:
	VulkanSwapChain(VulkanDevice *device, bool vsync);
	~VulkanSwapChain();

	uint32_t acquireImage(int width, int height, VulkanSemaphore *semaphore = nullptr, VulkanFence *fence = nullptr);
	void queuePresent(uint32_t imageIndex, VulkanSemaphore *semaphore = nullptr);

	bool vsync;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	VkSurfaceFormatKHR swapChainFormat;
	VkPresentModeKHR swapChainPresentMode;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VkExtent2D actualExtent;

private:
	void selectFormat();
	void selectPresentMode();
	bool createSwapChain(int width, int height, VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE);
	void createViews();
	void getImages();
	void releaseResources();
	void releaseViews();
	void recreate(int width, int height);

	VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
	std::vector<VkSurfaceFormatKHR> getSurfaceFormats();
	std::vector<VkPresentModeKHR> getPresentModes();

	VulkanDevice *device = nullptr;

	int lastSwapWidth = 0;
	int lastSwapHeight = 0;

	VulkanSwapChain(const VulkanSwapChain &) = delete;
	VulkanSwapChain &operator=(const VulkanSwapChain &) = delete;
};
