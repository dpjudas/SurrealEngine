
#include "Precomp.h"
#include "VulkanFrameBuffer.h"
#include "VulkanBuilders.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRenderDevice.h"

VulkanFrameBufferManager::VulkanFrameBufferManager(VulkanRenderDevice* renderer, int width, int height, int multisample) : width(width), height(height)
{
	sceneSamples = getBestSampleCount(renderer->Device, multisample);

	createImage(colorBuffer, colorBufferView, renderer->Device, width, height, sceneSamples, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	createImage(depthBuffer, depthBufferView, renderer->Device, width, height, sceneSamples, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

	colorBuffer->SetDebugName("VulkanFrameBufferManager.colorBuffer");
	colorBufferView->SetDebugName("VulkanFrameBufferManager.colorBufferView");
	depthBuffer->SetDebugName("VulkanFrameBufferManager.depthBuffer");
	depthBufferView->SetDebugName("VulkanFrameBufferManager.depthBufferView");

	PipelineBarrier barrier;
	barrier.addImage(colorBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	barrier.addImage(depthBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	barrier.execute(renderer->Commands->GetDrawCommands(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
}

VulkanFrameBufferManager::~VulkanFrameBufferManager()
{
}

void VulkanFrameBufferManager::createImage(std::unique_ptr<VulkanImage> &image, std::unique_ptr<VulkanImageView> &view, VulkanDevice *device, int width, int height, VkSampleCountFlagBits samples, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
{
	ImageBuilder imgbuild;
	imgbuild.setFormat(format);
	imgbuild.setUsage(usage);
	imgbuild.setSize(width, height);
	imgbuild.setSamples(samples);
	image = imgbuild.create(device);

	ImageViewBuilder viewbuild;
	viewbuild.setImage(image.get(), format, aspect);
	view = viewbuild.create(device);
}

VkSampleCountFlagBits VulkanFrameBufferManager::getBestSampleCount(VulkanDevice* device, int multisample)
{
	const auto& limits = device->physicalDevice.properties.limits;
	int requestedSamples = clamp(multisample, 0, 64);
	VkSampleCountFlags deviceSampleCounts = limits.sampledImageColorSampleCounts & limits.sampledImageDepthSampleCounts & limits.sampledImageStencilSampleCounts;

	int samples = 1;
	VkSampleCountFlags bit = VK_SAMPLE_COUNT_1_BIT;
	VkSampleCountFlags best = bit;
	while (samples < requestedSamples)
	{
		if (deviceSampleCounts & bit)
		{
			best = bit;
		}
		samples <<= 1;
		bit <<= 1;
	}

	return (VkSampleCountFlagBits)best;
}
