#pragma once

#include <string>
#include <vector>
#include <memory>
#include "gpuformat.h"
#include "vulkanobjects.h"

class GPUDevice;
class GPUBindGroupLayout;

class GPUPipelineLayoutDesc
{
public:
	std::vector<std::shared_ptr<GPUBindGroupLayout>> bindGroupLayouts;
	std::string label;
};

class GPUPipelineLayout
{
public:
	GPUPipelineLayout(GPUDevice* device, const GPUPipelineLayoutDesc& desc);

	GPUDevice* device = nullptr;
	GPUPipelineLayoutDesc desc;

	std::unique_ptr<VulkanPipelineLayout> layout;
};

class GPUComputePipelineDesc
{
public:
	std::shared_ptr<GPUPipelineLayout> layout;
	std::vector<uint32_t> computeShader;
	std::string label;
};

class GPUComputePipeline
{
public:
	GPUComputePipeline(GPUDevice* device, const GPUComputePipelineDesc& desc);

	std::shared_ptr<GPUBindGroupLayout> getBindLayout(int index);

	GPUDevice* device = nullptr;
	GPUComputePipelineDesc desc;

	std::unique_ptr<VulkanPipeline> pipeline;
};

enum class GPUCompareOp
{
	never,
	less,
	equal,
	lessOrEqual,
	greater,
	notEqual,
	greaterOrEqual,
	always
};

enum class GPUStencilOp
{
	keep,
	zero,
	replace,
	incrementAndClamp,
	decrementAndClamp,
	invert,
	incrementAndWrap,
	decrementAndWrap
};

class GPUDepthStencilDesc
{
public:
	bool depthTestEnable = false;
	bool depthWriteEnable = false;
	bool stencilTestEnable = false;
	bool depthBoundsTestEnable = false;

	GPUCompareOp depthCompareOp = GPUCompareOp::lessOrEqual;
	float minDepthBounds = 0.0f;
	float maxDepthBounds = 1.0f;

	struct
	{
		GPUStencilOp failOp = GPUStencilOp::keep;
		GPUStencilOp passOp = GPUStencilOp::keep;
		GPUStencilOp depthFailOp = GPUStencilOp::keep;
		GPUCompareOp compareOp = GPUCompareOp::never;
		uint32_t compareMask = 0;
		uint32_t writeMask = 0;
		uint32_t reference = 0;
	} front, back;
};

class GPUMultisampleDesc
{
public:
	bool alphaToCoverageEnable = false;
	bool alphaToOneEnable = false;
	int rasterizationSamples = 1;
};

enum class GPUPrimitiveTopology
{
	pointList,
	lineList,
	lineStrip,
	triangleList,
	triangleStrip,
	triangleFan,
};

enum class GPUPolygonMode
{
	fill,
	line,
	point,
};

enum class GPUCullMode
{
	none,
	front,
	back,
	frontAndBack
};

enum class GPUFrontFace
{
	counterClockwise,
	clockwise
};

class GPUPrimitiveDesc
{
public:
	GPUPrimitiveTopology topology = GPUPrimitiveTopology::triangleList;
	bool primitiveRestartEnable = false;
	bool depthClampEnable = false;
	bool rasterizerDiscardEnable = false;
	GPUPolygonMode polygonMode = GPUPolygonMode::fill;
	float lineWidth = 1.0f;
	GPUCullMode cullMode = GPUCullMode::none;
	GPUFrontFace frontFace = GPUFrontFace::clockwise;
	bool depthBiasEnable = false;
	float depthBiasConstantFactor = 0.0f;
	float depthBiasClamp = 0.0f;
	float depthBiasSlopeFactor = 0.0f;
};

class GPUVertexBufferBinding
{
public:
	uint32_t bindIndex = 0;
	uint32_t stride = 0;
};

class GPUVertexAttribute
{
public:
	uint32_t location = 0;
	uint32_t binding = 0;
	GPUFormat format = {};
	uint32_t offset = 0;
};

class GPUVertexDesc
{
public:
	std::vector<GPUVertexBufferBinding> buffers;
	std::vector<GPUVertexAttribute> attributes;
	// to do: constants
	std::vector<uint32_t> shader;
};

enum class GPUBlendFactor
{
	zero,
	one,
	srcColor,
	oneMinusSrcColor,
	dstColor,
	oneMinusDstColor,
	srcAlpha,
	oneMinusSrcAlpha,
	dstAlpha,
	oneMinusDstAlpha,
	constantColor,
	oneMinusConstantColor,
	constantAlpha,
	oneMinusConstantAlpha,
	srcAlphaSaturate,
	src1Color,
	oneMinusSrc1Color,
	src1Alpha,
	oneMinusSrc1Alpha,
};

enum class GPUBlendOp
{
	add,
	subtract,
	reverseSubtract,
	min,
	max,
};

class GPUFragmentTarget
{
public:
	struct
	{
		bool red = true;
		bool green = true;
		bool blue = true;
		bool alpha = true;
	} colorWriteMask;
	bool blendEnable = false;
	GPUBlendFactor srcColorBlendFactor = GPUBlendFactor::one;
	GPUBlendFactor dstColorBlendFactor = GPUBlendFactor::zero;
	GPUBlendOp colorBlendOp = GPUBlendOp::add;
	GPUBlendFactor srcAlphaBlendFactor = GPUBlendFactor::one;
	GPUBlendFactor dstAlphaBlendFactor = GPUBlendFactor::zero;
	GPUBlendOp alphaBlendOp = GPUBlendOp::add;
};

class GPUFragmentDesc
{
public:
	// to do: constants
	std::vector<uint32_t> shader;
	std::vector<GPUFragmentTarget> targets;
};

class GPURenderPipelineDesc
{
public:
	std::shared_ptr<GPUPipelineLayout> layout;
	GPUPrimitiveDesc primitive;
	GPUVertexDesc vertex;
	GPUFragmentDesc fragment;
	GPUMultisampleDesc multisample;
	GPUDepthStencilDesc depthStencil;
	std::string label;
};

class GPURenderPipeline
{
public:
	GPURenderPipeline(GPUDevice* device, const GPURenderPipelineDesc& desc);

	std::shared_ptr<GPUBindGroupLayout> getBindLayout(int index);

	GPUDevice* device = nullptr;
	GPURenderPipelineDesc desc;

	std::unique_ptr<VulkanPipeline> pipeline;
};
