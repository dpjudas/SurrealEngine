#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include "gpuformat.h"
#include "vulkanobjects.h"

class GPUDevice;
class GPUBuffer;
class GPUExternalTexture;
class GPUSampler;
class GPUTextureView;
class GPUBindGroupLayout;

struct GPUShaderStage
{
	enum
	{
		vertex = 0x1,
		fragment = 0x2,
		compute = 0x4
	};
};
typedef uint32_t GPUShaderStageFlags;

class GPUBufferBinding
{
public:
	std::shared_ptr<GPUBuffer> buffer;
	size_t offset = 0;
	size_t size = 0;
};

class GPUBindGroupBinding
{
public:
	int bindingIndex = 0;
	int arrayIndex = 0;
	struct
	{
		GPUBufferBinding buffer;
		std::shared_ptr<GPUExternalTexture> externalTexture;
		std::shared_ptr<GPUSampler> sampler;
		std::shared_ptr<GPUTextureView> textureView;
	} resource;
};

class GPUBindGroupDesc
{
public:
	std::vector<GPUBindGroupBinding> bindings;
	std::shared_ptr<GPUBindGroupLayout> layout;
	std::string label;
};

class GPUBindGroup
{
public:
	GPUBindGroup(GPUDevice* device, const GPUBindGroupDesc& desc);

	GPUDevice* device = nullptr;
	GPUBindGroupDesc desc;

	std::unique_ptr<VulkanDescriptorSet> descriptorSet;
};

enum class GPUBindGroupLayoutBufferType
{
	uniform,
	readOnlyStorage,
	storage,
};

enum class GPUBindGroupLayoutBindingType
{
	buffer,
	externalTexture,
	sampler,
	storageTexture,
	texture,
	combinedTextureSampler,
};

enum class GPUSamplerType
{
	filtering,
	comparison,
	nonFiltering
};

enum class GPUAccess
{
	readOnly,
	readWrite,
	writeOnly
};

enum class GPUViewDimension
{
	image1D,
	image2D,
	image2DArray,
	imageCube,
	imageCubeArray,
	image3D
};

enum class GPUViewSampleType
{
	float32,
	depth,
	sint,
	uint,
	unfilteredFloat
};

class GPUBindGroupLayoutBinding
{
public:
	int bindingIndex = 0;
	int arrayCount = 1;
	GPUShaderStageFlags visibility = GPUShaderStage::fragment;
	GPUBindGroupLayoutBindingType type = {};
	struct
	{
		bool hasDynamicOffset = false;
		int minBindingSize = 0;
		GPUBindGroupLayoutBufferType type = GPUBindGroupLayoutBufferType::uniform;
	} buffer;
	struct
	{
		GPUSamplerType type = GPUSamplerType::filtering;
	} sampler;
	struct
	{
		GPUAccess access = GPUAccess::readOnly;
		GPUFormat format = {};
		GPUViewDimension viewDimension = GPUViewDimension::image2D;
	} storageTexture;
	struct
	{
		bool multisampled = false;
		GPUViewSampleType sampleType = GPUViewSampleType::float32;
		GPUViewDimension viewDimension = GPUViewDimension::image2D;
	} texture;
};

class GPUBindGroupLayoutDesc
{
public:
	std::vector<GPUBindGroupLayoutBinding> bindings;
	std::string label;
};

class GPUBindGroupLayout
{
public:
	GPUBindGroupLayout(GPUDevice* device, const GPUBindGroupLayoutDesc& desc);

	GPUDevice* device = nullptr;
	GPUBindGroupLayoutDesc desc;

	std::unique_ptr<VulkanDescriptorSetLayout> layout;
	std::unique_ptr<VulkanDescriptorPool> pool;
};
