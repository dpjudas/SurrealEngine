
#include "Precomp.h"
#include "RenderPassManager.h"
#include "VulkanRenderDevice.h"
#include "VulkanBuilders.h"
#include "VulkanSwapChain.h"
#include "UObject/ULevel.h"

RenderPassManager::RenderPassManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	CreateScenePipelineLayout();
	CreateSceneBindlessPipelineLayout();
	CreatePresentPipelineLayout();
}

RenderPassManager::~RenderPassManager()
{
}

void RenderPassManager::CreateScenePipelineLayout()
{
	ScenePipelineLayout = PipelineLayoutBuilder()
		.AddSetLayout(renderer->DescriptorSets->SceneDescriptorSetLayout.get())
		.AddPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants))
		.DebugName("ScenePipelineLayout")
		.Create(renderer->Device);
}

void RenderPassManager::CreateSceneBindlessPipelineLayout()
{
	if (!renderer->SupportsBindless)
		return;

	SceneBindlessPipelineLayout = PipelineLayoutBuilder()
		.AddSetLayout(renderer->DescriptorSets->SceneBindlessDescriptorSetLayout.get())
		.AddPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants))
		.DebugName("SceneBindlessPipelineLayout")
		.Create(renderer->Device);
}

void RenderPassManager::CreatePresentPipelineLayout()
{
	PresentPipelineLayout = PipelineLayoutBuilder()
		.AddSetLayout(renderer->DescriptorSets->PresentDescriptorSetLayout.get())
		.AddPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PresentPushConstants))
		.DebugName("PresentPipelineLayout")
		.Create(renderer->Device);
}

void RenderPassManager::BeginScene(VulkanCommandBuffer* cmdbuffer)
{
	RenderPassBegin renderPassInfo;
	renderPassInfo.setRenderPass(SceneRenderPass.get());
	renderPassInfo.setFramebuffer(renderer->Framebuffers->sceneFramebuffer.get());
	renderPassInfo.setRenderArea(0, 0, renderer->Textures->Scene->width, renderer->Textures->Scene->height);
	renderPassInfo.addClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPassInfo.addClearDepthStencil(1.0f, 0);
	cmdbuffer->beginRenderPass(renderPassInfo);
}

void RenderPassManager::EndScene(VulkanCommandBuffer* cmdbuffer)
{
	cmdbuffer->endRenderPass();
}

void RenderPassManager::BeginPresent(VulkanCommandBuffer* cmdbuffer)
{
	RenderPassBegin renderPassInfo;
	renderPassInfo.setRenderPass(PresentRenderPass.get());
	renderPassInfo.setFramebuffer(renderer->Framebuffers->GetSwapChainFramebuffer());
	renderPassInfo.setRenderArea(0, 0, renderer->Textures->Scene->width, renderer->Textures->Scene->height);
	renderPassInfo.addClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	cmdbuffer->beginRenderPass(renderPassInfo);
}

void RenderPassManager::EndPresent(VulkanCommandBuffer* cmdbuffer)
{
	cmdbuffer->endRenderPass();
}

VulkanPipeline* RenderPassManager::getPipeline(uint32_t PolyFlags, bool bindless)
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

	return pipeline[bindless ? 1 : 0][index].get();
}

VulkanPipeline* RenderPassManager::getEndFlashPipeline()
{
	return pipeline[0][2].get();
}

void RenderPassManager::CreatePipelines()
{
	VulkanShader* vertShader[2] = { renderer->Shaders->Scene.VertexShader.get(), renderer->Shaders->SceneBindless.VertexShader.get() };
	VulkanShader* fragShader[2] = { renderer->Shaders->Scene.FragmentShader.get(), renderer->Shaders->SceneBindless.FragmentShader.get() };
	VulkanShader* fragShaderAlphaTest[2] = { renderer->Shaders->Scene.FragmentShaderAlphaTest.get(), renderer->Shaders->SceneBindless.FragmentShaderAlphaTest.get() };
	VulkanPipelineLayout* layout[2] = { ScenePipelineLayout.get(), SceneBindlessPipelineLayout.get() };
	static const char* debugName[2] = { "ScenePipeline", "SceneBindlessPipeline" };

	for (int type = 0; type < 2; type++)
	{
		if (type == 1 && !renderer->SupportsBindless)
			break;

		for (int i = 0; i < 32; i++)
		{
			GraphicsPipelineBuilder builder;
			builder.AddVertexShader(vertShader[type]);
			builder.Viewport(0.0f, 0.0f, (float)renderer->Textures->Scene->width, (float)renderer->Textures->Scene->height);
			builder.Scissor(0, 0, renderer->Textures->Scene->width, renderer->Textures->Scene->height);
			builder.Topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			builder.Cull(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
			builder.AddVertexBufferBinding(0, sizeof(SceneVertex));
			builder.AddVertexAttribute(0, 0, VK_FORMAT_R32_UINT, offsetof(SceneVertex, Flags));
			builder.AddVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, Position));
			builder.AddVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord));
			builder.AddVertexAttribute(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord2));
			builder.AddVertexAttribute(4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord3));
			builder.AddVertexAttribute(5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord4));
			builder.AddVertexAttribute(6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SceneVertex, Color));
			if (type == 1)
				builder.AddVertexAttribute(7, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(SceneVertex, TextureBinds));
			builder.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
			builder.Layout(layout[type]);
			builder.RenderPass(SceneRenderPass.get());

			// Avoid clipping the weapon. The UE1 engine clips the geometry anyway.
			//if (renderer->Device->EnabledFeatures.Features.depthClamp)
			//	builder.DepthClampEnable(true);

			switch (i & 3)
			{
			case 0: // PF_Translucent
				builder.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
				break;
			case 1: // PF_Modulated
				builder.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_SRC_COLOR);
				break;
			case 2: // PF_Highlighted
				builder.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
				break;
			case 3:
				builder.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO); // Hmm, is it faster to keep the blend mode enabled or to toggle it?
				break;
			}

			if (i & 4) // PF_Invisible
			{
				builder.ColorWriteMask(0);
			}

			if (i & 8) // PF_Occlude
			{
				builder.DepthStencilEnable(true, true, false);
			}
			else
			{
				builder.DepthStencilEnable(true, false, false);
			}

			if (i & 16) // PF_Masked
				builder.AddFragmentShader(fragShaderAlphaTest[type]);
			else
				builder.AddFragmentShader(fragShader[type]);

			builder.SubpassColorAttachmentCount(1);
			builder.RasterizationSamples(renderer->Textures->Scene->SceneSamples);
			builder.DebugName(debugName[type]);

			pipeline[type][i] = builder.Create(renderer->Device);
		}
	}
}

void RenderPassManager::CreateRenderPass()
{
	SceneRenderPass = RenderPassBuilder()
		.AddAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			renderer->Textures->Scene->SceneSamples,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddDepthStencilAttachment(
			VK_FORMAT_D32_SFLOAT,
			renderer->Textures->Scene->SceneSamples,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.AddExternalSubpassDependency(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpassDepthStencilAttachmentRef(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.DebugName("SceneRenderPass")
		.Create(renderer->Device);
}

void RenderPassManager::CreatePresentRenderPass()
{
	PresentRenderPass = RenderPassBuilder()
		.AddAttachment(
			renderer->Commands->SwapChain->swapChainFormat.format,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.AddExternalSubpassDependency(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.DebugName("PresentRenderPass")
		.Create(renderer->Device);
}

void RenderPassManager::CreatePresentPipeline()
{
	PresentPipeline = GraphicsPipelineBuilder()
		.AddVertexShader(renderer->Shaders->ppVertexShader.get())
		.AddFragmentShader(renderer->Shaders->ppFragmentPresentShader.get())
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.Viewport(0.0f, 0.0f, 0.0f, 0.0f) // Note: the actual values are ignored since we use dynamic viewport+scissor states
		.Scissor(0, 0, 0, 0)
		.Topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.RasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
		.Layout(PresentPipelineLayout.get())
		.RenderPass(PresentRenderPass.get())
		.DebugName("PresentPipeline")
		.Create(renderer->Device);
}
