
#include "Precomp.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "Postprocess.h"
#include "VulkanPostprocess.h"
#include "VulkanRenderDevice.h"

VulkanCommandBufferManager::VulkanCommandBufferManager(VulkanRenderDevice* renderer, bool vsync) : renderer(renderer)
{
	FrameDeleteList = std::make_unique<DeleteList>();
	SwapChain = std::make_unique<VulkanSwapChain>(renderer->Device, vsync);
	ImageAvailableSemaphore = std::make_unique<VulkanSemaphore>(renderer->Device);
	RenderFinishedSemaphore = std::make_unique<VulkanSemaphore>(renderer->Device);
	RenderFinishedFence = std::make_unique<VulkanFence>(renderer->Device);
	TransferSemaphore = std::make_unique<VulkanSemaphore>(renderer->Device);
	CommandPool = std::make_unique<VulkanCommandPool>(renderer->Device, renderer->Device->graphicsFamily);
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
	DeleteFrameObjects();
}

VulkanCommandBuffer* VulkanCommandBufferManager::GetTransferCommands()
{
	if (!TransferCommands)
	{
		TransferCommands = CommandPool->createBuffer().release();
		TransferCommands->begin();
	}
	return TransferCommands;
}

VulkanCommandBuffer* VulkanCommandBufferManager::GetDrawCommands()
{
	if (!DrawCommands)
	{
		DrawCommands = CommandPool->createBuffer().release();
		DrawCommands->begin();
	}
	return DrawCommands;
}

void VulkanCommandBufferManager::SubmitCommands(bool present, int presentWidth, int presentHeight)
{
	if (present)
	{
		//RECT clientbox = {};
		//GetClientRect(WindowHandle, &clientbox);
		//int presentWidth = clientbox.right;
		//int presentHeight = clientbox.bottom;

		PresentImageIndex = SwapChain->acquireImage(presentWidth, presentHeight, ImageAvailableSemaphore.get());
		if (PresentImageIndex != 0xffffffff)
		{
			PPViewport box;
			box.x = 0;
			box.y = 0;
			box.width = presentWidth;
			box.height = presentHeight;
			renderer->Postprocessing->drawPresentTexture(box);
		}
	}

	if (TransferCommands)
	{
		TransferCommands->end();

		QueueSubmit submit;
		submit.addCommandBuffer(TransferCommands);
		submit.addSignal(TransferSemaphore.get());
		submit.execute(renderer->Device, renderer->Device->graphicsQueue);
	}

	if (DrawCommands)
		DrawCommands->end();

	QueueSubmit submit;
	if (DrawCommands)
	{
		submit.addCommandBuffer(DrawCommands);
	}
	if (TransferCommands)
	{
		submit.addWait(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, TransferSemaphore.get());
	}
	if (present && PresentImageIndex != 0xffffffff)
	{
		submit.addWait(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, ImageAvailableSemaphore.get());
		submit.addSignal(RenderFinishedSemaphore.get());
	}
	submit.execute(renderer->Device, renderer->Device->graphicsQueue, RenderFinishedFence.get());

	if (present && PresentImageIndex != 0xffffffff)
	{
		SwapChain->queuePresent(PresentImageIndex, RenderFinishedSemaphore.get());
	}

	vkWaitForFences(renderer->Device->device, 1, &RenderFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(renderer->Device->device, 1, &RenderFinishedFence->fence);

	delete DrawCommands; DrawCommands = nullptr;
	delete TransferCommands; TransferCommands = nullptr;
	DeleteFrameObjects();
}

void VulkanCommandBufferManager::DeleteFrameObjects()
{
	FrameDeleteList = std::make_unique<DeleteList>();
}
