
#include "gpupipeline.h"
#include "gpubindgroup.h"
#include "gpudevice.h"
#include "vulkanbuilders.h"

GPUPipelineLayout::GPUPipelineLayout(GPUDevice* device, const GPUPipelineLayoutDesc& desc) : device(device), desc(desc)
{
	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkPushConstantRange> pushConstantRanges;

	for (const std::shared_ptr<GPUBindGroupLayout>& bindGroup : desc.bindGroupLayouts)
	{
		setLayouts.push_back(bindGroup->layout->layout);
	}

	/*
	for (const GPUPushConstantRange& rangeInfo : desc.pushConstantRanges)
	{
		VkPushConstantRange range = { };
		range.stageFlags = rangeInfo.stageFlags;
		range.offset = (uint32_t)rangeInfo.offset;
		range.size = (uint32_t)rangeInfo.size;
		pushConstantRanges.push_back(range);
	}
	*/

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = (uint32_t)setLayouts.size();
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	VkPipelineLayout pipelineLayout;
	VkResult result = vkCreatePipelineLayout(device->getVulkanDevice()->device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
	device->getVulkanDevice()->CheckVulkanError(result, "Could not create pipeline layout");
	layout = std::make_unique<VulkanPipelineLayout>(device->getVulkanDevice(), pipelineLayout);
	if (!desc.label.empty())
		layout->SetDebugName(desc.label.c_str());
}

/////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<VulkanShader> CreateShaderModule(VulkanDevice* device, const char* debugName, const uint32_t* code, size_t size)
{
	// To do:
	// If we have VK_KHR_maintenance5 enabled or Vulkan 1.4 we can chain
	// VkShaderModuleCreateInfo to VkPipelineShaderStageCreateInfo instead of
	// creating this pointless object.

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size * sizeof(uint32_t);
	createInfo.pCode = code;

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device->device, &createInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
		VulkanError("Could not create vulkan shader module");

	auto obj = std::make_unique<VulkanShader>(device, shaderModule);
	if (debugName)
		obj->SetDebugName(debugName);
	return obj;
}

/////////////////////////////////////////////////////////////////////////////

GPUComputePipeline::GPUComputePipeline(GPUDevice* device, const GPUComputePipelineDesc& desc) : device(device), desc(desc)
{
	VulkanPipelineCache* cache = nullptr;

	auto shader = CreateShaderModule(
		device->getVulkanDevice(),
		!desc.label.empty() ? desc.label.c_str() : nullptr,
		desc.computeShader.data(),
		desc.computeShader.size());

	VkPipelineShaderStageCreateInfo stageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.module = shader->module;
	stageInfo.pName = "main";

	VkComputePipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
	pipelineInfo.stage = stageInfo;
	pipelineInfo.layout = desc.layout->layout->layout;

	VkPipeline vkpipeline;
	vkCreateComputePipelines(device->getVulkanDevice()->device, cache ? cache->cache : VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkpipeline);
	pipeline = std::make_unique<VulkanPipeline>(device->getVulkanDevice(), vkpipeline);
	if (!desc.label.empty())
		pipeline->SetDebugName(desc.label.c_str());
}

std::shared_ptr<GPUBindGroupLayout> GPUComputePipeline::getBindLayout(int index)
{
	return desc.layout->desc.bindGroupLayouts[index];
}

/////////////////////////////////////////////////////////////////////////////

GPURenderPipeline::GPURenderPipeline(GPUDevice* device, const GPURenderPipelineDesc& desc) : device(device), desc(desc)
{
	VulkanPipelineCache* cache = nullptr;

	VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

	VkPipelineLibraryCreateInfoKHR libraryCreate = { VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR };
	VkGraphicsPipelineLibraryCreateInfoEXT pipelineLibrary = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT };

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	std::vector<VkVertexInputBindingDescription> vertexInputBindings;
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
	std::vector<VkDynamicState> dynamicStates;
	std::vector<VkPipeline> libraries;
	std::vector<std::vector<uint32_t>> shaderCode;

	struct ShaderSpecialization
	{
		VkSpecializationInfo info = {};
		std::vector<VkSpecializationMapEntry> entries;
		std::vector<uint8_t> data;
	};

	std::vector<std::unique_ptr<ShaderSpecialization>> specializations;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.layout = desc.layout->layout->layout;
	//pipelineInfo.renderPass = renderPass->renderPass;
	//pipelineInfo.subpass = subpass;

	//pipelineInfo.flags = flags;
	//pipelineLibrary.flags = flags;

	// AddLibrary(VulkanPipeline* pipeline)
	{
		//libraries.push_back(pipeline->pipeline);
	}

	// Always use dynamic state for viewport and scissor as baking this into the pipeline is horrible to work with
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

	depthStencil.depthCompareOp = (VkCompareOp)desc.depthStencil.depthCompareOp;
	depthStencil.depthBoundsTestEnable = (VkBool32)desc.depthStencil.depthBoundsTestEnable;
	depthStencil.depthTestEnable = (VkBool32)desc.depthStencil.depthTestEnable;
	depthStencil.depthWriteEnable = (VkBool32)desc.depthStencil.depthWriteEnable;
	depthStencil.stencilTestEnable = (VkBool32)desc.depthStencil.stencilTestEnable;
	depthStencil.minDepthBounds = desc.depthStencil.minDepthBounds;
	depthStencil.maxDepthBounds = desc.depthStencil.maxDepthBounds;
	depthStencil.stencilTestEnable = (VkBool32)desc.depthStencil.stencilTestEnable;
	depthStencil.front.failOp = (VkStencilOp)desc.depthStencil.front.failOp;
	depthStencil.front.passOp = (VkStencilOp)desc.depthStencil.front.passOp;
	depthStencil.front.depthFailOp = (VkStencilOp)desc.depthStencil.front.depthFailOp;
	depthStencil.front.compareOp = (VkCompareOp)desc.depthStencil.front.compareOp;
	depthStencil.front.compareMask = desc.depthStencil.front.compareMask;
	depthStencil.front.writeMask = desc.depthStencil.front.writeMask;
	depthStencil.front.reference = desc.depthStencil.front.reference;
	depthStencil.back.failOp = (VkStencilOp)desc.depthStencil.back.failOp;
	depthStencil.back.passOp = (VkStencilOp)desc.depthStencil.back.passOp;
	depthStencil.back.depthFailOp = (VkStencilOp)desc.depthStencil.back.depthFailOp;
	depthStencil.back.compareOp = (VkCompareOp)desc.depthStencil.back.compareOp;
	depthStencil.back.compareMask = desc.depthStencil.back.compareMask;
	depthStencil.back.writeMask = desc.depthStencil.back.writeMask;
	depthStencil.back.reference = desc.depthStencil.back.reference;

	inputAssembly.topology = (VkPrimitiveTopology)desc.primitive.topology;
	inputAssembly.primitiveRestartEnable = (VkBool32)desc.primitive.primitiveRestartEnable;

	rasterizer.depthClampEnable = (VkBool32)desc.primitive.depthClampEnable;
	rasterizer.rasterizerDiscardEnable = (VkBool32)desc.primitive.rasterizerDiscardEnable;
	rasterizer.polygonMode = (VkPolygonMode)desc.primitive.polygonMode;
	rasterizer.lineWidth = desc.primitive.lineWidth;
	rasterizer.cullMode = (VkCullModeFlags)desc.primitive.cullMode;
	rasterizer.frontFace = (VkFrontFace)desc.primitive.frontFace;
	rasterizer.depthBiasEnable = (VkBool32)desc.primitive.depthBiasEnable;
	rasterizer.depthBiasConstantFactor = desc.primitive.depthBiasConstantFactor;
	rasterizer.depthBiasClamp = desc.primitive.depthBiasClamp;
	rasterizer.depthBiasSlopeFactor = desc.primitive.depthBiasSlopeFactor;

	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = (VkBool32)desc.multisample.alphaToCoverageEnable;
	multisampling.alphaToOneEnable = (VkBool32)desc.multisample.alphaToOneEnable;
	multisampling.rasterizationSamples = (VkSampleCountFlagBits)desc.multisample.rasterizationSamples;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	for (const GPUFragmentTarget& target : desc.fragment.targets)
	{
		VkColorComponentFlags mask = 0;
		if (target.colorWriteMask.red)
			mask |= VK_COLOR_COMPONENT_R_BIT;
		if (target.colorWriteMask.green)
			mask |= VK_COLOR_COMPONENT_G_BIT;
		if (target.colorWriteMask.blue)
			mask |= VK_COLOR_COMPONENT_B_BIT;
		if (target.colorWriteMask.alpha)
			mask |= VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = mask;
		colorBlendAttachment.blendEnable = (VkBool32)target.blendEnable;
		colorBlendAttachment.srcColorBlendFactor = (VkBlendFactor)target.srcColorBlendFactor;
		colorBlendAttachment.dstColorBlendFactor = (VkBlendFactor)target.dstColorBlendFactor;
		colorBlendAttachment.colorBlendOp = (VkBlendOp)target.colorBlendOp;
		colorBlendAttachment.srcAlphaBlendFactor = (VkBlendFactor)target.srcAlphaBlendFactor;
		colorBlendAttachment.dstAlphaBlendFactor = (VkBlendFactor)target.dstAlphaBlendFactor;
		colorBlendAttachment.alphaBlendOp = (VkBlendOp)target.alphaBlendOp;
		colorBlendAttachments.push_back(colorBlendAttachment);
	}

	if (!desc.vertex.shader.empty())
	{
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.pName = "main";
		shaderStages.push_back(vertShaderStageInfo);
		shaderCode.push_back(desc.vertex.shader);
	}

	if (!desc.fragment.shader.empty())
	{
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
		shaderCode.push_back(desc.fragment.shader);
	}

	/*
	// AddConstant(uint32_t constantID, const void* data, size_t size)
	{
		VkPipelineShaderStageCreateInfo& stage = shaderStages.back();
		if (!stage.pSpecializationInfo)
		{
			specializations.push_back(std::make_unique<ShaderSpecialization>());
			stage.pSpecializationInfo = &specializations.back()->info;
		}

		ShaderSpecialization* s = specializations.back().get();

		VkSpecializationMapEntry entry = {};
		entry.constantID = constantID;
		entry.offset = (uint32_t)s->data.size();
		entry.size = (uint32_t)size;

		s->data.insert(s->data.end(), (uint8_t*)data, (uint8_t*)data + size);
		s->entries.push_back(entry);

		s->info.mapEntryCount = (uint32_t)s->entries.size();
		s->info.dataSize = (uint32_t)s->data.size();
		s->info.pMapEntries = s->entries.data();
		s->info.pData = s->data.data();
	}
	*/

	for (const GPUVertexBufferBinding& binding : desc.vertex.buffers)
	{
		VkVertexInputBindingDescription desc = {};
		desc.binding = binding.bindIndex;
		desc.stride = binding.stride;
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindings.push_back(desc);
	}

	for (const GPUVertexAttribute& attr : desc.vertex.attributes)
	{
		VkVertexInputAttributeDescription desc = { };
		desc.location = attr.location;
		desc.binding = attr.binding;
		desc.format = (VkFormat)attr.format;
		desc.offset = attr.offset;
		vertexInputAttributes.push_back(desc);
	}

	/*
	// AddDynamicState(VkDynamicState state)
	{
		dynamicStates.push_back(state);
	}
	*/

	// Create pipeline:

	vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexInputBindings.size();
	vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributes.size();
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

	dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	if (colorBlendAttachments.empty())
		colorBlendAttachments.push_back(ColorBlendAttachmentBuilder().Create());
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.attachmentCount = (uint32_t)colorBlendAttachments.size();

	if (!libraries.empty())
	{
		auto subpass = pipelineInfo.subpass;
		auto layout = pipelineInfo.layout;
		auto renderpass = pipelineInfo.renderPass;
		auto flags = pipelineInfo.flags;
		pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.flags = flags;
		pipelineInfo.subpass = subpass;
		pipelineInfo.layout = layout;
		pipelineInfo.renderPass = renderpass;
		libraryCreate.libraryCount = (uint32_t)libraries.size();
		libraryCreate.pLibraries = libraries.data();
	}

	std::vector<std::unique_ptr<VulkanShader>> shaders;
	shaders.reserve(shaderStages.size());
	for (size_t i = 0; i < shaderStages.size(); i++)
	{
		auto shader = CreateShaderModule(device->getVulkanDevice(), !desc.label.empty() ? desc.label.c_str() : nullptr, shaderCode[i].data(), shaderCode[i].size());
		shaderStages[i].module = shader->module;
		shaders.push_back(std::move(shader));
	}
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();

	const void** ppNext = &pipelineInfo.pNext;

	if (libraryCreate.libraryCount > 0 && device->getVulkanDevice()->SupportsExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME))
	{
		*ppNext = &libraryCreate;
		ppNext = &libraryCreate.pNext;
	}

	if (pipelineLibrary.flags != 0 && device->getVulkanDevice()->SupportsExtension(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME))
	{
		*ppNext = &pipelineLibrary;
		ppNext = &pipelineLibrary.pNext;
	}

	VkPipeline vkpipeline = 0;
	VkResult result = vkCreateGraphicsPipelines(device->getVulkanDevice()->device, cache ? cache->cache : VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkpipeline);
	device->getVulkanDevice()->CheckVulkanError(result, "Could not create graphics pipeline");
	pipeline = std::make_unique<VulkanPipeline>(device->getVulkanDevice(), vkpipeline);
	if (!desc.label.empty())
		pipeline->SetDebugName(desc.label.c_str());
}

std::shared_ptr<GPUBindGroupLayout> GPURenderPipeline::getBindLayout(int index)
{
	return desc.layout->desc.bindGroupLayouts[index];
}
