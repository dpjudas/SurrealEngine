#pragma once

#include <memory>
#include "vulkanobjects.h"

class GPUDevice;
class GPUTextureView;

enum class GPUSamplerAddressMode
{
	repeat,
	mirroredRepeat,
	clampToEdge,
	clampToBorder,
	mirrorClampToEdge,
};

enum class GPUSamplerFilter
{
	nearest,
	linear,
};

enum class GPUSamplerMipmapMode
{
	nearest,
	linear
};

class GPUSamplerDesc
{
public:
	GPUSamplerAddressMode addressModeU = GPUSamplerAddressMode::repeat;
	GPUSamplerAddressMode addressModeV = GPUSamplerAddressMode::repeat;
	GPUSamplerAddressMode addressModeW = GPUSamplerAddressMode::repeat;
	GPUSamplerFilter minFilter = GPUSamplerFilter::linear;
	GPUSamplerFilter magFilter = GPUSamplerFilter::linear;
	GPUSamplerMipmapMode mipmapMode = GPUSamplerMipmapMode::linear;

	bool anisotropyEnable = false;
	float maxAnisotropy = 1.0f;

	float mipLodBias = 0.0f;
	float minLod = 0.0f;
	float maxLod = 100.0f;

	std::string label;
};

class GPUSampler
{
public:
	GPUSampler(GPUDevice* device, const GPUSamplerDesc& desc);

	// no destroy

	GPUDevice* device = nullptr;
	GPUSamplerDesc desc;

	std::unique_ptr<VulkanSampler> sampler;
};

class GPUTextureDesc
{
public:
};

class GPUTexture
{
public:
	GPUTexture(GPUDevice* device, const GPUTextureDesc& desc);

	std::shared_ptr<GPUTextureView> createView();
	void destroy();

	GPUDevice* device = nullptr;
	GPUTextureDesc desc;

	std::unique_ptr<VulkanImage> image;
};

class GPUTextureView
{
public:
	GPUTextureView(GPUTexture* texture);

	// no destroy

	std::unique_ptr<VulkanImageView> view;
};

class GPUExternalTextureDesc
{
public:
};

class GPUExternalTexture
{
public:
	GPUExternalTexture(GPUDevice* device, const GPUExternalTextureDesc& desc);

	GPUDevice* device = nullptr;
	GPUExternalTextureDesc desc;
};
