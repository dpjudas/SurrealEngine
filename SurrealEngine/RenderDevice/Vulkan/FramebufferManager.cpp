
#include "Precomp.h"
#include "FramebufferManager.h"
#include "VulkanRenderDevice.h"
#include <zvulkan/vulkanbuilders.h>
#include <zvulkan/vulkanswapchain.h>

FramebufferManager::FramebufferManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
}

void FramebufferManager::CreateSceneFramebuffer()
{
	SceneFramebuffer = FramebufferBuilder()
		.RenderPass(renderer->RenderPasses->Scene.RenderPass.get())
		.Size(renderer->Textures->Scene->Width, renderer->Textures->Scene->Height)
		.AddAttachment(renderer->Textures->Scene->ColorBufferView.get())
		.AddAttachment(renderer->Textures->Scene->HitBufferView.get())
		.AddAttachment(renderer->Textures->Scene->DepthBufferView.get())
		.DebugName("SceneFramebuffer")
		.Create(renderer->Device.get());

	for (int level = 0; level < NumBloomLevels; level++)
	{
		BloomBlurLevels[level].VTextureFB = FramebufferBuilder()
			.RenderPass(renderer->RenderPasses->Postprocess.RenderPass.get())
			.Size(renderer->Textures->Scene->BloomBlurLevels[level].Width, renderer->Textures->Scene->BloomBlurLevels[level].Height)
			.AddAttachment(renderer->Textures->Scene->BloomBlurLevels[level].VTextureView.get())
			.DebugName("VTextureFB")
			.Create(renderer->Device.get());

		BloomBlurLevels[level].HTextureFB = FramebufferBuilder()
			.RenderPass(renderer->RenderPasses->Postprocess.RenderPass.get())
			.Size(renderer->Textures->Scene->BloomBlurLevels[level].Width, renderer->Textures->Scene->BloomBlurLevels[level].Height)
			.AddAttachment(renderer->Textures->Scene->BloomBlurLevels[level].HTextureView.get())
			.DebugName("HTextureFB")
			.Create(renderer->Device.get());
	}

	for (int i = 0; i < 2; i++)
	{
		PPImageFB[i] = FramebufferBuilder()
			.RenderPass(renderer->RenderPasses->Postprocess.RenderPass.get())
			.Size(renderer->Textures->Scene->Width, renderer->Textures->Scene->Height)
			.AddAttachment(renderer->Textures->Scene->PPImageView[i].get())
			.DebugName("BloomPPImageFB")
			.Create(renderer->Device.get());
	}
}

void FramebufferManager::DestroySceneFramebuffer()
{
	SceneFramebuffer.reset();
	for (int level = 0; level < NumBloomLevels; level++)
	{
		BloomBlurLevels[level].VTextureFB.reset();
		BloomBlurLevels[level].HTextureFB.reset();
	}

	for (int i = 0; i < 2; i++)
		PPImageFB[i].reset();
}

void FramebufferManager::CreateSwapChainFramebuffers()
{
	renderer->RenderPasses->CreatePresentRenderPass();
	renderer->RenderPasses->CreatePresentPipeline();

	auto swapchain = renderer->Commands->SwapChain.get();
	for (int i = 0; i < swapchain->ImageCount(); i++)
	{
		SwapChainFramebuffers.push_back(
			FramebufferBuilder()
				.RenderPass(renderer->RenderPasses->Present.RenderPass.get())
				.Size(renderer->Commands->SwapChain->Width(), renderer->Commands->SwapChain->Height())
				.AddAttachment(swapchain->GetImageView(i))
				.DebugName("SwapChainFramebuffer")
				.Create(renderer->Device.get()));
	}
}

void FramebufferManager::DestroySwapChainFramebuffers()
{
	SwapChainFramebuffers.clear();
}

VulkanFramebuffer* FramebufferManager::GetSwapChainFramebuffer()
{
	return SwapChainFramebuffers[renderer->Commands->PresentImageIndex].get();
}
