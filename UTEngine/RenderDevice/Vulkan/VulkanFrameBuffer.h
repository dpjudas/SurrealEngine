#pragma once

#include "VulkanObjects.h"
#include "Math/mat.h"
#include "Math/vec.h"

class VulkanRenderDevice;

struct SceneVertex
{
	uint32_t flags;
	float x, y, z;
	float u, v;
	float u2, v2;
	float u3, v3;
	float u4, v4;
	float r, g, b, a;
};

struct ScenePushConstants
{
	mat4 objectToProjection;
};

class VulkanFrameBufferManager
{
public:
	VulkanFrameBufferManager(VulkanRenderDevice* renderer, int width, int height, int multisample);
	~VulkanFrameBufferManager();

	VkSampleCountFlagBits sceneSamples = VK_SAMPLE_COUNT_1_BIT;

	std::unique_ptr<VulkanImage> colorBuffer;
	std::unique_ptr<VulkanImageView> colorBufferView;
	VkImageLayout colorBufferLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::unique_ptr<VulkanImage> depthBuffer;
	std::unique_ptr<VulkanImageView> depthBufferView;

	int width = 0;
	int height = 0;

private:
	VulkanFrameBufferManager(const VulkanFrameBufferManager &) = delete;
	VulkanFrameBufferManager &operator=(const VulkanFrameBufferManager &) = delete;

	static void createImage(std::unique_ptr<VulkanImage> &image, std::unique_ptr<VulkanImageView> &view, VulkanDevice *device, int width, int height, VkSampleCountFlagBits samples, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
	static VkSampleCountFlagBits getBestSampleCount(VulkanDevice* device, int multisample);
};
