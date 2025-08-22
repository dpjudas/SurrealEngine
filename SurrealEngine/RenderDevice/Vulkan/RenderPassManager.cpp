
#include "Precomp.h"
#include "RenderPassManager.h"
#include "VulkanRenderDevice.h"
#include <zvulkan/vulkanbuilders.h>
#include <zvulkan/vulkanswapchain.h>

RenderPassManager::RenderPassManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	CreateSceneBindlessPipelineLayout();
	CreatePostprocessRenderPass();
	CreatePresentPipelineLayout();
	CreateScreenshotPipeline();
	CreateBloomPipelineLayout();
	CreateBloomPipeline();
}

RenderPassManager::~RenderPassManager()
{
}

void RenderPassManager::CreateSceneBindlessPipelineLayout()
{
	Scene.BindlessPipelineLayout = PipelineLayoutBuilder()
		.AddSetLayout(renderer->DescriptorSets->GetTextureBindlessLayout())
		.AddPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants))
		.DebugName("SceneBindlessPipelineLayout")
		.Create(renderer->Device.get());
}

void RenderPassManager::CreatePresentPipelineLayout()
{
	Present.PipelineLayout = PipelineLayoutBuilder()
		.AddSetLayout(renderer->DescriptorSets->GetPresentLayout())
		.AddPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PresentPushConstants))
		.DebugName("PresentPipelineLayout")
		.Create(renderer->Device.get());
}

void RenderPassManager::CreateBloomPipelineLayout()
{
	Bloom.PipelineLayout = PipelineLayoutBuilder()
		.AddSetLayout(renderer->DescriptorSets->GetBloomLayout())
		.AddPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(BloomPushConstants))
		.DebugName("BloomPipelineLayout")
		.Create(renderer->Device.get());
}

PipelineState* RenderPassManager::GetPipeline(uint32_t PolyFlags)
{
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

	if (PolyFlags & PF_SubpixelFont)
	{
		index = 32;
	}

	return &Scene.Pipeline[index];
}

PipelineState* RenderPassManager::GetEndFlashPipeline()
{
	return &Scene.Pipeline[2];
}

void RenderPassManager::CreatePipelines()
{
	const std::vector<uint32_t>& vertShader = renderer->Shaders->Scene.VertexShader;
	const std::vector<uint32_t>& fragShader = renderer->Shaders->Scene.FragmentShader;
	const std::vector<uint32_t>& fragShaderAlphaTest = renderer->Shaders->Scene.FragmentShaderAlphaTest;
	VulkanPipelineLayout* layout = Scene.BindlessPipelineLayout.get();
	static const char* debugName = "ScenePipeline";

	for (int i = 0; i < 33; i++)
	{
		GraphicsPipelineBuilder builder;
		builder.AddVertexShader(vertShader);
		builder.Viewport(0.0f, 0.0f, (float)renderer->Textures->Scene->Width, (float)renderer->Textures->Scene->Height);
		builder.Scissor(0, 0, renderer->Textures->Scene->Width, renderer->Textures->Scene->Height);
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
		builder.AddVertexAttribute(7, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(SceneVertex, TextureBinds));
		builder.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		builder.Layout(layout);
		builder.RenderPass(Scene.RenderPass.get());

		// Avoid clipping the weapon. The UE1 engine clips the geometry anyway.
		if (renderer->Device.get()->EnabledFeatures.Features.depthClamp)
			builder.DepthClampEnable(true);

		if (i < 32)
		{
			ColorBlendAttachmentBuilder colorblend;
			switch (i & 3)
			{
			case 0: // PF_Translucent
				colorblend.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
				builder.DepthBias(true, -1.0f, 0.0f, -1.0f);
				break;
			case 1: // PF_Modulated
				colorblend.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_SRC_COLOR);
				builder.DepthBias(true, -1.0f, 0.0f, -1.0f);
				break;
			case 2: // PF_Highlighted
				colorblend.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
				builder.DepthBias(true, -1.0f, 0.0f, -1.0f);
				break;
			case 3:
				colorblend.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO); // Hmm, is it faster to keep the blend mode enabled or to toggle it?
				break;
			}

			if (i & 4) // PF_Invisible
			{
				colorblend.ColorWriteMask(0);
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
				builder.AddFragmentShader(fragShaderAlphaTest);
			else
				builder.AddFragmentShader(fragShader);

			builder.AddColorBlendAttachment(colorblend.Create());
			builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder().Create());
		}
		else // PF_SubpixelFont
		{
			builder.AddFragmentShader(fragShaderAlphaTest);
			builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder()
				.BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_CONSTANT_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR)
				.Create());
			builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder().Create());
		}

		builder.RasterizationSamples(renderer->Textures->Scene->SceneSamples);
		builder.DebugName(debugName);

		Scene.Pipeline[i].Pipeline = builder.Create(renderer->Device.get());
	}

	// Line pipeline
	for (int i = 0; i < 2; i++)
	{
		GraphicsPipelineBuilder builder;
		builder.AddVertexShader(vertShader);
		builder.Viewport(0.0f, 0.0f, (float)renderer->Textures->Scene->Width, (float)renderer->Textures->Scene->Height);
		builder.Scissor(0, 0, renderer->Textures->Scene->Width, renderer->Textures->Scene->Height);
		builder.Topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
		builder.Cull(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
		builder.AddVertexBufferBinding(0, sizeof(SceneVertex));
		builder.AddVertexAttribute(0, 0, VK_FORMAT_R32_UINT, offsetof(SceneVertex, Flags));
		builder.AddVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SceneVertex, Position));
		builder.AddVertexAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord));
		builder.AddVertexAttribute(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord2));
		builder.AddVertexAttribute(4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord3));
		builder.AddVertexAttribute(5, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SceneVertex, TexCoord4));
		builder.AddVertexAttribute(6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SceneVertex, Color));
		builder.AddVertexAttribute(7, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(SceneVertex, TextureBinds));
		builder.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		builder.Layout(layout);
		builder.RenderPass(Scene.RenderPass.get());

		builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder().BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA).Create());
		builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder().Create());

		builder.DepthStencilEnable(true, true, false);
		builder.AddFragmentShader(fragShader);

		builder.RasterizationSamples(renderer->Textures->Scene->SceneSamples);
		builder.DebugName(debugName);

		Scene.LinePipeline[i].Pipeline = builder.Create(renderer->Device.get());

		if (i == 0)
		{
			Scene.LinePipeline[i].MinDepth = 0.0f;
			Scene.LinePipeline[i].MaxDepth = 0.1f;
		}
	}

	// Point pipeline
	for (int i = 0; i < 2; i++)
	{
		GraphicsPipelineBuilder builder;
		builder.AddVertexShader(vertShader);
		builder.AddFragmentShader(fragShader);
		builder.Viewport(0.0f, 0.0f, (float)renderer->Textures->Scene->Width, (float)renderer->Textures->Scene->Height);
		builder.Scissor(0, 0, renderer->Textures->Scene->Width, renderer->Textures->Scene->Height);
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
		builder.AddVertexAttribute(7, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(SceneVertex, TextureBinds));
		builder.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		builder.Layout(layout);
		builder.RenderPass(Scene.RenderPass.get());

		builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder().BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA).Create());
		builder.AddColorBlendAttachment(ColorBlendAttachmentBuilder().Create());

		builder.DepthStencilEnable(true, true, false);
		builder.RasterizationSamples(renderer->Textures->Scene->SceneSamples);
		builder.DebugName(debugName);

		Scene.PointPipeline[i].Pipeline = builder.Create(renderer->Device.get());

		if (i == 0)
		{
			Scene.PointPipeline[i].MinDepth = 0.0f;
			Scene.PointPipeline[i].MaxDepth = 0.1f;
		}
	}
}

void RenderPassManager::CreateRenderPass()
{
	Scene.RenderPass = RenderPassBuilder()
		.AddAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			renderer->Textures->Scene->SceneSamples,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddAttachment(
			VK_FORMAT_R32_UINT,
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
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpassColorAttachmentRef(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpassDepthStencilAttachmentRef(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.DebugName("SceneRenderPass")
		.Create(renderer->Device.get());

	Scene.RenderPassContinue = RenderPassBuilder()
		.AddAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			renderer->Textures->Scene->SceneSamples,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddAttachment(
			VK_FORMAT_R32_UINT,
			renderer->Textures->Scene->SceneSamples,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddDepthStencilAttachment(
			VK_FORMAT_D32_SFLOAT,
			renderer->Textures->Scene->SceneSamples,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpassColorAttachmentRef(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpassDepthStencilAttachmentRef(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.DebugName("SceneRenderPassContinue")
		.Create(renderer->Device.get());
}

void RenderPassManager::CreatePresentRenderPass()
{
	Present.RenderPass = RenderPassBuilder()
		.AddAttachment(
			renderer->Commands->SwapChain->Format().format,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.DebugName("PresentRenderPass")
		.Create(renderer->Device.get());
}

void RenderPassManager::CreatePresentPipeline()
{
	for (int i = 0; i < 16; i++)
	{
		Present.Pipeline[i] = GraphicsPipelineBuilder()
			.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
			.AddFragmentShader(renderer->Shaders->Postprocess.FragmentPresentShader[i])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.Layout(Present.PipelineLayout.get())
			.RenderPass(Present.RenderPass.get())
			.DebugName("PresentPipeline")
			.Create(renderer->Device.get());
	}
}

void RenderPassManager::CreateScreenshotPipeline()
{
	for (int i = 0; i < 16; i++)
	{
		Present.ScreenshotPipeline[i] = GraphicsPipelineBuilder()
			.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
			.AddFragmentShader(renderer->Shaders->Postprocess.FragmentPresentShader[i])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.Layout(Present.PipelineLayout.get())
			.RenderPass(Postprocess.RenderPass.get())
			.DebugName("ScreenshotPipeline")
			.Create(renderer->Device.get());
	}
}

void RenderPassManager::CreatePostprocessRenderPass()
{
	Postprocess.RenderPass = RenderPassBuilder()
		.AddAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.DebugName("PPRenderPass")
		.Create(renderer->Device.get());

	Postprocess.RenderPassCombine = RenderPassBuilder()
		.AddAttachment(
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.AddSubpass()
		.AddSubpassColorAttachmentRef(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.DebugName("PPRenderPassCombine")
		.Create(renderer->Device.get());
}

void RenderPassManager::CreateBloomPipeline()
{
	Bloom.Extract = GraphicsPipelineBuilder()
		.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
		.AddFragmentShader(renderer->Shaders->Bloom.Extract)
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.Layout(Bloom.PipelineLayout.get())
		.RenderPass(Postprocess.RenderPass.get())
		.DebugName("Bloom.Extract")
		.Create(renderer->Device.get());

	Bloom.Combine = GraphicsPipelineBuilder()
		.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
		.AddFragmentShader(renderer->Shaders->Bloom.Combine)
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.AddColorBlendAttachment(ColorBlendAttachmentBuilder().BlendMode(VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE).Create())
		.Layout(Bloom.PipelineLayout.get())
		.RenderPass(Postprocess.RenderPass.get())
		.DebugName("Bloom.Combine")
		.Create(renderer->Device.get());

	Bloom.Scale = GraphicsPipelineBuilder()
		.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
		.AddFragmentShader(renderer->Shaders->Bloom.Combine)
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.Layout(Bloom.PipelineLayout.get())
		.RenderPass(Postprocess.RenderPass.get())
		.DebugName("Bloom.Copy")
		.Create(renderer->Device.get());

	Bloom.BlurVertical = GraphicsPipelineBuilder()
		.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
		.AddFragmentShader(renderer->Shaders->Bloom.BlurVertical)
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.Layout(Bloom.PipelineLayout.get())
		.RenderPass(Postprocess.RenderPass.get())
		.DebugName("Bloom.BlurVertical")
		.Create(renderer->Device.get());

	Bloom.BlurHorizontal = GraphicsPipelineBuilder()
		.AddVertexShader(renderer->Shaders->Postprocess.VertexShader)
		.AddFragmentShader(renderer->Shaders->Bloom.BlurHorizontal)
		.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
		.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
		.Layout(Bloom.PipelineLayout.get())
		.RenderPass(Postprocess.RenderPass.get())
		.DebugName("Bloom.BlurHorizontal")
		.Create(renderer->Device.get());
}
