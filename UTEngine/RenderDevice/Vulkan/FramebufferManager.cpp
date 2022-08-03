
#include "Precomp.h"
#include "FramebufferManager.h"
#include "VulkanRenderDevice.h"
#include "VulkanBuilders.h"
#include "VulkanSwapChain.h"

FramebufferManager::FramebufferManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
}

void FramebufferManager::CreateSceneFramebuffer()
{
	sceneFramebuffer = FramebufferBuilder()
		.RenderPass(renderer->RenderPasses->SceneRenderPass.get())
		.Size(renderer->Textures->Scene->width, renderer->Textures->Scene->height)
		.AddAttachment(renderer->Textures->Scene->ColorBufferView.get())
		.AddAttachment(renderer->Textures->Scene->DepthBufferView.get())
		.DebugName("SceneFramebuffer")
		.Create(renderer->Device);
}

void FramebufferManager::DestroySceneFramebuffer()
{
	sceneFramebuffer.reset();
}

VulkanFramebuffer* FramebufferManager::GetSwapChainFramebuffer()
{
	swapChainFramebuffer.reset();

	swapChainFramebuffer = FramebufferBuilder()
		.RenderPass(renderer->RenderPasses->PresentRenderPass.get())
		.Size(renderer->Textures->Scene->width, renderer->Textures->Scene->height)
		.AddAttachment(renderer->Commands->SwapChain->swapChainImageViews[renderer->Commands->PresentImageIndex])
		.DebugName("SwapChainFramebuffer")
		.Create(renderer->Device);
	return swapChainFramebuffer.get();
}
