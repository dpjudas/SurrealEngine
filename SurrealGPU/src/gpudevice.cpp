
#include "gpudevice.h"
#include "gpubindgroup.h"
#include "gpubuffer.h"
#include "gpucommands.h"
#include "gpupipeline.h"
#include "gpuqueryset.h"
#include "gputexture.h"

GPUDevice::GPUDevice()
{
	graphicsQueue = std::make_shared<GPUQueue>(this);
}

GPUDevice::~GPUDevice()
{
}

std::shared_ptr<GPUBindGroup> GPUDevice::createBindGroup(const GPUBindGroupDesc& desc)
{
	return std::make_shared<GPUBindGroup>(this, desc);
}

std::shared_ptr<GPUBindGroupLayout> GPUDevice::createBindGroupLayout(const GPUBindGroupLayoutDesc& desc)
{
	return std::make_shared<GPUBindGroupLayout>(this, desc);
}

std::shared_ptr<GPUPipelineLayout> GPUDevice::createPipelineLayout(const GPUPipelineLayoutDesc& desc)
{
	return std::make_shared<GPUPipelineLayout>(this, desc);
}

std::shared_ptr<GPUComputePipeline> GPUDevice::createComputePipeline(const GPUComputePipelineDesc& desc)
{
	return std::make_shared<GPUComputePipeline>(this, desc);
}

std::shared_ptr<GPURenderPipeline> GPUDevice::createRenderPipeline(const GPURenderPipelineDesc& desc)
{
	return std::make_shared<GPURenderPipeline>(this, desc);
}

std::shared_ptr<GPUBuffer> GPUDevice::createBuffer(const GPUBufferDesc& desc)
{
	return std::make_shared<GPUBuffer>(this, desc);
}

std::shared_ptr<GPUQuerySet> GPUDevice::createQuerySet(const GPUQuerySetDesc& desc)
{
	return std::make_shared<GPUQuerySet>(this, desc);
}

std::shared_ptr<GPUSampler> GPUDevice::createSampler(const GPUSamplerDesc& desc)
{
	return std::make_shared<GPUSampler>(this, desc);
}

std::shared_ptr<GPUTexture> GPUDevice::createTexture(const GPUTextureDesc& desc)
{
	return std::make_shared<GPUTexture>(this, desc);
}

std::shared_ptr<GPUExternalTexture> GPUDevice::importExternalTexture(const GPUExternalTextureDesc& desc)
{
	return std::make_shared<GPUExternalTexture>(this, desc);
}

std::shared_ptr<GPUCommandEncoder> GPUDevice::createCommandEncoder(const std::string& label)
{
	return std::make_shared<GPUCommandEncoder>(this, label);
}

std::shared_ptr<GPURenderBundleEncoder> GPUDevice::createRenderBundleEncoder(const GPURenderBundleEncoderDesc& desc)
{
	return std::make_shared<GPURenderBundleEncoder>(this, desc);
}
