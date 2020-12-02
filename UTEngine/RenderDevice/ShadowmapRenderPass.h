#pragma once

#include "VulkanObjects.h"
#include "Math/vec.h"
#include "Math/mat.h"
#include <array>

class VulkanPipeline;
class VulkanPipelineLayout;
class VulkanRenderPass;
class VulkanFramebuffer;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class Renderer;

class ShadowmapRenderPass
{
public:
	ShadowmapRenderPass(Renderer* renderer);
	~ShadowmapRenderPass();

	void begin(VulkanCommandBuffer *cmdbuffer);
	void end(VulkanCommandBuffer *cmdbuffer);

	VulkanPipeline* getPipeline() { return pipeline.get(); }

private:
	void createRenderPass();
	void createPipeline();
	void createFramebuffer();

	Renderer* renderer = nullptr;

	std::unique_ptr<VulkanShader> vertexShader;
	std::unique_ptr<VulkanShader> fragmentShader;

	std::unique_ptr<VulkanRenderPass> renderPass;
	std::unique_ptr<VulkanFramebuffer> shadowFramebuffer;
	std::unique_ptr<VulkanPipeline> pipeline;

	ShadowmapRenderPass(const ShadowmapRenderPass &) = delete;
	ShadowmapRenderPass &operator=(const ShadowmapRenderPass &) = delete;
};
