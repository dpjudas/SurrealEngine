
#include "Precomp.h"
#include "VulkanPostprocess.h"
#include "SceneBuffers.h"
#include "VulkanBuilders.h"
#include "VulkanSwapChain.h"
#include "VulkanObjects.h"

#ifdef _MSC_VER
#pragma warning(disable: 4267) // warning C4267: '=': conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4244) // warning C4244: '=': conversion from 'int' to 'float', possible loss of data
#endif

static float r_gamma = 1.0f;
/*
CVarFloat r_gamma("r_gamma", 1.0f, [](float v) {
	return clamp(v, 0.1f, 4.0f);
});
*/

VulkanPostprocess::VulkanPostprocess(Renderer* renderer) : renderer(renderer)
{
}

VulkanPostprocess::~VulkanPostprocess()
{
}

void VulkanPostprocess::blitSceneToPostprocess()
{
	auto buffers = renderer->SceneBuffers.get();
	auto cmdbuffer = renderer->GetDrawCommands();

	mCurrentPipelineImage = 0;

	VulkanPPImageTransition imageTransition0;
	imageTransition0.addImage(buffers->colorBuffer.get(), &buffers->colorBufferLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, false);
	imageTransition0.addImage(pipelineImage[mCurrentPipelineImage].get(), &pipelineLayout[mCurrentPipelineImage], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
	imageTransition0.execute(renderer->GetDrawCommands());

	if (buffers->sceneSamples != VK_SAMPLE_COUNT_1_BIT)
	{
		auto colorBuffer = buffers->colorBuffer.get();
		VkImageResolve resolve = {};
		resolve.srcOffset = { 0, 0, 0 };
		resolve.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolve.srcSubresource.mipLevel = 0;
		resolve.srcSubresource.baseArrayLayer = 0;
		resolve.srcSubresource.layerCount = 1;
		resolve.dstOffset = { 0, 0, 0 };
		resolve.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolve.dstSubresource.mipLevel = 0;
		resolve.dstSubresource.baseArrayLayer = 0;
		resolve.dstSubresource.layerCount = 1;
		resolve.extent = { (uint32_t)colorBuffer->width, (uint32_t)colorBuffer->height, 1 };
		cmdbuffer->resolveImage(
			colorBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			pipelineImage[mCurrentPipelineImage]->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &resolve);
	}
	else
	{
		auto colorBuffer = buffers->colorBuffer.get();
		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { colorBuffer->width, colorBuffer->height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { colorBuffer->width, colorBuffer->height, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		cmdbuffer->blitImage(
			colorBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			pipelineImage[mCurrentPipelineImage]->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_NEAREST);
	}
}

void VulkanPostprocess::imageTransitionScene(bool undefinedSrcLayout)
{
	auto buffers = renderer->SceneBuffers.get();

	VulkanPPImageTransition imageTransition;
	imageTransition.addImage(buffers->colorBuffer.get(), &buffers->colorBufferLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, undefinedSrcLayout);
	imageTransition.execute(renderer->GetDrawCommands());
}

void VulkanPostprocess::blitCurrentToImage(VulkanImage *dstimage, VkImageLayout *dstlayout, VkImageLayout finallayout)
{
	auto srcimage = pipelineImage[mCurrentPipelineImage].get();
	auto srclayout = &pipelineLayout[mCurrentPipelineImage];
	auto cmdbuffer = renderer->GetDrawCommands();

	*dstlayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // needed by VkPPImageTransition.addImage. Actual layout is undefined.

	VulkanPPImageTransition imageTransition0;
	imageTransition0.addImage(srcimage, srclayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, false);
	imageTransition0.addImage(dstimage, dstlayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, true);
	imageTransition0.execute(cmdbuffer);

	VkImageBlit blit = {};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { srcimage->width, srcimage->height, 1 };
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { dstimage->width, dstimage->height, 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;
	cmdbuffer->blitImage(
		srcimage->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstimage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit, VK_FILTER_NEAREST);

	VulkanPPImageTransition imageTransition1;
	imageTransition1.addImage(dstimage, dstlayout, finallayout, false);
	imageTransition1.execute(cmdbuffer);
}

void VulkanPostprocess::drawPresentTexture(const PPViewport &box)
{
	VulkanPPRenderState renderstate(renderer);

	PresentUniforms uniforms;
	uniforms.invGamma = 1.0f / renderer->PostprocessModel->present.gamma;

	renderstate.clear();
	renderstate.shader = &renderer->PostprocessModel->present.present;
	renderstate.uniforms.set(uniforms);
	renderstate.viewport = box;
	renderstate.setInputCurrent(0, PPFilterMode::Linear);
	renderstate.setInputTexture(1, &renderer->PostprocessModel->present.dither, PPFilterMode::Nearest, PPWrapMode::Repeat);
	renderstate.setOutputSwapChain();
	renderstate.setNoBlend();
	renderstate.draw();
}

void VulkanPostprocess::beginFrame()
{
	if (!mDescriptorPool)
	{
		DescriptorPoolBuilder builder;
		builder.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200);
		builder.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4);
		builder.setMaxSets(100);
		mDescriptorPool = builder.create(renderer->Device.get());
		mDescriptorPool->SetDebugName("VulkanPostprocess.mDescriptorPool");
	}

	int width = renderer->SceneBuffers->colorBuffer->width;
	int height = renderer->SceneBuffers->colorBuffer->height;

	if (!pipelineImage[0] || pipelineImage[0]->width != width || pipelineImage[0]->height != height)
	{
		for (int i = 0; i < 2; i++)
		{
			pipelineImage[i].reset();
			pipelineView[i].reset();

			ImageBuilder imgbuilder;
			imgbuilder.setSize(width, height);
			imgbuilder.setFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
			imgbuilder.setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			pipelineImage[i] = imgbuilder.create(renderer->Device.get());

			ImageViewBuilder viewbuilder;
			viewbuilder.setImage(pipelineImage[i].get(), VK_FORMAT_R16G16B16A16_SFLOAT);
			pipelineView[i] = viewbuilder.create(renderer->Device.get());
		}
	}
}

void VulkanPostprocess::renderBuffersReset()
{
	mRenderPassSetup.clear();
}

VulkanSampler *VulkanPostprocess::getSampler(PPFilterMode filter, PPWrapMode wrap)
{
	int index = (((int)filter) << 2) | (int)wrap;
	auto &sampler = mSamplers[index];
	if (sampler)
		return sampler.get();

	SamplerBuilder builder;
	builder.setMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
	builder.setMinFilter(filter == PPFilterMode::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
	builder.setMagFilter(filter == PPFilterMode::Nearest ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
	builder.setAddressMode(wrap == PPWrapMode::Clamp ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT);
	sampler = builder.create(renderer->Device.get());
	sampler->SetDebugName("VulkanPostprocess.mSamplers");
	return sampler.get();
}

/////////////////////////////////////////////////////////////////////////////

VulkanPPTexture::VulkanPPTexture(Renderer* renderer, PPTexture *texture)
{
	int pixelsize;
	switch (texture->format)
	{
	default:
	case PixelFormat::rgba8: format = VK_FORMAT_R8G8B8A8_UNORM; pixelsize = 4; break;
	case PixelFormat::rgba16f: format = VK_FORMAT_R16G16B16A16_SFLOAT; pixelsize = 8; break;
	case PixelFormat::r32f: format = VK_FORMAT_R32_SFLOAT; pixelsize = 4; break;
	case PixelFormat::rg16f: format = VK_FORMAT_R16G16_SFLOAT; pixelsize = 4; break;
	case PixelFormat::rgba16_snorm: format = VK_FORMAT_R16G16B16A16_SNORM; pixelsize = 8; break;
	}

	ImageBuilder imgbuilder;
	imgbuilder.setFormat(format);
	imgbuilder.setSize(texture->width, texture->height);
	if (texture->data)
		imgbuilder.setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	else
		imgbuilder.setUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	if (!imgbuilder.isFormatSupported(renderer->Device.get()))
		throw std::runtime_error("Vulkan device does not support the image format required by a postprocess texture");
	image = imgbuilder.create(renderer->Device.get());
	image->SetDebugName("VulkanPPTexture");

	ImageViewBuilder viewbuilder;
	viewbuilder.setImage(image.get(), format);
	view = viewbuilder.create(renderer->Device.get());
	view->SetDebugName("VulkanPPTextureView");

	if (texture->data)
	{
		size_t totalsize = texture->width * texture->height * pixelsize;
		BufferBuilder stagingbuilder;
		stagingbuilder.setSize(totalsize);
		stagingbuilder.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging = stagingbuilder.create(renderer->Device.get());
		staging->SetDebugName("VulkanPPTextureStaging");

		PipelineBarrier barrier0;
		barrier0.addImage(image.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
		barrier0.execute(renderer->GetTransferCommands(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		void *data = staging->Map(0, totalsize);
		memcpy(data, texture->data.get(), totalsize);
		staging->Unmap();

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.depth = 1;
		region.imageExtent.width = texture->width;
		region.imageExtent.height = texture->height;
		renderer->GetTransferCommands()->copyBufferToImage(staging->buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		PipelineBarrier barrier1;
		barrier1.addImage(image.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		barrier1.execute(renderer->GetTransferCommands(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else
	{
		PipelineBarrier barrier;
		barrier.addImage(image.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
		barrier.execute(renderer->GetTransferCommands(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
}

/////////////////////////////////////////////////////////////////////////////

VulkanPPShader::VulkanPPShader(Renderer* renderer, const PPShader *shaderdesc)
{
	std::string decl;

	if (!shaderdesc->uniforms.empty())
	{
		decl = "layout(push_constant) uniform Uniforms\n{\n";
		for (const auto &uniform : shaderdesc->uniforms)
		{
			decl += "\t";
			decl += getTypeStr(uniform.type);
			decl += " ";
			decl += uniform.name;
			decl += ";\n";
		}
		decl += "};\n";
	}

	vertexShader = Renderer::CreateVertexShader(renderer->Device.get(), "shaders/PPStep.vert");
	fragmentShader = Renderer::CreateFragmentShader(renderer->Device.get(), shaderdesc->fragmentShader, decl + shaderdesc->defines);
}

const char *VulkanPPShader::getTypeStr(UniformType type)
{
	switch (type)
	{
	default:
	case UniformType::Int: return "int";
	case UniformType::UInt: return "uint";
	case UniformType::Float: return "float";
	case UniformType::Vec2: return "vec2";
	case UniformType::Vec3: return "vec3";
	case UniformType::Vec4: return "vec4";
	case UniformType::IVec2: return "ivec2";
	case UniformType::IVec3: return "ivec3";
	case UniformType::IVec4: return "ivec4";
	case UniformType::UVec2: return "uvec2";
	case UniformType::UVec3: return "uvec3";
	case UniformType::UVec4: return "uvec4";
	case UniformType::Mat4: return "mat4";
	}
}

/////////////////////////////////////////////////////////////////////////////

VulkanPPRenderState::VulkanPPRenderState(Renderer* renderer) : renderer(renderer)
{
	model = renderer->PostprocessModel.get();
}

void VulkanPPRenderState::draw()
{
	auto pp = renderer->Postprocess.get();

	VulkanPPRenderPassKey key;
	key.blendMode = blendMode;
	key.inputTextures = textures.size();
	key.uniforms = uniforms.data.size();
	key.shader = getVulkanShader(shader);
	key.swapChain = (output.type == PPTextureType::SwapChain);
	if (output.type == PPTextureType::PPTexture)
		key.outputFormat = getVulkanTexture(output.texture)->format;
	else if (output.type == PPTextureType::SwapChain)
		key.outputFormat = renderer->SwapChain->swapChainFormat.format;
	else
		key.outputFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	key.samples = VK_SAMPLE_COUNT_1_BIT;

	auto &passSetup = pp->mRenderPassSetup[key];
	if (!passSetup)
		passSetup.reset(new VulkanPPRenderPassSetup(renderer, key));

	int framebufferWidth = 0, framebufferHeight = 0;
	VulkanDescriptorSet *input = getInput(passSetup.get(), textures);
	VulkanFramebuffer *outputFB = getOutput(passSetup.get(), output, framebufferWidth, framebufferHeight);

	renderScreenQuad(passSetup.get(), input, outputFB, framebufferWidth, framebufferHeight, viewport.x, viewport.y, viewport.width, viewport.height, uniforms.data.data(), uniforms.data.size());

	// Advance to next PP texture if our output was sent there
	if (output.type == PPTextureType::NextPipelineTexture)
	{
		pp->mCurrentPipelineImage = (pp->mCurrentPipelineImage + 1) % VulkanPostprocess::numPipelineImages;
	}
}

void VulkanPPRenderState::renderScreenQuad(VulkanPPRenderPassSetup *passSetup, VulkanDescriptorSet *descriptorSet, VulkanFramebuffer *framebuffer, int framebufferWidth, int framebufferHeight, int x, int y, int width, int height, const void *pushConstants, uint32_t pushConstantsSize)
{
	auto cmdbuffer = renderer->GetDrawCommands();

	VkViewport viewport = { };
	viewport.x = x;
	viewport.y = y;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = { };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = framebufferWidth;
	scissor.extent.height = framebufferHeight;

	RenderPassBegin beginInfo;
	beginInfo.setRenderPass(passSetup->renderPass.get());
	beginInfo.setRenderArea(0, 0, framebufferWidth, framebufferHeight);
	beginInfo.setFramebuffer(framebuffer);
	beginInfo.addClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	cmdbuffer->beginRenderPass(beginInfo);
	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, passSetup->pipeline.get());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, passSetup->pipelineLayout.get(), 0, descriptorSet);
	cmdbuffer->setViewport(0, 1, &viewport);
	cmdbuffer->setScissor(0, 1, &scissor);
	if (pushConstantsSize > 0)
		cmdbuffer->pushConstants(passSetup->pipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstantsSize, pushConstants);
	cmdbuffer->draw(6, 1, 0, 0);
	cmdbuffer->endRenderPass();
}

VulkanDescriptorSet *VulkanPPRenderState::getInput(VulkanPPRenderPassSetup *passSetup, const std::vector<PPTextureInput> &textures)
{
	auto pp = renderer->Postprocess.get();
	auto descriptors = pp->mDescriptorPool->allocate(passSetup->descriptorLayout.get());
	descriptors->SetDebugName("VulkanPostprocess.descriptors");

	WriteDescriptors write;
	VulkanPPImageTransition imageTransition;

	for (size_t index = 0; index < textures.size(); index++)
	{
		const PPTextureInput &input = textures[index];
		VulkanSampler *sampler = pp->getSampler(input.filter, input.wrap);
		TextureImage tex = getTexture(input.type, input.texture);

		write.addCombinedImageSampler(descriptors.get(), index, tex.view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		imageTransition.addImage(tex.image, tex.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
	}

	write.updateSets(renderer->Device.get());
	imageTransition.execute(renderer->GetDrawCommands());

	VulkanDescriptorSet *set = descriptors.get();
	renderer->FrameDeleteList->descriptors.push_back(std::move(descriptors));
	return set;
}

VulkanFramebuffer *VulkanPPRenderState::getOutput(VulkanPPRenderPassSetup *passSetup, const PPOutput &output, int &framebufferWidth, int &framebufferHeight)
{
	TextureImage tex = getTexture(output.type, output.texture);

	VkImageView view;
	int w, h;
	if (tex.view)
	{
		VulkanPPImageTransition imageTransition;
		imageTransition.addImage(tex.image, tex.layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, output.type == PPTextureType::NextPipelineTexture);
		imageTransition.execute(renderer->GetDrawCommands());

		view = tex.view->view;
		w = tex.image->width;
		h = tex.image->height;
	}
	else
	{
		view = renderer->SwapChain->swapChainImageViews[renderer->PresentImageIndex];
		w = renderer->SwapChain->actualExtent.width;
		h = renderer->SwapChain->actualExtent.height;
	}

	auto &framebuffer = passSetup->framebuffers[view];
	if (!framebuffer)
	{
		FramebufferBuilder builder;
		builder.setRenderPass(passSetup->renderPass.get());
		builder.setSize(w, h);
		builder.addAttachment(view);
		framebuffer = builder.create(renderer->Device.get());
		framebuffer->SetDebugName(tex.debugname);
	}

	framebufferWidth = w;
	framebufferHeight = h;
	return framebuffer.get();
}

VulkanPPRenderState::TextureImage VulkanPPRenderState::getTexture(const PPTextureType &type, PPTexture *pptexture)
{
	TextureImage tex = {};

	if (type == PPTextureType::CurrentPipelineTexture || type == PPTextureType::NextPipelineTexture)
	{
		int idx = renderer->Postprocess->mCurrentPipelineImage;
		if (type == PPTextureType::NextPipelineTexture)
			idx = (idx + 1) % VulkanPostprocess::numPipelineImages;

		tex.image = renderer->Postprocess->pipelineImage[idx].get();
		tex.view = renderer->Postprocess->pipelineView[idx].get();
		tex.layout = &renderer->Postprocess->pipelineLayout[idx];
		tex.debugname = "PipelineTexture";
	}
	else if (type == PPTextureType::PPTexture)
	{
		auto vktex = getVulkanTexture(pptexture);
		tex.image = vktex->image.get();
		tex.view = vktex->view.get();
		tex.layout = &vktex->layout;
		tex.debugname = "PPTexture";
	}
	else if (type == PPTextureType::SwapChain)
	{
		tex.image = nullptr;
		tex.view = nullptr;
		tex.layout = nullptr;
		tex.debugname = "SwapChain";
	}
	else
	{
		throw std::runtime_error("VulkanPPRenderState::GetTexture not implemented yet for this texture type");
	}

	return tex;
}

VulkanPPShader *VulkanPPRenderState::getVulkanShader(PPShader *shader)
{
	if (!shader->backend)
		shader->backend = std::make_unique<VulkanPPShader>(renderer, shader);
	return static_cast<VulkanPPShader*>(shader->backend.get());
}

VulkanPPTexture *VulkanPPRenderState::getVulkanTexture(PPTexture *texture)
{
	if (!texture->backend)
		texture->backend = std::make_unique<VulkanPPTexture>(renderer, texture);
	return static_cast<VulkanPPTexture*>(texture->backend.get());
}

/////////////////////////////////////////////////////////////////////////////

VulkanPPRenderPassSetup::VulkanPPRenderPassSetup(Renderer* renderer, const VulkanPPRenderPassKey &key) : renderer(renderer)
{
	createDescriptorLayout(key);
	createPipelineLayout(key);
	createRenderPass(key);
	createPipeline(key);
}

void VulkanPPRenderPassSetup::createDescriptorLayout(const VulkanPPRenderPassKey &key)
{
	DescriptorSetLayoutBuilder builder;
	for (int i = 0; i < key.inputTextures; i++)
		builder.addBinding(i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
	descriptorLayout = builder.create(renderer->Device.get());
	descriptorLayout->SetDebugName("VulkanPPRenderPassSetup.DescriptorLayout");
}

void VulkanPPRenderPassSetup::createPipelineLayout(const VulkanPPRenderPassKey &key)
{
	PipelineLayoutBuilder builder;
	builder.addSetLayout(descriptorLayout.get());
	if (key.uniforms > 0)
		builder.addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, key.uniforms);
	pipelineLayout = builder.create(renderer->Device.get());
	pipelineLayout->SetDebugName("VulkanPPRenderPassSetup.PipelineLayout");
}

void VulkanPPRenderPassSetup::createPipeline(const VulkanPPRenderPassKey &key)
{
	GraphicsPipelineBuilder builder;
	builder.addVertexShader(key.shader->vertexShader.get());
	builder.addFragmentShader(key.shader->fragmentShader.get());

	builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	builder.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	// Note: the actual values are ignored since we use dynamic viewport+scissor states
	builder.setViewport(0.0f, 0.0f, 320.0f, 200.0f);
	builder.setScissor(0.0f, 0.0f, 320.0f, 200.0f);
	builder.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	switch (key.blendMode)
	{
	default:
	case PPBlendMode::none:
		break;
	case PPBlendMode::additive:
		builder.setAdditiveBlendMode();
		break;
	case PPBlendMode::alpha:
		builder.setAlphaBlendMode();
		break;
	}
	builder.setRasterizationSamples(key.samples);
	builder.setLayout(pipelineLayout.get());
	builder.setRenderPass(renderPass.get());
	pipeline = builder.create(renderer->Device.get());
	pipeline->SetDebugName("VulkanPPRenderPassSetup.Pipeline");
}

void VulkanPPRenderPassSetup::createRenderPass(const VulkanPPRenderPassKey &key)
{
	RenderPassBuilder builder;
	if (key.swapChain)
		builder.addAttachment(key.outputFormat, key.samples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	else
		builder.addAttachment(key.outputFormat, key.samples, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	builder.addSubpass();
	builder.addSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	builder.addExternalSubpassDependency(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
	renderPass = builder.create(renderer->Device.get());
	renderPass->SetDebugName("VulkanPPRenderPassSetup.RenderPass");
}

/////////////////////////////////////////////////////////////////////////////

void VulkanPPImageTransition::addImage(VulkanImage *image, VkImageLayout *layout, VkImageLayout targetLayout, bool undefinedSrcLayout)
{
	if (*layout == targetLayout)
		return;

	VkAccessFlags srcAccess = 0;
	VkAccessFlags dstAccess = 0;
	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	switch (*layout)
	{
	case VK_IMAGE_LAYOUT_GENERAL:
		srcAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		srcAccess = VK_ACCESS_SHADER_READ_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		srcAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	default:
		throw std::runtime_error("Unimplemented src image layout transition");
	}

	switch (targetLayout)
	{
	case VK_IMAGE_LAYOUT_GENERAL:
		dstAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		dstStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
		dstStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		dstAccess = VK_ACCESS_SHADER_READ_BIT;
		dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		dstAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	default:
		throw std::runtime_error("Unimplemented dst image layout transition");
	}

	barrier.addImage(image, undefinedSrcLayout ? VK_IMAGE_LAYOUT_UNDEFINED : *layout, targetLayout, srcAccess, dstAccess, aspectMask);
	needbarrier = true;
	*layout = targetLayout;
}

void VulkanPPImageTransition::execute(VulkanCommandBuffer *cmdbuffer)
{
	if (needbarrier)
		barrier.execute(cmdbuffer, srcStageMask, dstStageMask);
}
