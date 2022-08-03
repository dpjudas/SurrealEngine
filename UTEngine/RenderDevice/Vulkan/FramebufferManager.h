#pragma once

#include "VulkanObjects.h"

class VulkanRenderDevice;

class FramebufferManager
{
public:
	FramebufferManager(VulkanRenderDevice* renderer);

	void CreateSceneFramebuffer();
	void DestroySceneFramebuffer();

	VulkanFramebuffer* GetSwapChainFramebuffer();

	std::unique_ptr<VulkanFramebuffer> sceneFramebuffer;

private:
	VulkanRenderDevice* renderer = nullptr;
	std::unique_ptr<VulkanFramebuffer> swapChainFramebuffer;
};
