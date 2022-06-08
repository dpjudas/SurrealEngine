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
class VulkanRenderDevice;

class VulkanRenderPassManager
{
public:
	VulkanRenderPassManager(VulkanRenderDevice* renderer);
	~VulkanRenderPassManager();

	void begin(VulkanCommandBuffer *cmdbuffer);
	void end(VulkanCommandBuffer *cmdbuffer);

	VulkanPipeline* getPipeline(uint32_t polyflags);
	VulkanPipeline* getEndFlashPipeline();

	std::unique_ptr<VulkanPipelineLayout> PipelineLayout;

private:
	void CreateScenePipelineLayout();
	void createRenderPass();
	void createPipeline();
	void createFramebuffer();

	VulkanRenderDevice* renderer = nullptr;

	std::unique_ptr<VulkanRenderPass> renderPass;
	std::unique_ptr<VulkanFramebuffer> sceneFramebuffer;
	std::unique_ptr<VulkanPipeline> pipeline[32];

	VulkanRenderPassManager(const VulkanRenderPassManager &) = delete;
	VulkanRenderPassManager &operator=(const VulkanRenderPassManager &) = delete;
};
