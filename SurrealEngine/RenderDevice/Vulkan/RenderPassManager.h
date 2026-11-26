#pragma once

#include "RenderDevice/RenderDevice.h"
#include <surrealgpu/vulkanobjects.h>

class VulkanRenderDevice;

struct PipelineState
{
	std::unique_ptr<VulkanPipeline> Pipeline;
	float MinDepth = 0.1f;
	float MaxDepth = 1.0f;
};

class RenderPassManager
{
public:
	RenderPassManager(VulkanRenderDevice* renderer);
	~RenderPassManager();

	void CreateRenderPass();
	void CreatePipelines();

	void CreatePresentRenderPass();
	void CreatePresentPipeline();
	void CreateScreenshotPipeline();

	void CreatePostprocessRenderPass();
	void CreateBloomPipeline();

	PipelineState* GetPipeline(uint32_t polyflags, bool tile);
	PipelineState* GetEndFlashPipeline();
	PipelineState* GetLinePipeline(bool occludeLines) { return &Scene.LinePipeline[occludeLines]; }
	PipelineState* GetPointPipeline(bool occludeLines) { return &Scene.PointPipeline[occludeLines]; }
	PipelineState* GetStencilPipeline(StencilPolygonType type) { return &Scene.StencilPipeline[(int)type]; }

	struct
	{
		std::unique_ptr<VulkanPipelineLayout> BindlessPipelineLayout;
		std::unique_ptr<VulkanRenderPass> RenderPass;
		std::unique_ptr<VulkanRenderPass> RenderPassContinue;
		PipelineState Pipeline[33];
		PipelineState TilePipeline[33];
		PipelineState LinePipeline[2];
		PipelineState PointPipeline[2];
		PipelineState StencilPipeline[3];
	} Scene;

	struct
	{
		std::unique_ptr<VulkanPipelineLayout> PipelineLayout;
		std::unique_ptr<VulkanRenderPass> RenderPass;
		std::unique_ptr<VulkanPipeline> Pipeline[16];
		std::unique_ptr<VulkanPipeline> ScreenshotPipeline[16];
	} Present;

	struct
	{
		std::unique_ptr<VulkanPipelineLayout> PipelineLayout;
		std::unique_ptr<VulkanPipeline> Extract;
		std::unique_ptr<VulkanPipeline> Combine;
		std::unique_ptr<VulkanPipeline> Scale;
		std::unique_ptr<VulkanPipeline> BlurVertical;
		std::unique_ptr<VulkanPipeline> BlurHorizontal;
	} Bloom;

	struct
	{
		std::unique_ptr<VulkanRenderPass> RenderPass;
		std::unique_ptr<VulkanRenderPass> RenderPassCombine;
	} Postprocess;

private:
	void CreateSceneBindlessPipelineLayout();
	void CreatePresentPipelineLayout();
	void CreateBloomPipelineLayout();

	VulkanRenderDevice* renderer = nullptr;
};
