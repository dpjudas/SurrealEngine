
#include "gpucommands.h"
#include "gpudevice.h"

GPUQueue::GPUQueue(GPUDevice* device) : device(device)
{
}

void GPUQueue::submit(const std::vector<std::shared_ptr<GPUCommandBuffer>>& commandBuffers)
{
}

GPUPromise GPUQueue::onSubmittedWorkDone()
{
	return {};
}

void GPUQueue::copyExternalImageToTexture(const GPUExternalCopySrc& src, const GPUExternalCopyDest& dest, const std::vector<GPUExternalCopyRegion>& regions)
{
}

void GPUQueue::writeBuffer(const std::shared_ptr<GPUBuffer>& buffer, size_t dstOffset, const void* data, size_t size)
{
}

void GPUQueue::writeTexture(const GPUWriteTextureDestination& dest, const void* data, size_t size)
{
}

/////////////////////////////////////////////////////////////////////////////

GPUCommandEncoder::GPUCommandEncoder(GPUDevice* device, const std::string& label) : device(device), label(label)
{
	cmdbuffer->begin();
}

std::shared_ptr<GPURenderPassEncoder> GPUCommandEncoder::beginRenderPass()
{
	return std::make_shared<GPURenderPassEncoder>(this);
}

std::shared_ptr<GPUComputePassEncoder> GPUCommandEncoder::beginComputePass()
{
	return std::make_shared<GPUComputePassEncoder>(this);
}

void GPUCommandEncoder::clearBuffer()
{
	// cmdbuffer->clearColorImage();
	// cmdbuffer->clearDepthStencilImage();
	// cmdbuffer->clearAttachments();
}

void GPUCommandEncoder::copyBufferToBuffer()
{
	// cmdbuffer->copyBuffer();
}

void GPUCommandEncoder::copyBufferToTexture()
{
	// cmdbuffer->copyBufferToImage();
}

void GPUCommandEncoder::copyTextureToBuffer()
{
	// cmdbuffer->copyImageToBuffer();
}

void GPUCommandEncoder::copyTextureToTexture()
{
	// cmdbuffer->copyImage();
}

void GPUCommandEncoder::insertDebugMarker()
{
}

void GPUCommandEncoder::pushDebugGroup()
{
}

void GPUCommandEncoder::popDebugGroup()
{
}

void GPUCommandEncoder::resolveQuerySet()
{
	//cmdbuffer->copyQueryPoolResults();
}

std::shared_ptr<GPUCommandBuffer> GPUCommandEncoder::finish()
{
	cmdbuffer->end();
	return std::make_shared<GPUCommandBuffer>(device, std::move(cmdbuffer));
}

/////////////////////////////////////////////////////////////////////////////

GPURenderPassEncoder::GPURenderPassEncoder(GPUCommandEncoder* commandEncoder) : commandEncoder(commandEncoder)
{
	// commandEncoder->cmdbuffer->beginRenderPass();
}

void GPURenderPassEncoder::beginOcclusionQuery()
{
	// commandEncoder->cmdbuffer->beginQuery();
}

void GPURenderPassEncoder::endOcclusionQuery()
{
	// commandEncoder->cmdbuffer->endQuery();
}

void GPURenderPassEncoder::setPipeline()
{
	// commandEncoder->cmdbuffer->bindPipeline();
}

void GPURenderPassEncoder::setBindGroup()
{
	// commandEncoder->cmdbuffer->bindDescriptorSet();
}

void GPURenderPassEncoder::setVertexBuffer()
{
	// commandEncoder->cmdbuffer->bindVertexBuffers();
}

void GPURenderPassEncoder::setIndexBuffer()
{
	// commandEncoder->cmdbuffer->bindIndexBuffer();
}

void GPURenderPassEncoder::setBlendConstant()
{
	// commandEncoder->cmdbuffer->setBlendConstants();
}

void GPURenderPassEncoder::setScissorRect()
{
	// commandEncoder->cmdbuffer->setScissor();
}

void GPURenderPassEncoder::setStencilReference()
{
	// commandEncoder->cmdbuffer->setStencilReference();
}

void GPURenderPassEncoder::setViewport()
{
	// commandEncoder->cmdbuffer->setViewport();
}

void GPURenderPassEncoder::draw()
{
	// commandEncoder->cmdbuffer->draw();
}

void GPURenderPassEncoder::drawIndexed()
{
	// commandEncoder->cmdbuffer->drawIndexed();
}

void GPURenderPassEncoder::drawIndirect()
{
	// commandEncoder->cmdbuffer->drawIndirect();
}

void GPURenderPassEncoder::executeBundles()
{
	// commandEncoder->cmdbuffer->executeCommands();
}

void GPURenderPassEncoder::insertDebugMarker()
{
}

void GPURenderPassEncoder::pushDebugGroup()
{
}

void GPURenderPassEncoder::popDebugGroup()
{
}

void GPURenderPassEncoder::end()
{
	commandEncoder->cmdbuffer->endRenderPass();
}

/////////////////////////////////////////////////////////////////////////////

GPUComputePassEncoder::GPUComputePassEncoder(GPUCommandEncoder* commandEncoder) : commandEncoder(commandEncoder)
{
}

void GPUComputePassEncoder::setPipeline()
{
	// commandEncoder->cmdbuffer->bindPipeline();
}

void GPUComputePassEncoder::setBindGroup()
{
	// commandEncoder->cmdbuffer->bindDescriptorSet();
}

void GPUComputePassEncoder::dispatchWorkgroups()
{
	// commandEncoder->cmdbuffer->dispatch();
}

void GPUComputePassEncoder::dispatchWorkgroupsIndirect()
{
	// commandEncoder->cmdbuffer->dispatchIndirect();
}

void GPUComputePassEncoder::insertDebugMarker()
{
}

void GPUComputePassEncoder::pushDebugGroup()
{
}

void GPUComputePassEncoder::popDebugGroup()
{
}

void GPUComputePassEncoder::end()
{
}

/////////////////////////////////////////////////////////////////////////////

GPURenderBundleEncoder::GPURenderBundleEncoder(GPUDevice* device, const GPURenderBundleEncoderDesc& desc)
{
}

void GPURenderBundleEncoder::setPipeline()
{
	// cmdbuffer->bindPipeline();
}

void GPURenderBundleEncoder::setBindGroup()
{
	// cmdbuffer->bindDescriptorSet();
}

void GPURenderBundleEncoder::setVertexBuffer()
{
	// cmdbuffer->bindVertexBuffers();
}

void GPURenderBundleEncoder::setIndexBuffer()
{
	// cmdbuffer->bindIndexBuffer();
}

void GPURenderBundleEncoder::draw()
{
	// cmdbuffer->draw();
}

void GPURenderBundleEncoder::drawIndexed()
{
	// cmdbuffer->drawIndexed();
}

void GPURenderBundleEncoder::drawIndirect()
{
	// cmdbuffer->drawIndirect();
}

void GPURenderBundleEncoder::drawIndexedIndirect()
{
	// cmdbuffer->drawIndexedIndirect();
}

void GPURenderBundleEncoder::insertDebugMarker()
{
}

void GPURenderBundleEncoder::pushDebugGroup()
{
}

void GPURenderBundleEncoder::popDebugGroup()
{
}

std::shared_ptr<GPURenderBundle> GPURenderBundleEncoder::finish()
{
	cmdbuffer->end();
	return std::make_shared<GPURenderBundle>(device, std::move(cmdbuffer));
}

/////////////////////////////////////////////////////////////////////////////

GPUCommandBuffer::GPUCommandBuffer(GPUDevice* device, std::unique_ptr<VulkanCommandBuffer> cmdbuffer) : device(device), cmdbuffer(std::move(cmdbuffer))
{
}

/////////////////////////////////////////////////////////////////////////////

GPURenderBundle::GPURenderBundle(GPUDevice* device, std::unique_ptr<VulkanCommandBuffer> cmdbuffer) : device(device), cmdbuffer(std::move(cmdbuffer))
{
}
