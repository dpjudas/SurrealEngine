
#include "Precomp.h"
#include "SceneRenderPass.h"
#include "SceneBuffers.h"
#include "VulkanObjects.h"
#include "VulkanBuilders.h"
#include "Renderer.h"
#include "UObject/ULevel.h"

SceneRenderPass::SceneRenderPass(Renderer* renderer) : renderer(renderer)
{
	vertexShader = Renderer::CreateVertexShader(renderer->Device.get(), "shaders/Scene.vert");
	fragmentShader = Renderer::CreateFragmentShader(renderer->Device.get(), "shaders/Scene.frag");
	fragmentShaderAlphaTest = Renderer::CreateFragmentShader(renderer->Device.get(), "shaders/Scene.frag", "#define ALPHATEST");

	createRenderPass();
	createPipeline();
	createFramebuffer();
}

SceneRenderPass::~SceneRenderPass()
{
}

void SceneRenderPass::begin(VulkanCommandBuffer *cmdbuffer)
{
	RenderPassBegin renderPassInfo;
	renderPassInfo.setRenderPass(renderPass.get());
	renderPassInfo.setFramebuffer(sceneFramebuffer.get());
	renderPassInfo.setRenderArea(0, 0, renderer->SceneBuffers_->width, renderer->SceneBuffers_->height);
	renderPassInfo.addClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPassInfo.addClearDepthStencil(1.0f, 0);
	cmdbuffer->beginRenderPass(renderPassInfo);
}

void SceneRenderPass::end(VulkanCommandBuffer *cmdbuffer)
{
	cmdbuffer->endRenderPass();
}

VulkanPipeline* SceneRenderPass::getPipeline(uint32_t PolyFlags)
{
	// Adjust PolyFlags according to Unreal's precedence rules.
	if (!(PolyFlags & (PF_Translucent | PF_Modulated)))
		PolyFlags |= PF_Occlude;
	else if (PolyFlags & PF_Translucent)
		PolyFlags &= ~PF_Masked;

	int index;
	if (PolyFlags & PF_Translucent)
	{
		index = 0;
	}
	else if (PolyFlags & PF_Modulated)
	{
		index = 1;
	}
	else if (PolyFlags & PF_Highlighted)
	{
		index = 2;
	}
	else
	{
		index = 3;
	}

	if (PolyFlags & PF_Invisible)
	{
		index |= 4;
	}
	if (PolyFlags & PF_Occlude)
	{
		index |= 8;
	}
	if (PolyFlags & PF_Masked)
	{
		index |= 16;
	}

	return pipeline[index].get();
}

VulkanPipeline* SceneRenderPass::getEndFlashPipeline()
{
	return pipeline[2].get();
}

void SceneRenderPass::createPipeline()
{
	for (int i = 0; i < 32; i++)
	{
		GraphicsPipelineBuilder builder;
		builder.addVertexShader(vertexShader.get());
		builder.setViewport(0.0f, 0.0f, (float)renderer->SceneBuffers_->width, (float)renderer->SceneBuffers_->height);
		builder.setScissor(0, 0, renderer->SceneBuffers_->width, renderer->SceneBuffers_->height);
		builder.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
		builder.setCull(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.addVertexBufferBinding(0, sizeof(SceneVertex));
		builder.addVertexAttribute(0, 0, VK_FORMAT_R32_UINT, offsetof(SceneVertex, flags));
		builder.addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, x));
		builder.addVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, u));
		builder.addVertexAttribute(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, u2));
		builder.addVertexAttribute(4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, u3));
		builder.addVertexAttribute(5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, u4));
		builder.addVertexAttribute(6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SceneVertex, r));
		builder.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		builder.setLayout(renderer->ScenePipelineLayout.get());
		builder.setRenderPass(renderPass.get());

		switch (i & 3)
		{
		case 0: // PF_Translucent
			builder.setBlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
			break;
		case 1: // PF_Modulated
			builder.setBlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_SRC_COLOR);
			break;
		case 2: // PF_Highlighted
			builder.setBlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			break;
		case 3:
			builder.setBlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO); // Hmm, is it faster to keep the blend mode enabled or to toggle it?
			break;
		}

		if (i & 4) // PF_Invisible
		{
			builder.setColorWriteMask(0);
		}

		if (i & 8) // PF_Occlude
		{
			builder.setDepthStencilEnable(true, true, false);
		}
		else
		{
			builder.setDepthStencilEnable(true, false, false);
		}

		if (i & 16) // PF_Masked
			builder.addFragmentShader(fragmentShaderAlphaTest.get());
		else
			builder.addFragmentShader(fragmentShader.get());

		builder.setSubpassColorAttachmentCount(1);
		builder.setRasterizationSamples(renderer->SceneBuffers_->sceneSamples);

		pipeline[i] = builder.create(renderer->Device.get());
	}
}

void SceneRenderPass::createRenderPass()
{
	int numColorAttachments = 1;

	RenderPassBuilder builder;

	for (int i = 0; i < numColorAttachments; i++)
		builder.addAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, renderer->SceneBuffers_->sceneSamples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	builder.addDepthStencilAttachment(VK_FORMAT_D32_SFLOAT, renderer->SceneBuffers_->sceneSamples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	builder.addExternalSubpassDependency(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	builder.addSubpass();
	for (int i = 0; i < numColorAttachments; i++)
		builder.addSubpassColorAttachmentRef(i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	builder.addSubpassDepthStencilAttachmentRef(numColorAttachments, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	renderPass = builder.create(renderer->Device.get());
}

void SceneRenderPass::createFramebuffer()
{
	FramebufferBuilder builder;
	builder.setRenderPass(renderPass.get());
	builder.setSize(renderer->SceneBuffers_->width, renderer->SceneBuffers_->height);
	builder.addAttachment(renderer->SceneBuffers_->colorBufferView.get());
	builder.addAttachment(renderer->SceneBuffers_->depthBufferView.get());
	sceneFramebuffer = builder.create(renderer->Device.get());
}
