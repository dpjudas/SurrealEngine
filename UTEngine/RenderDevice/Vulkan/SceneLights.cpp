
#include "Precomp.h"
#include "SceneLights.h"
#include "VulkanBuilders.h"
#include "Renderer.h"

SceneLights::SceneLights(Renderer* renderer) : renderer(renderer)
{
	BufferBuilder bufbuild;

	bufbuild.setSize(sizeof(SceneLight) * MaxLights);
	bufbuild.setUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Lights = bufbuild.create(renderer->Device);
	bufbuild.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	StagingLights = bufbuild.create(renderer->Device);

	bufbuild.setSize(1'000'000 * sizeof(int32_t));
	bufbuild.setUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	SurfaceLights = bufbuild.create(renderer->Device);
	bufbuild.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	StagingSurfaceLights = bufbuild.create(renderer->Device);

	createImage(Shadowmap, ShadowmapView, renderer->Device, 4096, 4096, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	createImage(ShadowColorBuffer, ShadowColorBufferView, renderer->Device, ShadowmapSize, ShadowmapSize, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	createImage(ShadowDepthBuffer, ShadowDepthBufferView, renderer->Device, ShadowmapSize, ShadowmapSize, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

	Shadowmap->SetDebugName("SceneBuffers.shadowmap");
	ShadowmapView->SetDebugName("SceneBuffers.shadowmapView");
	ShadowColorBuffer->SetDebugName("SceneBuffers.shadowColorBuffer");
	ShadowColorBufferView->SetDebugName("SceneBuffers.shadowColorBufferView");
	ShadowDepthBuffer->SetDebugName("SceneBuffers.shadowDepthBuffer");
	ShadowDepthBufferView->SetDebugName("SceneBuffers.shadowDepthBufferView");

	PipelineBarrier barrier;
	barrier.addImage(Shadowmap.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	barrier.addImage(ShadowColorBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	barrier.addImage(ShadowDepthBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	barrier.execute(renderer->GetDrawCommands(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

SceneLights::~SceneLights()
{
}

void SceneLights::createImage(std::unique_ptr<VulkanImage>& image, std::unique_ptr<VulkanImageView>& view, VulkanDevice* device, int width, int height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect)
{
	ImageBuilder imgbuild;
	imgbuild.setFormat(format);
	imgbuild.setUsage(usage);
	imgbuild.setSize(width, height);
	image = imgbuild.create(device);

	ImageViewBuilder viewbuild;
	viewbuild.setImage(image.get(), format, aspect);
	view = viewbuild.create(device);
}
