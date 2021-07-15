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

class SceneRenderPass
{
public:
	SceneRenderPass(Renderer* renderer);
	~SceneRenderPass();

	void begin(VulkanCommandBuffer *cmdbuffer);
	void end(VulkanCommandBuffer *cmdbuffer);

	VulkanPipeline* getPipeline(uint32_t polyflags);
	VulkanPipeline* getEndFlashPipeline();

private:
	void createRenderPass();
	void createPipeline();
	void createFramebuffer();

	Renderer* renderer = nullptr;

	std::unique_ptr<VulkanShader> vertexShader;
	std::unique_ptr<VulkanShader> fragmentShader;
	std::unique_ptr<VulkanShader> fragmentShaderAlphaTest;

	std::unique_ptr<VulkanRenderPass> renderPass;
	std::unique_ptr<VulkanFramebuffer> sceneFramebuffer;
	std::unique_ptr<VulkanPipeline> pipeline[32];

	SceneRenderPass(const SceneRenderPass &) = delete;
	SceneRenderPass &operator=(const SceneRenderPass &) = delete;
};
