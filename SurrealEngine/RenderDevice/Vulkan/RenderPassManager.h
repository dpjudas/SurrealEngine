#pragma once

#include <zvulkan/vulkanobjects.h>
#include "Math/vec.h"

class VulkanRenderDevice;

class RenderPassManager
{
public:
	RenderPassManager(VulkanRenderDevice* renderer);
	~RenderPassManager();

	std::unique_ptr<VulkanPipelineLayout> ScenePipelineLayout;
	std::unique_ptr<VulkanPipelineLayout> SceneBindlessPipelineLayout;
	std::unique_ptr<VulkanPipelineLayout> PresentPipelineLayout;

	void CreateRenderPass();
	void CreatePipelines();

	void CreatePresentRenderPass();
	void CreatePresentPipeline();
	void DestroyPresentRenderPass();
	void DestroyPresentPipeline();

	void BeginScene(VulkanCommandBuffer* cmdbuffer, vec4 screenClear);
	void EndScene(VulkanCommandBuffer* cmdbuffer);

	void BeginPresent(VulkanCommandBuffer* cmdbuffer);
	void EndPresent(VulkanCommandBuffer* cmdbuffer);

	VulkanPipeline* getPipeline(uint32_t polyflags, bool bindless);
	VulkanPipeline* getEndFlashPipeline();

	std::unique_ptr<VulkanRenderPass> SceneRenderPass;

	std::unique_ptr<VulkanRenderPass> PresentRenderPass;
	std::unique_ptr<VulkanPipeline> PresentPipeline;

private:
	void CreateScenePipelineLayout();
	void CreateSceneBindlessPipelineLayout();
	void CreatePresentPipelineLayout();

	VulkanRenderDevice* renderer = nullptr;
	std::unique_ptr<VulkanPipeline> pipeline[2][32];
};
