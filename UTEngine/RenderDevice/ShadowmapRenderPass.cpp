
#include "Precomp.h"
#include "ShadowmapRenderPass.h"
#include "SceneLights.h"
#include "SceneBuffers.h"
#include "VulkanObjects.h"
#include "VulkanBuilders.h"
#include "Renderer.h"
#include "UObject/ULevel.h"

ShadowmapRenderPass::ShadowmapRenderPass(Renderer* renderer) : renderer(renderer)
{
	vertexShader = Renderer::CreateVertexShader(renderer->Device.get(), "shaders/Shadowmap.vert");
	fragmentShader = Renderer::CreateFragmentShader(renderer->Device.get(), "shaders/Shadowmap.frag");

	createRenderPass();
	createPipeline();
	createFramebuffer();
}

ShadowmapRenderPass::~ShadowmapRenderPass()
{
}

void ShadowmapRenderPass::begin(VulkanCommandBuffer *cmdbuffer)
{
	RenderPassBegin renderPassInfo;
	renderPassInfo.setRenderPass(renderPass.get());
	renderPassInfo.setFramebuffer(shadowFramebuffer.get());
	renderPassInfo.setRenderArea(0, 0, renderer->SceneLights->ShadowmapSize, renderer->SceneLights->ShadowmapSize);
	renderPassInfo.addClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPassInfo.addClearDepthStencil(1.0f, 0);
	cmdbuffer->beginRenderPass(renderPassInfo);
}

void ShadowmapRenderPass::end(VulkanCommandBuffer *cmdbuffer)
{
	cmdbuffer->endRenderPass();
}

void ShadowmapRenderPass::createPipeline()
{
	GraphicsPipelineBuilder builder;
	builder.addVertexShader(vertexShader.get());
	builder.setViewport(0.0f, 0.0f, (float)renderer->SceneLights->ShadowmapSize, (float)renderer->SceneLights->ShadowmapSize);
	builder.setScissor(0, 0, renderer->SceneLights->ShadowmapSize, renderer->SceneLights->ShadowmapSize);
	builder.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
	builder.setCull(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	builder.addVertexBufferBinding(0, sizeof(SceneVertex));
	builder.addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, x));
	builder.setLayout(renderer->ScenePipelineLayout.get());
	builder.setRenderPass(renderPass.get());

	builder.setDepthStencilEnable(true, true, false);
	builder.addFragmentShader(fragmentShader.get());

	builder.setSubpassColorAttachmentCount(1);
	builder.setRasterizationSamples(VK_SAMPLE_COUNT_1_BIT);

	pipeline = builder.create(renderer->Device.get());
}

void ShadowmapRenderPass::createRenderPass()
{
	int numColorAttachments = 1;

	RenderPassBuilder builder;

	for (int i = 0; i < numColorAttachments; i++)
		builder.addAttachment(VK_FORMAT_R32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	builder.addDepthStencilAttachment(VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

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

void ShadowmapRenderPass::createFramebuffer()
{
	FramebufferBuilder builder;
	builder.setRenderPass(renderPass.get());
	builder.setSize(renderer->SceneLights->ShadowmapSize, renderer->SceneLights->ShadowmapSize);
	builder.addAttachment(renderer->SceneLights->ShadowColorBufferView.get());
	builder.addAttachment(renderer->SceneLights->ShadowDepthBufferView.get());
	shadowFramebuffer = builder.create(renderer->Device.get());
}
