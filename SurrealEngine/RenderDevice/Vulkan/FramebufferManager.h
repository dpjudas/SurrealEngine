#pragma once

#include "SceneTextures.h"

class VulkanRenderDevice;

class FramebufferManager
{
public:
	FramebufferManager(VulkanRenderDevice* renderer);

	void CreateSceneFramebuffer();
	void DestroySceneFramebuffer();

	void CreateSwapChainFramebuffers();
	void DestroySwapChainFramebuffers();

	VulkanFramebuffer* GetSwapChainFramebuffer();

	std::unique_ptr<VulkanFramebuffer> SceneFramebuffer;
	std::unique_ptr<VulkanFramebuffer> PPImageFB[2];

	struct
	{
		std::unique_ptr<VulkanFramebuffer> VTextureFB;
		std::unique_ptr<VulkanFramebuffer> HTextureFB;
	} BloomBlurLevels[NumBloomLevels];

private:
	VulkanRenderDevice* renderer = nullptr;
	std::vector<std::unique_ptr<VulkanFramebuffer>> SwapChainFramebuffers;
};
