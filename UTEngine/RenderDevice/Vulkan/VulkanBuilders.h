#pragma once

#include "vulkanobjects.h"
#include <cassert>

class ImageBuilder
{
public:
	ImageBuilder();

	void setSize(int width, int height, int miplevels = 1);
	void setSamples(VkSampleCountFlagBits samples);
	void setFormat(VkFormat format);
	void setUsage(VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY, VmaAllocationCreateFlags allocFlags = 0);
	void setMemoryType(VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, uint32_t memoryTypeBits = 0);
	void setLinearTiling();

	bool isFormatSupported(VulkanDevice *device);

	std::unique_ptr<VulkanImage> create(VulkanDevice *device);

private:
	VkImageCreateInfo imageInfo = {};
	VmaAllocationCreateInfo allocInfo = {};
};

class ImageViewBuilder
{
public:
	ImageViewBuilder();

	void setImage(VulkanImage *image, VkFormat format, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

	std::unique_ptr<VulkanImageView> create(VulkanDevice *device);

private:
	VkImageViewCreateInfo viewInfo = {};
};

class SamplerBuilder
{
public:
	SamplerBuilder();

	void setAddressMode(VkSamplerAddressMode addressMode);
	void setAddressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w);
	void setMinFilter(VkFilter minFilter);
	void setMagFilter(VkFilter magFilter);
	void setMipmapMode(VkSamplerMipmapMode mode);
	void setAnisotropy(float maxAnisotropy);
	void setMaxLod(float value);

	std::unique_ptr<VulkanSampler> create(VulkanDevice *device);

private:
	VkSamplerCreateInfo samplerInfo = {};
};

class BufferBuilder
{
public:
	BufferBuilder();

	void setSize(size_t size);
	void setUsage(VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY, VmaAllocationCreateFlags allocFlags = 0);
	void setMemoryType(VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags, uint32_t memoryTypeBits = 0);

	std::unique_ptr<VulkanBuffer> create(VulkanDevice *device);

private:
	VkBufferCreateInfo bufferInfo = {};
	VmaAllocationCreateInfo allocInfo = {};
};

class ShaderBuilder
{
public:
	ShaderBuilder();

	static void init();
	static void deinit();

	void setVertexShader(const std::string &code);
	void setTessControlShader(const std::string &code);
    void setTessEvaluationShader(const std::string &code);
    void setGeometryShader(const std::string &code);
    void setFragmentShader(const std::string &code);
    void setComputeShader(const std::string &code);
    void setRayGenShader(const std::string &code);
    void setIntersectShader(const std::string &code);
    void setAnyHitShader(const std::string &code);
    void setClosestHitShader(const std::string &code);
    void setMissShader(const std::string &code);
    void setCallableShader(const std::string &code);

	std::unique_ptr<VulkanShader> create(VulkanDevice *device);

private:
	std::string code;
	int stage = 0;
};

class ComputePipelineBuilder
{
public:
	ComputePipelineBuilder();

	void setLayout(VulkanPipelineLayout *layout);
	void setComputeShader(VulkanShader *shader);

	std::unique_ptr<VulkanPipeline> create(VulkanDevice *device);

private:
	VkComputePipelineCreateInfo pipelineInfo = {};
	VkPipelineShaderStageCreateInfo stageInfo = {};
};

class DescriptorSetLayoutBuilder
{
public:
	DescriptorSetLayoutBuilder();

	void addBinding(int binding, VkDescriptorType type, int arrayCount, VkShaderStageFlags stageFlags);

	std::unique_ptr<VulkanDescriptorSetLayout> create(VulkanDevice *device);

private:
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

class DescriptorPoolBuilder
{
public:
	DescriptorPoolBuilder();

	void setMaxSets(int value);
	void addPoolSize(VkDescriptorType type, int count);

	std::unique_ptr<VulkanDescriptorPool> create(VulkanDevice *device);

private:
	std::vector<VkDescriptorPoolSize> poolSizes;
	VkDescriptorPoolCreateInfo poolInfo = {};
};

class QueryPoolBuilder
{
public:
	QueryPoolBuilder();

	void setQueryType(VkQueryType type, int count, VkQueryPipelineStatisticFlags pipelineStatistics = 0);

	std::unique_ptr<VulkanQueryPool> create(VulkanDevice *device);

private:
	VkQueryPoolCreateInfo poolInfo = {};
};

class FramebufferBuilder
{
public:
	FramebufferBuilder();

	void setRenderPass(VulkanRenderPass *renderPass);
	void addAttachment(VulkanImageView *view);
	void addAttachment(VkImageView view);
	void setSize(int width, int height, int layers = 1);

	std::unique_ptr<VulkanFramebuffer> create(VulkanDevice *device);

private:
	VkFramebufferCreateInfo framebufferInfo = {};
	std::vector<VkImageView> attachments;
};

union FRenderStyle;

class GraphicsPipelineBuilder
{
public:
	GraphicsPipelineBuilder();

	void setSubpass(int subpass);
	void setLayout(VulkanPipelineLayout *layout);
	void setRenderPass(VulkanRenderPass *renderPass);
	void setTopology(VkPrimitiveTopology topology);
	void setViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
	void setScissor(int x, int y, int width, int height);
	void setRasterizationSamples(VkSampleCountFlagBits samples);

	void setCull(VkCullModeFlags cullMode, VkFrontFace frontFace);
	void setDepthStencilEnable(bool test, bool write, bool stencil);
	void setDepthFunc(VkCompareOp func);
	void setDepthClampEnable(bool value);
	void setDepthBias(bool enable, float biasConstantFactor, float biasClamp, float biasSlopeFactor);
	void setColorWriteMask(VkColorComponentFlags mask);
	void setStencil(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp, uint32_t compareMask, uint32_t writeMask, uint32_t reference);

	void setAdditiveBlendMode();
	void setAlphaBlendMode();
	void setBlendMode(VkBlendOp op, VkBlendFactor src, VkBlendFactor dst);
	void setSubpassColorAttachmentCount(int count);

	void addVertexShader(VulkanShader *shader);
	void addFragmentShader(VulkanShader *shader);

	void addVertexBufferBinding(int index, size_t stride);
	void addVertexAttribute(int location, int binding, VkFormat format, size_t offset);

	void addDynamicState(VkDynamicState state);

	std::unique_ptr<VulkanPipeline> create(VulkanDevice *device);

private:
	VkGraphicsPipelineCreateInfo pipelineInfo = { };
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { };
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { };
	VkViewport viewport = { };
	VkRect2D scissor = { };
	VkPipelineViewportStateCreateInfo viewportState = { };
	VkPipelineRasterizationStateCreateInfo rasterizer = { };
	VkPipelineMultisampleStateCreateInfo multisampling = { };
	VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
	VkPipelineColorBlendStateCreateInfo colorBlending = { };
	VkPipelineDepthStencilStateCreateInfo depthStencil = { };
	VkPipelineDynamicStateCreateInfo dynamicState = {};

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	std::vector<VkVertexInputBindingDescription> vertexInputBindings;
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
	std::vector<VkDynamicState> dynamicStates;
};

class PipelineLayoutBuilder
{
public:
	PipelineLayoutBuilder();

	void addSetLayout(VulkanDescriptorSetLayout *setLayout);
	void addPushConstantRange(VkShaderStageFlags stageFlags, size_t offset, size_t size);

	std::unique_ptr<VulkanPipelineLayout> create(VulkanDevice *device);

private:
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkPushConstantRange> pushConstantRanges;
};

class RenderPassBuilder
{
public:
	RenderPassBuilder();

	void addAttachment(VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp load, VkAttachmentStoreOp store, VkImageLayout initialLayout, VkImageLayout finalLayout);
	void addDepthStencilAttachment(VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp load, VkAttachmentStoreOp store, VkAttachmentLoadOp stencilLoad, VkAttachmentStoreOp stencilStore, VkImageLayout initialLayout, VkImageLayout finalLayout);

	void addExternalSubpassDependency(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	void addSubpass();
	void addSubpassColorAttachmentRef(uint32_t index, VkImageLayout layout);
	void addSubpassDepthStencilAttachmentRef(uint32_t index, VkImageLayout layout);

	std::unique_ptr<VulkanRenderPass> create(VulkanDevice *device);

private:
	VkRenderPassCreateInfo renderPassInfo = { };

	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkSubpassDependency> dependencies;
	std::vector<VkSubpassDescription> subpasses;

	struct SubpassData
	{
		std::vector<VkAttachmentReference> colorRefs;
		VkAttachmentReference depthRef = { };
	};

	std::vector<std::unique_ptr<SubpassData>> subpassData;
};

class PipelineBarrier
{
public:
	void addMemory(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void addBuffer(VulkanBuffer *buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void addBuffer(VulkanBuffer *buffer, VkDeviceSize offset, VkDeviceSize size, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void addImage(VulkanImage *image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, int baseMipLevel = 0, int levelCount = 1);
	void addImage(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, int baseMipLevel = 0, int levelCount = 1);
	void addQueueTransfer(int srcFamily, int dstFamily, VulkanBuffer *buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
	void addQueueTransfer(int srcFamily, int dstFamily, VulkanImage *image, VkImageLayout layout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, int baseMipLevel = 0, int levelCount = 1);

	void execute(VulkanCommandBuffer *commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags = 0);

private:
	std::vector<VkMemoryBarrier> memoryBarriers;
	std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
	std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
};

class QueueSubmit
{
public:
	QueueSubmit();

	void addCommandBuffer(VulkanCommandBuffer *buffer);
	void addWait(VkPipelineStageFlags waitStageMask, VulkanSemaphore *semaphore);
	void addSignal(VulkanSemaphore *semaphore);
	void execute(VulkanDevice *device, VkQueue queue, VulkanFence *fence = nullptr);

private:
	VkSubmitInfo submitInfo = {};
	std::vector<VkSemaphore> waitSemaphores;
	std::vector<VkPipelineStageFlags> waitStages;
	std::vector<VkSemaphore> signalSemaphores;
	std::vector<VkCommandBuffer> commandBuffers;
};

class WriteDescriptors
{
public:
	void addBuffer(VulkanDescriptorSet *descriptorSet, int binding, VkDescriptorType type, VulkanBuffer *buffer);
	void addBuffer(VulkanDescriptorSet *descriptorSet, int binding, VkDescriptorType type, VulkanBuffer *buffer, size_t offset, size_t range);
	void addStorageImage(VulkanDescriptorSet *descriptorSet, int binding, VulkanImageView *view, VkImageLayout imageLayout);
	void addCombinedImageSampler(VulkanDescriptorSet *descriptorSet, int binding, VulkanImageView *view, VulkanSampler *sampler, VkImageLayout imageLayout);
	void addAccelerationStructure(VulkanDescriptorSet *descriptorSet, int binding, VulkanAccelerationStructure *accelStruct);

	void updateSets(VulkanDevice *device);

private:
	struct WriteExtra
	{
		VkDescriptorImageInfo imageInfo;
		VkDescriptorBufferInfo bufferInfo;
		VkBufferView bufferView;
		VkWriteDescriptorSetAccelerationStructureKHR accelStruct;
	};

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<std::unique_ptr<WriteExtra>> writeExtras;
};

/////////////////////////////////////////////////////////////////////////////

inline void PipelineBarrier::addMemory(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	memoryBarriers.push_back(barrier);
}

inline void PipelineBarrier::addBuffer(VulkanBuffer *buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	addBuffer(buffer, 0, buffer->size, srcAccessMask, dstAccessMask);
}

inline void PipelineBarrier::addBuffer(VulkanBuffer *buffer, VkDeviceSize offset, VkDeviceSize size, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkBufferMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer->buffer;
	barrier.offset = offset;
	barrier.size = size;
	bufferMemoryBarriers.push_back(barrier);
}

inline void PipelineBarrier::addImage(VulkanImage *image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask, int baseMipLevel, int levelCount)
{
	addImage(image->image, oldLayout, newLayout, srcAccessMask, dstAccessMask, aspectMask, baseMipLevel, levelCount);
}

inline void PipelineBarrier::addImage(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageAspectFlags aspectMask, int baseMipLevel, int levelCount)
{
	VkImageMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspectMask;
	barrier.subresourceRange.baseMipLevel = baseMipLevel;
	barrier.subresourceRange.levelCount = levelCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	imageMemoryBarriers.push_back(barrier);
}

inline void PipelineBarrier::addQueueTransfer(int srcFamily, int dstFamily, VulkanBuffer *buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
	VkBufferMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.srcQueueFamilyIndex = srcFamily;
	barrier.dstQueueFamilyIndex = dstFamily;
	barrier.buffer = buffer->buffer;
	barrier.offset = 0;
	barrier.size = buffer->size;
	bufferMemoryBarriers.push_back(barrier);
}

inline void PipelineBarrier::addQueueTransfer(int srcFamily, int dstFamily, VulkanImage *image, VkImageLayout layout, VkImageAspectFlags aspectMask, int baseMipLevel, int levelCount)
{
	VkImageMemoryBarrier barrier = { };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = layout;
	barrier.newLayout = layout;
	barrier.srcQueueFamilyIndex = srcFamily;
	barrier.dstQueueFamilyIndex = dstFamily;
	barrier.image = image->image;
	barrier.subresourceRange.aspectMask = aspectMask;
	barrier.subresourceRange.baseMipLevel = baseMipLevel;
	barrier.subresourceRange.levelCount = levelCount;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	imageMemoryBarriers.push_back(barrier);
}

inline void PipelineBarrier::execute(VulkanCommandBuffer *commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags)
{
	commandBuffer->pipelineBarrier(
		srcStageMask, dstStageMask, dependencyFlags,
		(uint32_t)memoryBarriers.size(), memoryBarriers.data(),
		(uint32_t)bufferMemoryBarriers.size(), bufferMemoryBarriers.data(),
		(uint32_t)imageMemoryBarriers.size(), imageMemoryBarriers.data());
}

/////////////////////////////////////////////////////////////////////////////

inline QueueSubmit::QueueSubmit()
{
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
}

inline void QueueSubmit::addCommandBuffer(VulkanCommandBuffer *buffer)
{
	commandBuffers.push_back(buffer->buffer);
	submitInfo.pCommandBuffers = commandBuffers.data();
	submitInfo.commandBufferCount = (uint32_t)commandBuffers.size();
}

inline void QueueSubmit::addWait(VkPipelineStageFlags waitStageMask, VulkanSemaphore *semaphore)
{
	waitStages.push_back(waitStageMask);
	waitSemaphores.push_back(semaphore->semaphore);

	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.pWaitSemaphores = waitSemaphores.data();
	submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
}

inline void QueueSubmit::addSignal(VulkanSemaphore *semaphore)
{
	signalSemaphores.push_back(semaphore->semaphore);
	submitInfo.pSignalSemaphores = signalSemaphores.data();
	submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
}

inline void QueueSubmit::execute(VulkanDevice *device, VkQueue queue, VulkanFence *fence)
{
	VkResult result = vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, fence ? fence->fence : VK_NULL_HANDLE);
	if (result < VK_SUCCESS)
		throw std::runtime_error("Failed to submit command buffer");
}

/////////////////////////////////////////////////////////////////////////////

inline void WriteDescriptors::addBuffer(VulkanDescriptorSet *descriptorSet, int binding, VkDescriptorType type, VulkanBuffer *buffer)
{
	addBuffer(descriptorSet, binding, type, buffer, 0, buffer->size);
}

inline void WriteDescriptors::addBuffer(VulkanDescriptorSet *descriptorSet, int binding, VkDescriptorType type, VulkanBuffer *buffer, size_t offset, size_t range)
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer->buffer;
	bufferInfo.offset = offset;
	bufferInfo.range = range;

	auto extra = std::make_unique<WriteExtra>();
	extra->bufferInfo = bufferInfo;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet->set;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &extra->bufferInfo;
	writes.push_back(descriptorWrite);
	writeExtras.push_back(std::move(extra));
}

inline void WriteDescriptors::addStorageImage(VulkanDescriptorSet *descriptorSet, int binding, VulkanImageView *view, VkImageLayout imageLayout)
{
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = view->view;
	imageInfo.imageLayout = imageLayout;

	auto extra = std::make_unique<WriteExtra>();
	extra->imageInfo = imageInfo;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet->set;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &extra->imageInfo;
	writes.push_back(descriptorWrite);
	writeExtras.push_back(std::move(extra));
}

inline void WriteDescriptors::addCombinedImageSampler(VulkanDescriptorSet *descriptorSet, int binding, VulkanImageView *view, VulkanSampler *sampler, VkImageLayout imageLayout)
{
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = view->view;
	imageInfo.sampler = sampler->sampler;
	imageInfo.imageLayout = imageLayout;

	auto extra = std::make_unique<WriteExtra>();
	extra->imageInfo = imageInfo;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet->set;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &extra->imageInfo;
	writes.push_back(descriptorWrite);
	writeExtras.push_back(std::move(extra));
}

inline void WriteDescriptors::addAccelerationStructure(VulkanDescriptorSet *descriptorSet, int binding, VulkanAccelerationStructure *accelStruct)
{
	auto extra = std::make_unique<WriteExtra>();
	extra->accelStruct = {};
	extra->accelStruct.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	extra->accelStruct.accelerationStructureCount = 1;
	extra->accelStruct.pAccelerationStructures = &accelStruct->accelstruct;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet->set;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pNext = &extra->accelStruct;
	writes.push_back(descriptorWrite);
	writeExtras.push_back(std::move(extra));
}

inline void WriteDescriptors::updateSets(VulkanDevice *device)
{
	vkUpdateDescriptorSets(device->device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}
