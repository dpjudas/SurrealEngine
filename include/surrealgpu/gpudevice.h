#pragma once

#include <memory>
#include <vector>
#include <string>

class GPUBindGroup;
class GPUBindGroupLayout;
class GPUBindGroupLayoutDesc;
class GPUBuffer;
class GPUBufferDesc;
class GPUComputePipeline;
class GPUComputePipelineDesc;
class GPUQuerySet;
class GPUQuerySetDesc;
class GPUCommandBuffer;
class GPUCommandEncoder;
class GPURenderBundle;
class GPURenderBundleEncoder;
class GPURenderBundleEncoderDesc;
class GPUPipelineLayout;
class GPUPipelineLayoutDesc;
class GPURenderPipeline;
class GPURenderPipelineDesc;
class GPUSampler;
class GPUSamplerDesc;
class GPUTexture;
class GPUTextureDesc;
class GPUTextureView;
class GPUExternalTexture;
class GPUExternalTextureDesc;
class GPUBindGroupDesc;
class GPUQueue;
class VulkanDevice;

class GPUFeatures
{
public:
};

class GPULimits
{
public:
};

class GPUDevice
{
public:
	GPUDevice();
	~GPUDevice();

	std::shared_ptr<GPUBindGroup> createBindGroup(const GPUBindGroupDesc& desc);
	std::shared_ptr<GPUBindGroupLayout> createBindGroupLayout(const GPUBindGroupLayoutDesc& desc);

	std::shared_ptr<GPUPipelineLayout> createPipelineLayout(const GPUPipelineLayoutDesc& desc);

	std::shared_ptr<GPUComputePipeline> createComputePipeline(const GPUComputePipelineDesc& desc);
	std::shared_ptr<GPURenderPipeline> createRenderPipeline(const GPURenderPipelineDesc& desc);

	std::shared_ptr<GPUBuffer> createBuffer(const GPUBufferDesc& desc);
	std::shared_ptr<GPUQuerySet> createQuerySet(const GPUQuerySetDesc& desc);
	std::shared_ptr<GPUSampler> createSampler(const GPUSamplerDesc& desc);
	std::shared_ptr<GPUTexture> createTexture(const GPUTextureDesc& desc);
	std::shared_ptr<GPUExternalTexture> importExternalTexture(const GPUExternalTextureDesc& desc);

	std::shared_ptr<GPUCommandEncoder> createCommandEncoder(const std::string& label = {});
	std::shared_ptr<GPURenderBundleEncoder> createRenderBundleEncoder(const GPURenderBundleEncoderDesc& desc);

	std::shared_ptr<GPUQueue> queue() { return graphicsQueue; }

	GPUFeatures features;
	GPULimits limits;

	VulkanDevice* getVulkanDevice() { return device.get(); }

private:
	std::unique_ptr<VulkanDevice> device;
	std::shared_ptr<GPUQueue> graphicsQueue;
};
