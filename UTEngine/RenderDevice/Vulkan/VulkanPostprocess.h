#pragma once

#include "VulkanRenderDevice.h"
#include "VulkanObjects.h"
#include "VulkanBuilders.h"
#include "Postprocess.h"
#include <array>

class VulkanPPShader;
class VulkanPPTexture;
class VulkanPPRenderPassSetup;
class PipelineBarrier;
class VulkanRenderDevice;

class VulkanPPRenderPassKey
{
public:
	VulkanPPShader *shader;
	int uniforms;
	int inputTextures;
	PPBlendMode blendMode;
	VkFormat outputFormat;
	int swapChain;
	VkSampleCountFlagBits samples;

	bool operator<(const VulkanPPRenderPassKey &other) const { return memcmp(this, &other, sizeof(VulkanPPRenderPassKey)) < 0; }
	bool operator==(const VulkanPPRenderPassKey &other) const { return memcmp(this, &other, sizeof(VulkanPPRenderPassKey)) == 0; }
	bool operator!=(const VulkanPPRenderPassKey &other) const { return memcmp(this, &other, sizeof(VulkanPPRenderPassKey)) != 0; }
};

class VulkanPPImageTransition
{
public:
	void addImage(VulkanImage *image, VkImageLayout *layout, VkImageLayout targetLayout, bool undefinedSrcLayout);
	void execute(VulkanCommandBuffer *cmdbuffer);

private:
	PipelineBarrier barrier;
	VkPipelineStageFlags srcStageMask = 0;
	VkPipelineStageFlags dstStageMask = 0;
	bool needbarrier = false;
};

class VulkanPostprocess
{
public:
	VulkanPostprocess(VulkanRenderDevice* renderer);
	~VulkanPostprocess();

	void beginFrame();
	void renderBuffersReset();

	void imageTransitionScene(bool undefinedSrcLayout);

	void blitSceneToPostprocess();
	void blitCurrentToImage(VulkanImage *image, VkImageLayout *layout, VkImageLayout finallayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	void drawPresentTexture(const PPViewport &box);

	enum { numPipelineImages = 2 };

private:
	VulkanSampler *getSampler(PPFilterMode filter, PPWrapMode wrap);

	VulkanRenderDevice* renderer;
	std::array<std::unique_ptr<VulkanSampler>, 16> mSamplers;
	std::map<VulkanPPRenderPassKey, std::unique_ptr<VulkanPPRenderPassSetup>> mRenderPassSetup;
	std::unique_ptr<VulkanDescriptorPool> mDescriptorPool;

	std::unique_ptr<VulkanImage> pipelineImage[2];
	std::unique_ptr<VulkanImageView> pipelineView[2];
	VkImageLayout pipelineLayout[2] = { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	int mCurrentPipelineImage = 0;

	friend class VulkanPPRenderState;
};

class VulkanPPShader : public PPShaderBackend
{
public:
	VulkanPPShader(VulkanRenderDevice* renderer, const PPShader *shader);

	std::unique_ptr<VulkanShader> vertexShader;
	std::unique_ptr<VulkanShader> fragmentShader;

private:
	static const char *getTypeStr(UniformType type);
};

class VulkanPPTexture : public PPTextureBackend
{
public:
	VulkanPPTexture(VulkanRenderDevice* renderer, PPTexture *texture);

	std::unique_ptr<VulkanImage> image;
	std::unique_ptr<VulkanImageView> view;
	std::unique_ptr<VulkanBuffer> staging;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkFormat format;
};

class VulkanPPRenderPassSetup
{
public:
	VulkanPPRenderPassSetup(VulkanRenderDevice* renderer, const VulkanPPRenderPassKey &key);

	VulkanRenderDevice* renderer;
	std::unique_ptr<VulkanDescriptorSetLayout> descriptorLayout;
	std::unique_ptr<VulkanPipelineLayout> pipelineLayout;
	std::unique_ptr<VulkanRenderPass> renderPass;
	std::unique_ptr<VulkanPipeline> pipeline;
	std::map<VkImageView, std::unique_ptr<VulkanFramebuffer>> framebuffers;

private:
	void createDescriptorLayout(const VulkanPPRenderPassKey &key);
	void createPipelineLayout(const VulkanPPRenderPassKey &key);
	void createPipeline(const VulkanPPRenderPassKey &key);
	void createRenderPass(const VulkanPPRenderPassKey &key);
};

class VulkanPPRenderState : public PPRenderState
{
public:
	VulkanPPRenderState(VulkanRenderDevice* renderer);

	void draw() override;

private:
	void renderScreenQuad(VulkanPPRenderPassSetup *passSetup, VulkanDescriptorSet *descriptorSet, VulkanFramebuffer *framebuffer, int framebufferWidth, int framebufferHeight, int x, int y, int width, int height, const void *pushConstants, uint32_t pushConstantsSize);

	VulkanDescriptorSet *getInput(VulkanPPRenderPassSetup *passSetup, const std::vector<PPTextureInput> &textures);
	VulkanFramebuffer *getOutput(VulkanPPRenderPassSetup *passSetup, const PPOutput &output, int &framebufferWidth, int &framebufferHeight);

	VulkanPPShader *getVulkanShader(PPShader *shader);
	VulkanPPTexture *getVulkanTexture(PPTexture *texture);

	struct TextureImage
	{
		VulkanImage *image;
		VulkanImageView *view;
		VkImageLayout *layout;
		const char *debugname;
	};
	TextureImage getTexture(const PPTextureType &type, PPTexture *tex);

	VulkanRenderDevice* renderer;
};
