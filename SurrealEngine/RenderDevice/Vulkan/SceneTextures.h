#pragma once

#include "ShaderManager.h"
#include "Math/mat.h"
#include "Math/vec.h"

class VulkanRenderDevice;

enum { NumBloomLevels = 4 };

class SceneTextures
{
public:
	SceneTextures(VulkanRenderDevice* renderer, int width, int height, int multisample);
	~SceneTextures();

	// Current active multisample setting
	VkSampleCountFlagBits SceneSamples = VK_SAMPLE_COUNT_1_BIT;

	// Scene framebuffer color image
	std::unique_ptr<VulkanImage> ColorBuffer;
	std::unique_ptr<VulkanImageView> ColorBufferView;

	// Scene framebuffer hit results
	std::unique_ptr<VulkanImage> HitBuffer;
	std::unique_ptr<VulkanImageView> HitBufferView;

	// Scene framebuffer depth buffer
	std::unique_ptr<VulkanImage> DepthBuffer;
	std::unique_ptr<VulkanImageView> DepthBufferView;

	// Post processing image buffers
	std::unique_ptr<VulkanImage> PPImage[2];
	std::unique_ptr<VulkanImageView> PPImageView[2];

	// Texture and buffer used to download the hitbuffer
	std::unique_ptr<VulkanImage> PPHitBuffer;
	std::unique_ptr<VulkanBuffer> StagingHitBuffer;

	// Size of the scene framebuffer
	int Width = 0;
	int Height = 0;
	int Multisample = 0;

	// Bloom post processing buffers
	struct
	{
		std::unique_ptr<VulkanImage> VTexture;
		std::unique_ptr<VulkanImageView> VTextureView;
		std::unique_ptr<VulkanImage> HTexture;
		std::unique_ptr<VulkanImageView> HTextureView;
		int Width = 0;
		int Height = 0;
	} BloomBlurLevels[NumBloomLevels];

private:
	static VkSampleCountFlagBits GetBestSampleCount(VulkanDevice* device, int multisample);
};
