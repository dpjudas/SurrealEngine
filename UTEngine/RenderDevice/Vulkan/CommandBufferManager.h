#pragma once

#include "VulkanObjects.h"

class VulkanRenderDevice;

class CommandBufferManager
{
public:
	CommandBufferManager(VulkanRenderDevice* renderer);
	~CommandBufferManager();

	void WaitForTransfer();
	void SubmitCommands(bool present, int presentWidth, int presentHeight);
	VulkanCommandBuffer* GetTransferCommands();
	VulkanCommandBuffer* GetDrawCommands();
	void DeleteFrameObjects();

	struct DeleteList
	{
		std::vector<std::unique_ptr<VulkanImage>> images;
		std::vector<std::unique_ptr<VulkanImageView>> imageViews;
		std::vector<std::unique_ptr<VulkanBuffer>> buffers;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> descriptors;
	};
	std::unique_ptr<DeleteList> FrameDeleteList;

	std::unique_ptr<VulkanSwapChain> SwapChain;
	uint32_t PresentImageIndex = 0xffffffff;

private:
	VulkanRenderDevice* renderer = nullptr;

	std::unique_ptr<VulkanSemaphore> ImageAvailableSemaphore;
	std::unique_ptr<VulkanSemaphore> RenderFinishedSemaphore;
	std::unique_ptr<VulkanSemaphore> TransferSemaphore;
	std::unique_ptr<VulkanFence> RenderFinishedFence;
	std::unique_ptr<VulkanCommandPool> CommandPool;
	std::unique_ptr<VulkanCommandBuffer> DrawCommands;
	std::unique_ptr<VulkanCommandBuffer> TransferCommands;
};
