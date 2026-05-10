#pragma once

#include <vector>
#include <string>
#include <memory>
#include "vulkanobjects.h"
#include "gpuformat.h"

class GPUDevice;
class GPUTexture;
class GPUPromise;
class GPUBuffer;
class GPUCommandEncoder;
class GPUCommandBuffer;

class GPUPromise
{
public:
};

class GPUExternalCopySrc
{
public:
	struct
	{
		// external image object
	} source;
	bool flipY = false;
};

class GPUExternalCopyDest
{
public:
	// aspect
	// colorspace
	int miplevel = 0;
	bool premultipliedAlpha = false;
	std::shared_ptr<GPUTexture> texture;
};

class GPUExternalCopyRegion
{
public:
	struct
	{
		int x = 0;
		int y = 0;
	} src, dest;
	int width = 1;
	int height = 1;
	int depthOrArrayLayers = 1;
};

class GPUWriteTextureDestination
{
public:
};

class GPUQueue
{
public:
	GPUQueue(GPUDevice* device);

	void submit(const std::vector<std::shared_ptr<GPUCommandBuffer>>& commandBuffers);
	GPUPromise onSubmittedWorkDone();

	void copyExternalImageToTexture(const GPUExternalCopySrc& src, const GPUExternalCopyDest& dest, const std::vector<GPUExternalCopyRegion>& regions);
	void writeBuffer(const std::shared_ptr<GPUBuffer>& buffer, size_t dstOffset, const void* data, size_t size);
	void writeTexture(const GPUWriteTextureDestination& dest, const void* data, size_t size);

	GPUDevice* device = nullptr;
};

class GPURenderBundleEncoderDesc
{
public:
	std::vector<GPUFormat> colorFormats;
	bool depthOnly = false;
	GPUFormat depthStencilFormat = {};
	int sampleCount = 0;
	bool stencilReadOnly = false;
	std::string label;
};

class GPURenderBundle
{
public:
	GPURenderBundle(GPUDevice* device, std::unique_ptr<VulkanCommandBuffer> cmdbuffer);

	GPUDevice* device = nullptr;

	std::unique_ptr<VulkanCommandBuffer> cmdbuffer;
};

class GPURenderBundleEncoder
{
public:
	GPURenderBundleEncoder(GPUDevice* device, const GPURenderBundleEncoderDesc& desc);

	void setPipeline();
	void setBindGroup();
	void setVertexBuffer();
	void setIndexBuffer();

	void draw();
	void drawIndexed();
	void drawIndirect();
	void drawIndexedIndirect();

	void insertDebugMarker();
	void pushDebugGroup();
	void popDebugGroup();

	std::shared_ptr<GPURenderBundle> finish();

	GPUDevice* device = nullptr;
	GPURenderBundleEncoderDesc desc;

	std::unique_ptr<VulkanCommandBuffer> cmdbuffer;
};

class GPURenderPassEncoder
{
public:
	GPURenderPassEncoder(GPUCommandEncoder* commandEncoder);

	void beginOcclusionQuery();
	void endOcclusionQuery();

	void setPipeline();
	void setBindGroup();
	void setVertexBuffer();
	void setIndexBuffer();

	void setBlendConstant();
	void setScissorRect();
	void setStencilReference();
	void setViewport();

	void draw();
	void drawIndexed();
	void drawIndirect();

	void executeBundles();

	void insertDebugMarker();
	void pushDebugGroup();
	void popDebugGroup();

	void end();

private:
	GPUCommandEncoder* commandEncoder = nullptr;
};

class GPUComputePassEncoder
{
public:
	GPUComputePassEncoder(GPUCommandEncoder* commandEncoder);

	void setPipeline();
	void setBindGroup();

	void dispatchWorkgroups();
	void dispatchWorkgroupsIndirect();

	void insertDebugMarker();
	void pushDebugGroup();
	void popDebugGroup();

	void end();

private:
	GPUCommandEncoder* commandEncoder = nullptr;
};

class GPUCommandEncoder
{
public:
	GPUCommandEncoder(GPUDevice* device, const std::string& label);

	std::shared_ptr<GPURenderPassEncoder> beginRenderPass();
	std::shared_ptr<GPUComputePassEncoder> beginComputePass();

	void clearBuffer();

	void copyBufferToBuffer();
	void copyBufferToTexture();
	void copyTextureToBuffer();
	void copyTextureToTexture();

	void insertDebugMarker();
	void pushDebugGroup();
	void popDebugGroup();

	void resolveQuerySet();

	std::shared_ptr<GPUCommandBuffer> finish();

	GPUDevice* device = nullptr;
	std::string label;

	std::unique_ptr<VulkanCommandBuffer> cmdbuffer;
};

class GPUCommandBuffer
{
public:
	GPUCommandBuffer(GPUDevice* device, std::unique_ptr<VulkanCommandBuffer> cmdbuffer);

	GPUDevice* device = nullptr;
	std::unique_ptr<VulkanCommandBuffer> cmdbuffer;
};
