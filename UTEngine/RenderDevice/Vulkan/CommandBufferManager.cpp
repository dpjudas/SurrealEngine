
#include "Precomp.h"
#include "CommandBufferManager.h"
#include "VulkanRenderDevice.h"
#include "VulkanBuilders.h"
#include "VulkanSwapChain.h"

CommandBufferManager::CommandBufferManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	SwapChain.reset(new VulkanSwapChain(renderer->Device, renderer->UseVSync));
	ImageAvailableSemaphore.reset(new VulkanSemaphore(renderer->Device));
	RenderFinishedSemaphore.reset(new VulkanSemaphore(renderer->Device));
	RenderFinishedFence.reset(new VulkanFence(renderer->Device));
	TransferSemaphore.reset(new VulkanSemaphore(renderer->Device));
	CommandPool.reset(new VulkanCommandPool(renderer->Device, renderer->Device->GraphicsFamily));
	FrameDeleteList.reset(new DeleteList());
}

CommandBufferManager::~CommandBufferManager()
{
	DeleteFrameObjects();
}

void CommandBufferManager::WaitForTransfer()
{
	renderer->Uploads->SubmitUploads();

	if (TransferCommands)
	{
		TransferCommands->end();

		QueueSubmit()
			.AddCommandBuffer(TransferCommands.get())
			.Execute(renderer->Device, renderer->Device->GraphicsQueue, RenderFinishedFence.get());

		vkWaitForFences(renderer->Device->device, 1, &RenderFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(renderer->Device->device, 1, &RenderFinishedFence->fence);

		TransferCommands.reset();
	}
}

void CommandBufferManager::SubmitCommands(bool present, int presentWidth, int presentHeight)
{
	renderer->Uploads->SubmitUploads();

	if (present)
	{
		PresentImageIndex = SwapChain->acquireImage(presentWidth, presentHeight, false, ImageAvailableSemaphore.get());
		if (PresentImageIndex != 0xffffffff)
		{
			renderer->DrawPresentTexture(0, 0, presentWidth, presentHeight);
		}
	}

	if (TransferCommands)
	{
		TransferCommands->end();

		QueueSubmit()
			.AddCommandBuffer(TransferCommands.get())
			.AddSignal(TransferSemaphore.get())
			.Execute(renderer->Device, renderer->Device->GraphicsQueue);
	}

	if (DrawCommands)
		DrawCommands->end();

	QueueSubmit submit;
	if (DrawCommands)
	{
		submit.AddCommandBuffer(DrawCommands.get());
	}
	if (TransferCommands)
	{
		submit.AddWait(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, TransferSemaphore.get());
	}
	if (present && PresentImageIndex != 0xffffffff)
	{
		submit.AddWait(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, ImageAvailableSemaphore.get());
		submit.AddSignal(RenderFinishedSemaphore.get());
	}
	submit.Execute(renderer->Device, renderer->Device->GraphicsQueue, RenderFinishedFence.get());

	if (present && PresentImageIndex != 0xffffffff)
	{
		SwapChain->queuePresent(PresentImageIndex, RenderFinishedSemaphore.get());
	}

	vkWaitForFences(renderer->Device->device, 1, &RenderFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(renderer->Device->device, 1, &RenderFinishedFence->fence);

	DrawCommands.reset();
	TransferCommands.reset();
	DeleteFrameObjects();
}

VulkanCommandBuffer* CommandBufferManager::GetTransferCommands()
{
	if (!TransferCommands)
	{
		TransferCommands = CommandPool->createBuffer();
		TransferCommands->begin();
	}
	return TransferCommands.get();
}

VulkanCommandBuffer* CommandBufferManager::GetDrawCommands()
{
	if (!DrawCommands)
	{
		DrawCommands = CommandPool->createBuffer();
		DrawCommands->begin();
	}
	return DrawCommands.get();
}

void CommandBufferManager::DeleteFrameObjects()
{
	FrameDeleteList.reset(new DeleteList());
}
