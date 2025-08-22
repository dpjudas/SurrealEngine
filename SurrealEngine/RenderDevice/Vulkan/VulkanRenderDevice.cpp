
#include "Precomp.h"
#include "VulkanRenderDevice.h"
#include "CachedTexture.h"
#include "Utils/Logger.h"
#include <zvulkan/vulkanbuilders.h>
#include <zvulkan/vulkanswapchain.h>
#include <zvulkan/vulkansurface.h>
#include <zwidget/core/widget.h>
#include <cmath>
#include <stdexcept>

VulkanRenderDevice::VulkanRenderDevice(Widget* InViewport, std::shared_ptr<VulkanSurface> surface)
{
	Viewport = InViewport;

	try
	{
		auto deviceBuilder = VulkanDeviceBuilder();
		deviceBuilder.OptionalDescriptorIndexing();
		deviceBuilder.OptionalRayQuery();
		deviceBuilder.Surface(surface);
		deviceBuilder.RequireExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		deviceBuilder.RequireExtension(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
		deviceBuilder.SelectDevice(VkDeviceIndex);
		Device = deviceBuilder.Create(surface->Instance);

		bool supportsBindless =
			Device->EnabledFeatures.DescriptorIndexing.descriptorBindingPartiallyBound &&
			Device->EnabledFeatures.DescriptorIndexing.runtimeDescriptorArray &&
			Device->EnabledFeatures.DescriptorIndexing.shaderSampledImageArrayNonUniformIndexing;

		if (!supportsBindless)
			throw std::runtime_error("VulkanDrv requires a GPU that supports bindless textures!");

		Commands.reset(new CommandBufferManager(this));
		Samplers.reset(new SamplerManager(this));
		Textures.reset(new TextureManager(this));
		Buffers.reset(new BufferManager(this));
		Shaders.reset(new ShaderManager(this));
		Uploads.reset(new UploadManager(this));
		DescriptorSets.reset(new DescriptorSetManager(this));
		RenderPasses.reset(new RenderPassManager(this));
		Framebuffers.reset(new FramebufferManager(this));

		const auto& props = Device->PhysicalDevice.Properties.Properties;

		std::string deviceType;
		switch (props.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_OTHER: deviceType = "other"; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: deviceType = "integrated gpu"; break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: deviceType = "discrete gpu"; break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: deviceType = "virtual gpu"; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: deviceType = "cpu"; break;
		default: deviceType = std::to_string((int)props.deviceType); break;
		}

		std::string apiVersion = std::to_string(VK_VERSION_MAJOR(props.apiVersion)) + "." + std::to_string(VK_VERSION_MINOR(props.apiVersion)) + "." + std::to_string(VK_VERSION_PATCH(props.apiVersion));
		std::string driverVersion = std::to_string(VK_VERSION_MAJOR(props.driverVersion)) + "." + std::to_string(VK_VERSION_MINOR(props.driverVersion)) + "." + std::to_string(VK_VERSION_PATCH(props.driverVersion));

		LogMessage(std::string("Vulkan device: ") + props.deviceName);
		LogMessage("Vulkan device type: " + deviceType);
		LogMessage("Vulkan version: " + apiVersion + " (api) " + driverVersion + " (driver)");
	}
	catch (const std::exception& e)
	{
		Exception::Throw(std::string("Could not create vulkan renderer: ") + e.what());
	}
}

VulkanRenderDevice::~VulkanRenderDevice()
{
	if (Device)
		vkDeviceWaitIdle(Device->device);

	Framebuffers.reset();
	RenderPasses.reset();
	DescriptorSets.reset();
	Uploads.reset();
	Shaders.reset();
	Buffers.reset();
	Textures.reset();
	Samplers.reset();
	Commands.reset();

	Device.reset();
}

void VulkanPrintLog(const char* typestr, const std::string& msg)
{
	LogMessage(std::string("[") + typestr + "] " + msg);
}

void VulkanError(const char* text)
{
	throw std::runtime_error(text);
}

void VulkanRenderDevice::SubmitAndWait(bool present, int presentWidth, int presentHeight, bool presentFullscreen)
{
	DescriptorSets->UpdateBindlessSet();

	Commands->SubmitCommands(present, presentWidth, presentHeight, presentFullscreen);

	Batch.SceneIndexStart = 0;
	SceneVertexPos = 0;
	SceneIndexPos = 0;
}

void VulkanRenderDevice::Flush(bool AllowPrecache)
{
	if (IsLocked)
	{
		DrawBatch(Commands->GetDrawCommands());
		Commands->GetDrawCommands()->endRenderPass();
		SubmitAndWait(false, 0, 0, false);

		ClearTextureCache();

		auto cmdbuffer = Commands->GetDrawCommands();

		VkAccessFlags srcColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		VkAccessFlags dstColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		VkAccessFlags srcDepthAccess = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		VkAccessFlags dstDepthAccess = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		PipelineBarrier()
			.AddImage(Textures->Scene->ColorBuffer.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
			.AddImage(Textures->Scene->HitBuffer.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
			.AddImage(Textures->Scene->DepthBuffer.get(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, srcDepthAccess, dstDepthAccess, VK_IMAGE_ASPECT_DEPTH_BIT)
			.Execute(cmdbuffer, srcStages, dstStages);

		RenderPassBegin()
			.RenderPass(RenderPasses->Scene.RenderPassContinue.get())
			.Framebuffer(Framebuffers->SceneFramebuffer.get())
			.RenderArea(0, 0, Textures->Scene->Width, Textures->Scene->Height)
			.AddClearColor(0.0f, 0.0f, 0.0f, 0.0f)
			.AddClearColor(0.0f, 0.0f, 0.0f, 0.0f)
			.AddClearDepthStencil(1.0f, 0)
			.Execute(cmdbuffer);

		VkBuffer vertexBuffers[] = { Buffers->SceneVertexBuffer->buffer };
		VkDeviceSize offsets[] = { 0 };
		cmdbuffer->bindVertexBuffers(0, 1, vertexBuffers, offsets);
		cmdbuffer->bindIndexBuffer(Buffers->SceneIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
	}
	else
	{
		ClearTextureCache();
	}

	if (AllowPrecache && UsePrecache)
		PrecacheOnFlip = 1;
}

void VulkanRenderDevice::Lock(vec4 InFlashScale, vec4 InFlashFog, vec4 ScreenClear, uint8_t* InHitData, int* InHitSize)
{
	HitData = InHitData;
	HitSize = InHitSize;

	FlashScale = InFlashScale;
	FlashFog = InFlashFog;

	pushconstants.hitIndex = 0;
	ForceHitIndex = -1;

	// If frame textures no longer match the window or user settings, recreate them along with the swap chain
	if (!Textures->Scene || Textures->Scene->Width != Viewport->GetNativePixelWidth() || Textures->Scene->Height != Viewport->GetNativePixelHeight() ||Textures->Scene->Multisample != GetSettingsMultisample())
	{
		Framebuffers->DestroySceneFramebuffer();
		Textures->Scene.reset();
		Textures->Scene.reset(new SceneTextures(this, Viewport->GetNativePixelWidth(), Viewport->GetNativePixelHeight(), GetSettingsMultisample()));
		RenderPasses->CreateRenderPass();
		RenderPasses->CreatePipelines();
		Framebuffers->CreateSceneFramebuffer();
		DescriptorSets->UpdateFrameDescriptors();
	}

	auto cmdbuffer = Commands->GetDrawCommands();

	// Special thanks to Khronos and AMD for making this absolute hell to use.
	VkAccessFlags srcColorAccess = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	VkAccessFlags dstColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	VkAccessFlags srcDepthAccess = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkAccessFlags dstDepthAccess = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	PipelineBarrier()
		.AddImage(Textures->Scene->ColorBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
		.AddImage(Textures->Scene->HitBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
		.AddImage(Textures->Scene->DepthBuffer.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, srcDepthAccess, dstDepthAccess, VK_IMAGE_ASPECT_DEPTH_BIT)
		.Execute(cmdbuffer, srcStages, dstStages);

	RenderPassBegin()
		.RenderPass(RenderPasses->Scene.RenderPass.get())
		.Framebuffer(Framebuffers->SceneFramebuffer.get())
		.RenderArea(0, 0, Textures->Scene->Width, Textures->Scene->Height)
		.AddClearColor(ScreenClear.x, ScreenClear.y, ScreenClear.z, ScreenClear.w)
		.AddClearColor(0.0f, 0.0f, 0.0f, 0.0f)
		.AddClearDepthStencil(1.0f, 0)
		.Execute(cmdbuffer);

	VkBuffer vertexBuffers[] = { Buffers->SceneVertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	cmdbuffer->bindVertexBuffers(0, 1, vertexBuffers, offsets);
	cmdbuffer->bindIndexBuffer(Buffers->SceneIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

	IsLocked = true;
}

void VulkanRenderDevice::FlushDrawBatchAndWait()
{
	DrawBatch(Commands->GetDrawCommands());
	Commands->GetDrawCommands()->endRenderPass();
	SubmitAndWait(false, 0, 0, false);

	auto drawcommands = Commands->GetDrawCommands();

	VkAccessFlags srcColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	VkAccessFlags dstColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	VkAccessFlags srcDepthAccess = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkAccessFlags dstDepthAccess = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	PipelineBarrier()
		.AddImage(Textures->Scene->ColorBuffer.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
		.AddImage(Textures->Scene->HitBuffer.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
		.AddImage(Textures->Scene->DepthBuffer.get(), VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, srcDepthAccess, dstDepthAccess, VK_IMAGE_ASPECT_DEPTH_BIT)
		.Execute(drawcommands, srcStages, dstStages);

	RenderPassBegin()
		.RenderPass(RenderPasses->Scene.RenderPassContinue.get())
		.Framebuffer(Framebuffers->SceneFramebuffer.get())
		.RenderArea(0, 0, Textures->Scene->Width, Textures->Scene->Height)
		.Execute(drawcommands);

	VkBuffer vertexBuffers[] = { Buffers->SceneVertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	drawcommands->bindVertexBuffers(0, 1, vertexBuffers, offsets);
	drawcommands->bindIndexBuffer(Buffers->SceneIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
	drawcommands->setViewport(0, 1, &viewportdesc);
}

void VulkanRenderDevice::Unlock(bool Blit)
{
	DrawBatch(Commands->GetDrawCommands());
	Commands->GetDrawCommands()->endRenderPass();

	BlitSceneToPostprocess();
	if (Bloom)
	{
		RunBloomPass();
	}

	int windowWidth = Viewport->GetNativePixelWidth();
	int windowHeight = Viewport->GetNativePixelHeight();

	SubmitAndWait(Blit ? true : false, windowWidth, windowHeight, Viewport->IsFullscreen());

	Batch.Pipeline = nullptr;

	if (Samplers->LODBias != LODBias)
	{
		DescriptorSets->ClearCache();
		Textures->ClearAllBindlessIndexes();
		Samplers->CreateSceneSamplers();
	}

	if (HitData)
	{
		// Look for the last hit
		int width = HitWidth;
		int height = HitHeight;
		int hit = 0;
		const int32_t* data = (const int32_t*)Textures->Scene->StagingHitBuffer->Map(0, width * height * sizeof(int32_t));
		if (data)
		{
			for (int y = 0; y < height; y++)
			{
				const int* line = data + y * width;
				for (int x = 0; x < width; x++)
				{
					hit = std::max(hit, line[x]);
				}
			}
			Textures->Scene->StagingHitBuffer->Unmap();
		}
		hit--;

		hit = std::max(hit, ForceHitIndex);

		if (hit >= 0 && hit < (int)HitQueries.size())
		{
			const HitQuery& query = HitQueries[hit];
			memcpy(HitData, HitBuffer.data() + query.Start, query.Count);
			*HitSize = query.Count;
		}
		else
		{
			*HitSize = 0;
		}
	}

	HitQueryStack.clear();
	HitQueries.clear();
	HitBuffer.clear();
	HitData = nullptr;
	HitSize = nullptr;

	IsLocked = false;
}

bool VulkanRenderDevice::SupportsTextureFormat(TextureFormat Format)
{
	return Uploads->SupportsTextureFormat(Format);
}

void VulkanRenderDevice::UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL)
{
	Textures->UpdateTextureRect(&Info, U, V, UL, VL);
}

void VulkanRenderDevice::DrawBatch(VulkanCommandBuffer* cmdbuffer)
{
	size_t icount = SceneIndexPos - Batch.SceneIndexStart;
	if (icount > 0)
	{
		if (viewportdesc.minDepth != Batch.Pipeline->MinDepth || viewportdesc.maxDepth != Batch.Pipeline->MaxDepth)
		{
			viewportdesc.minDepth = Batch.Pipeline->MinDepth;
			viewportdesc.maxDepth = Batch.Pipeline->MaxDepth;
			cmdbuffer->setViewport(0, 1, &viewportdesc);
		}

		auto layout = RenderPasses->Scene.BindlessPipelineLayout.get();
		cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, Batch.Pipeline->Pipeline.get());
		cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, DescriptorSets->GetBindlessSet());
		cmdbuffer->pushConstants(layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &pushconstants);
		cmdbuffer->setBlendConstants(Batch.BlendConstants);
		cmdbuffer->drawIndexed((uint32_t)icount, 1, (uint32_t)Batch.SceneIndexStart, 0, 0);
		Batch.SceneIndexStart = SceneIndexPos;
		Stats.DrawCalls++;
	}
}

void VulkanRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	if (Facet.VertexCount < 3)
		return;

	uint32_t PolyFlags = ApplyPrecedenceRules(Surface.PolyFlags);

	CachedTexture* tex = Textures->GetTexture(Surface.Texture, !!(PolyFlags & PF_Masked));
	CachedTexture* lightmap = Textures->GetTexture(Surface.LightMap, false);
	CachedTexture* macrotex = Textures->GetTexture(Surface.MacroTexture, false);
	CachedTexture* detailtex = Textures->GetTexture(Surface.DetailTexture, false);
	CachedTexture* fogmap = (Surface.FogMap && Surface.FogMap->NumMips > 0 && !Surface.FogMap->Mips[0].Data.empty()) ? Textures->GetTexture(Surface.FogMap, false) : nullptr;

	if (Surface.DetailTexture && Surface.FogMap) detailtex = nullptr;

	float UDot = dot(Facet.MapCoords.XAxis, Facet.MapCoords.Origin);
	float VDot = dot(Facet.MapCoords.YAxis, Facet.MapCoords.Origin);

	float UPan = tex ? UDot + Surface.Texture->Pan.x : 0.0f;
	float VPan = tex ? VDot + Surface.Texture->Pan.y : 0.0f;
	float UMult = tex ? GetUMult(*Surface.Texture) : 0.0f;
	float VMult = tex ? GetVMult(*Surface.Texture) : 0.0f;
	float LMUPan = lightmap ? UDot + Surface.LightMap->Pan.x - 0.5f * Surface.LightMap->UScale : 0.0f;
	float LMVPan = lightmap ? VDot + Surface.LightMap->Pan.y - 0.5f * Surface.LightMap->VScale : 0.0f;
	float LMUMult = lightmap ? GetUMult(*Surface.LightMap) : 0.0f;
	float LMVMult = lightmap ? GetVMult(*Surface.LightMap) : 0.0f;
	float MacroUPan = macrotex ? UDot + Surface.MacroTexture->Pan.x : 0.0f;
	float MacroVPan = macrotex ? VDot + Surface.MacroTexture->Pan.y : 0.0f;
	float MacroUMult = macrotex ? GetUMult(*Surface.MacroTexture) : 0.0f;
	float MacroVMult = macrotex ? GetVMult(*Surface.MacroTexture) : 0.0f;
	float DetailUPan = UPan;
	float DetailVPan = VPan;
	float DetailUMult = detailtex ? GetUMult(*Surface.DetailTexture) : 0.0f;
	float DetailVMult = detailtex ? GetVMult(*Surface.DetailTexture) : 0.0f;

	uint32_t flags = 0;
	if (lightmap) flags |= 1;
	if (macrotex) flags |= 2;
	if (detailtex && !fogmap) flags |= 4;
	if (fogmap) flags |= 8;

	if (LightMode == 1) flags |= 64;

	if (fogmap) // if Surface.FogMap exists, use instead of detail texture
	{
		detailtex = fogmap;
		DetailUPan = UDot + Surface.FogMap->Pan.x - 0.5f * Surface.FogMap->UScale;
		DetailVPan = VDot + Surface.FogMap->Pan.y - 0.5f * Surface.FogMap->VScale;
		DetailUMult = GetUMult(*Surface.FogMap);
		DetailVMult = GetVMult(*Surface.FogMap);
	}

	SetPipeline(RenderPasses->GetPipeline(PolyFlags));

	ivec4 textureBinds = GetTextureIndexes(PolyFlags, tex, lightmap, macrotex, detailtex);
	vec4 color(1.0f);

	auto pts = Facet.Vertices;
	uint32_t vcount = Facet.VertexCount;

	uint32_t icount = (vcount - 2) * 3;
	auto alloc = ReserveVertices(vcount, icount);
	if (alloc.vptr)
	{
		SceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		for (uint32_t i = 0; i < vcount; i++)
		{
			vec3 point = pts[i];
			float u = dot(Facet.MapCoords.XAxis, point);
			float v = dot(Facet.MapCoords.YAxis, point);

			vptr->Flags = flags;
			vptr->Position.x = point.x;
			vptr->Position.y = point.y;
			vptr->Position.z = point.z;
			vptr->TexCoord.s = (u - UPan) * UMult;
			vptr->TexCoord.t = (v - VPan) * VMult;
			vptr->TexCoord2.s = (u - LMUPan) * LMUMult;
			vptr->TexCoord2.t = (v - LMVPan) * LMVMult;
			vptr->TexCoord3.s = (u - MacroUPan) * MacroUMult;
			vptr->TexCoord3.t = (v - MacroVPan) * MacroVMult;
			vptr->TexCoord4.s = (u - DetailUPan) * DetailUMult;
			vptr->TexCoord4.t = (v - DetailVPan) * DetailVMult;
			vptr->Color = color;
			vptr->TextureBinds = textureBinds;
			vptr++;
		}

		for (uint32_t i = vpos + 2; i < vpos + vcount; i++)
		{
			*(iptr++) = vpos;
			*(iptr++) = i - 1;
			*(iptr++) = i;
		}

		UseVertices(vcount, icount);
	}

	Stats.ComplexSurfaces++;
}

void VulkanRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	if (NumPts < 3) return; // This can apparently happen!!

	PolyFlags = ApplyPrecedenceRules(PolyFlags);

	SetPipeline(RenderPasses->GetPipeline(PolyFlags));

	CachedTexture* tex = Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
	ivec4 textureBinds = GetTextureIndexes(PolyFlags, tex);

	float UMult = GetUMult(Info);
	float VMult = GetVMult(Info);
	int flags = (PolyFlags & (PF_RenderFog | PF_Translucent | PF_Modulated)) == PF_RenderFog ? 16 : 0;

	if ((PolyFlags & (PF_Translucent | PF_Modulated)) == 0 && LightMode == 2) flags |= 32;

	auto alloc = ReserveVertices(NumPts, (NumPts - 2) * 3);
	if (alloc.vptr)
	{
		SceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		if (PolyFlags & PF_Modulated)
		{
			SceneVertex* vertex = vptr;
			for (int i = 0; i < NumPts; i++)
			{
				const GouraudVertex* P = &Pts[i];
				vertex->Flags = flags;
				vertex->Position.x = P->Point.x;
				vertex->Position.y = P->Point.y;
				vertex->Position.z = P->Point.z;
				vertex->TexCoord.s = P->UV.x * UMult;
				vertex->TexCoord.t = P->UV.y * VMult;
				vertex->TexCoord2.s = P->Fog.x;
				vertex->TexCoord2.t = P->Fog.y;
				vertex->TexCoord3.s = P->Fog.z;
				vertex->TexCoord3.t = P->Fog.w;
				vertex->TexCoord4.s = 0.0f;
				vertex->TexCoord4.t = 0.0f;
				vertex->Color.r = 1.0f;
				vertex->Color.g = 1.0f;
				vertex->Color.b = 1.0f;
				vertex->Color.a = 1.0f;
				vertex->TextureBinds = textureBinds;
				vertex++;
			}
		}
		else
		{
			SceneVertex* vertex = vptr;
			for (int i = 0; i < NumPts; i++)
			{
				const GouraudVertex* P = &Pts[i];
				vertex->Flags = flags;
				vertex->Position.x = P->Point.x;
				vertex->Position.y = P->Point.y;
				vertex->Position.z = P->Point.z;
				vertex->TexCoord.s = P->UV.x * UMult;
				vertex->TexCoord.t = P->UV.y * VMult;
				vertex->TexCoord2.s = P->Fog.x;
				vertex->TexCoord2.t = P->Fog.y;
				vertex->TexCoord3.s = P->Fog.z;
				vertex->TexCoord3.t = P->Fog.w;
				vertex->TexCoord4.s = 0.0f;
				vertex->TexCoord4.t = 0.0f;
				vertex->Color.r = P->Light.x;
				vertex->Color.g = P->Light.y;
				vertex->Color.b = P->Light.z;
				vertex->Color.a = 1.0f;
				vertex->TextureBinds = textureBinds;
				vertex++;
			}
		}

		uint32_t vstart = vpos;
		uint32_t vcount = NumPts;
		for (uint32_t i = vstart + 2; i < vstart + vcount; i++)
		{
			*(iptr++) = vstart;
			*(iptr++) = i - 1;
			*(iptr++) = i;
		}

		UseVertices(NumPts, (NumPts - 2) * 3);
	}

	Stats.GouraudPolygons++;
}

void VulkanRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	PolyFlags = ApplyPrecedenceRules(PolyFlags);

	if (PolyFlags & PF_SubpixelFont)
	{
		DrawBatch(Commands->GetDrawCommands());
		Batch.BlendConstants[0] = Color.x;
		Batch.BlendConstants[1] = Color.y;
		Batch.BlendConstants[2] = Color.z;
		Batch.BlendConstants[3] = Color.w;
		Color = vec4(1.0f);
	}

	CachedTexture* tex = Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
	float UMult = tex ? GetUMult(Info) : 0.0f;
	float VMult = tex ? GetVMult(Info) : 0.0f;
	float u0 = U * UMult;
	float v0 = V * VMult;
	float u1 = (U + UL) * UMult;
	float v1 = (V + VL) * VMult;
	bool clamp = (u0 >= 0.0f && u1 <= 1.00001f && v0 >= 0.0f && v1 <= 1.00001f);

	SetPipeline(RenderPasses->GetPipeline(PolyFlags));
	ivec4 textureBinds = GetTextureIndexes(PolyFlags, tex, clamp);

	float r, g, b, a;
	if (PolyFlags & PF_Modulated)
	{
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
	}
	else
	{
		r = Color.x;
		g = Color.y;
		b = Color.z;
	}
	a = 1.0f;

	if (Textures->Scene->Multisample > 1)
	{
		XL = std::floor(X + XL + 0.5f);
		YL = std::floor(Y + YL + 0.5f);
		X = std::floor(X + 0.5f);
		Y = std::floor(Y + 0.5f);
		XL = XL - X;
		YL = YL - Y;
	}

	auto alloc = ReserveVertices(4, 6);
	if (alloc.vptr)
	{
		SceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		vptr[0] = { 0, vec3(RFX2 * Z * (X - Frame->FX2),      RFY2 * Z * (Y - Frame->FY2),      Z), vec2(u0, v0), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };
		vptr[1] = { 0, vec3(RFX2 * Z * (X + XL - Frame->FX2), RFY2 * Z * (Y - Frame->FY2),      Z), vec2(u1, v0), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };
		vptr[2] = { 0, vec3(RFX2 * Z * (X + XL - Frame->FX2), RFY2 * Z * (Y + YL - Frame->FY2), Z), vec2(u1, v1), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };
		vptr[3] = { 0, vec3(RFX2 * Z * (X - Frame->FX2),      RFY2 * Z * (Y + YL - Frame->FY2), Z), vec2(u0, v1), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };

		iptr[0] = vpos;
		iptr[1] = vpos + 1;
		iptr[2] = vpos + 2;
		iptr[3] = vpos;
		iptr[4] = vpos + 2;
		iptr[5] = vpos + 3;

		UseVertices(4, 6);
	}

	Stats.Tiles++;
}

vec4 VulkanRenderDevice::ApplyInverseGamma(vec4 color)
{
	if (IsOrtho)
		return color;
	float brightness = clamp(Brightness * 2.0f, 0.05f, 2.99f);
	float gammaRed = std::max(brightness + GammaOffset + GammaOffsetRed, 0.001f);
	float gammaGreen = std::max(brightness + GammaOffset + GammaOffsetGreen, 0.001f);
	float gammaBlue = std::max(brightness + GammaOffset + GammaOffsetBlue, 0.001f);
	return vec4(pow(color.r, gammaRed), pow(color.g, gammaGreen), pow(color.b, gammaBlue), color.a);
}

void VulkanRenderDevice::Draw3DLine(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2)
{
	if (IsOrtho)
	{
		P1.x = (P1.x) / Frame->Zoom + Frame->FX2;
		P1.y = (P1.y) / Frame->Zoom + Frame->FY2;
		P1.z = 1;
		P2.x = (P2.x) / Frame->Zoom + Frame->FX2;
		P2.y = (P2.y) / Frame->Zoom + Frame->FY2;
		P2.z = 1;

		if (std::abs(P2.x - P1.x) + std::abs(P2.y - P1.y) >= 0.2f)
		{
			Draw2DLine(Frame, Color, LineFlags, P1, P2);
		}
		else if (IsOrthoLowDetail)
		{
			Draw2DPoint(Frame, Color, LINE_None, P1.x - 1, P1.y - 1, P1.x + 1, P1.y + 1, P1.z);
		}
	}
	else
	{
		bool occlude = !!(LineFlags & LINE_DepthCued);
		SetPipeline(RenderPasses->GetLinePipeline(occlude));
		ivec4 textureBinds = GetTextureIndexes(PF_Highlighted, nullptr);
		vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));

		auto alloc = ReserveVertices(2, 2);
		if (alloc.vptr)
		{
			SceneVertex* vptr = alloc.vptr;
			uint32_t* iptr = alloc.iptr;
			uint32_t vpos = alloc.vpos;

			vptr[0] = { 0, vec3(P1.x, P1.y, P1.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };
			vptr[1] = { 0, vec3(P2.x, P2.y, P2.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };

			iptr[0] = vpos;
			iptr[1] = vpos + 1;

			UseVertices(2, 2);
		}
	}
}

void VulkanRenderDevice::Draw2DLine(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2)
{
	bool occlude = !!(LineFlags & LINE_DepthCued);
	SetPipeline(RenderPasses->GetLinePipeline(occlude));
	ivec4 textureBinds = GetTextureIndexes(PF_Highlighted, nullptr);
	vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));

	auto alloc = ReserveVertices(2, 2);
	if (alloc.vptr)
	{
		SceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		vptr[0] = { 0, vec3(RFX2 * P1.z * (P1.x - Frame->FX2), RFY2 * P1.z * (P1.y - Frame->FY2), P1.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };
		vptr[1] = { 0, vec3(RFX2 * P2.z * (P2.x - Frame->FX2), RFY2 * P2.z * (P2.y - Frame->FY2), P2.z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };

		iptr[0] = vpos;
		iptr[1] = vpos + 1;

		UseVertices(2, 2);
	}
}

void VulkanRenderDevice::Draw2DPoint(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, float X1, float Y1, float X2, float Y2, float Z)
{
	bool occlude = !!(LineFlags & LINE_DepthCued);
	SetPipeline(RenderPasses->GetPointPipeline(occlude));
	ivec4 textureBinds = GetTextureIndexes(PF_Highlighted, nullptr);
	vec4 color = ApplyInverseGamma(vec4(Color.x, Color.y, Color.z, 1.0f));

	auto alloc = ReserveVertices(4, 6);
	if (alloc.vptr)
	{
		SceneVertex* vptr = alloc.vptr;
		uint32_t* iptr = alloc.iptr;
		uint32_t vpos = alloc.vpos;

		vptr[0] = { 0, vec3(RFX2 * Z * (X1 - Frame->FX2 - 0.5f), RFY2 * Z * (Y1 - Frame->FY2 - 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };
		vptr[1] = { 0, vec3(RFX2 * Z * (X2 - Frame->FX2 + 0.5f), RFY2 * Z * (Y1 - Frame->FY2 - 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };
		vptr[2] = { 0, vec3(RFX2 * Z * (X2 - Frame->FX2 + 0.5f), RFY2 * Z * (Y2 - Frame->FY2 + 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };
		vptr[3] = { 0, vec3(RFX2 * Z * (X1 - Frame->FX2 - 0.5f), RFY2 * Z * (Y2 - Frame->FY2 + 0.5f), Z), vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f), color, textureBinds };

		iptr[0] = vpos;
		iptr[1] = vpos + 1;
		iptr[2] = vpos + 2;
		iptr[3] = vpos;
		iptr[4] = vpos + 2;
		iptr[5] = vpos + 3;

		UseVertices(4, 6);
	}
}

void VulkanRenderDevice::ClearZ(FSceneNode* Frame)
{
	DrawBatch(Commands->GetDrawCommands());

	VkClearAttachment attachment = {};
	VkClearRect rect = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	attachment.clearValue.depthStencil.depth = 1.0f;
	rect.layerCount = 1;
	rect.rect.extent.width = Textures->Scene->Width;
	rect.rect.extent.height = Textures->Scene->Height;
	Commands->GetDrawCommands()->clearAttachments(1, &attachment, 1, &rect);
}

void VulkanRenderDevice::PushHit(const uint8_t* Data, int Count)
{
	if (Count <= 0) return;
	HitQueryStack.insert(HitQueryStack.end(), Data, Data + Count);

	SetHitLocation();
}

void VulkanRenderDevice::PopHit(int Count, bool bForce)
{
	if (bForce) // Force hit what we are popping
		ForceHitIndex = (int)HitQueries.size() - 1;

	HitQueryStack.resize(HitQueryStack.size() - Count);

	SetHitLocation();
}

void VulkanRenderDevice::SetHitLocation()
{
	DrawBatch(Commands->GetDrawCommands());

	if (!HitQueryStack.empty())
	{
		int index = (int)HitQueries.size();

		HitQuery query;
		query.Start = (int)HitBuffer.size();
		query.Count = (int)HitQueryStack.size();
		HitQueries.push_back(query);

		HitBuffer.insert(HitBuffer.end(), HitQueryStack.begin(), HitQueryStack.end());

		pushconstants.hitIndex = index + 1;
	}
	else
	{
		pushconstants.hitIndex = 0;
	}
}

void VulkanRenderDevice::ReadPixels(FColor* Pixels)
{
	auto cmdbuffer = Commands->GetDrawCommands();

	DrawBatch(cmdbuffer);

	if (GammaCorrectScreenshots)
	{
		PresentPushConstants pushconstants = GetPresentPushConstants();

		bool ActiveHdr = false; // (Commands->SwapChain->Format().colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT) ? 1 : 0;

		// Select present shader based on what the user is actually using
		int presentShader = 0;
		if (ActiveHdr) presentShader |= 1;
		if (GammaMode == 1) presentShader |= 2;
		if (pushconstants.Brightness != 0.0f || pushconstants.Contrast != 1.0f || pushconstants.Saturation != 1.0f) presentShader |= (clamp(GrayFormula, 0, 2) + 1) << 2;

		VkViewport viewport = {};
		viewport.width = (float)Textures->Scene->Width;
		viewport.height = (float)Textures->Scene->Height;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width = Textures->Scene->Width;
		scissor.extent.height = Textures->Scene->Height;

		auto cmdbuffer = Commands->GetDrawCommands();

		VkAccessFlags srcColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		VkAccessFlags dstColorAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		PipelineBarrier()
			.AddImage(Textures->Scene->PPImage[1].get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, srcColorAccess, dstColorAccess)
			.Execute(cmdbuffer, srcStages, dstStages);

		RenderPassBegin()
			.RenderPass(RenderPasses->Postprocess.RenderPass.get())
			.Framebuffer(Framebuffers->PPImageFB[1].get())
			.RenderArea(0, 0, Textures->Scene->Width, Textures->Scene->Height)
			.AddClearColor(0.0f, 0.0f, 0.0f, 1.0f)
			.Execute(cmdbuffer);

		cmdbuffer->setViewport(0, 1, &viewport);
		cmdbuffer->setScissor(0, 1, &scissor);
		cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->Present.ScreenshotPipeline[presentShader].get());
		cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->Present.PipelineLayout.get(), 0, DescriptorSets->GetPresentSet());
		cmdbuffer->pushConstants(RenderPasses->Present.PipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PresentPushConstants), &pushconstants);
		cmdbuffer->draw(6, 1, 0, 0);

		cmdbuffer->endRenderPass();

		PipelineBarrier()
			.AddImage(Textures->Scene->PPImage[1].get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}

	// Convert from rgba16f to bgra8 using the GPU:
	auto srcimage = Textures->Scene->PPImage[GammaCorrectScreenshots ? 1 : 0].get();

	int w = Viewport->GetNativePixelWidth();
	int h = Viewport->GetNativePixelHeight();
	void* data = Pixels;

	auto dstimage = ImageBuilder()
		.Format(VK_FORMAT_B8G8R8A8_UNORM)
		.Usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.Size(w, h)
		.DebugName("ReadPixelsDstImage")
		.Create(Device.get());

	PipelineBarrier()
		.AddImage(srcimage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT)
		.AddImage(dstimage.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

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

	PipelineBarrier()
		.AddImage(srcimage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
		.AddImage(dstimage.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	// Staging buffer for download
	auto staging = BufferBuilder()
		.Size(w * h * 4)
		.Usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU)
		.DebugName("ReadPixelsStaging")
		.Create(Device.get());

	// Copy from image to buffer
	VkBufferImageCopy region = {};
	region.imageExtent.width = w;
	region.imageExtent.height = h;
	region.imageExtent.depth = 1;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cmdbuffer->copyImageToBuffer(dstimage->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging->buffer, 1, &region);

	// Submit command buffers and wait for device to finish the work
	SubmitAndWait(false, 0, 0, false);

	uint8_t* pixels = (uint8_t*)staging->Map(0, w * h * 4);
	memcpy(data, pixels, w * h * 4);
	staging->Unmap();
}

void VulkanRenderDevice::EndFlash()
{
	if (FlashScale != vec4(0.5f, 0.5f, 0.5f, 0.0f) || FlashFog != vec4(0.0f, 0.0f, 0.0f, 0.0f))
	{
		vec4 color(FlashFog.x, FlashFog.y, FlashFog.z, 1.0f - std::min(FlashScale.x * 2.0f, 1.0f));
		vec2 zero2(0.0f);
		ivec4 zero4(0);

		DrawBatch(Commands->GetDrawCommands());
		pushconstants.objectToProjection = mat4::identity();
		pushconstants.nearClip = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		SetPipeline(RenderPasses->GetEndFlashPipeline());

		auto alloc = ReserveVertices(4, 6);
		if (alloc.vptr)
		{
			SceneVertex* vptr = alloc.vptr;
			uint32_t* iptr = alloc.iptr;
			uint32_t vpos = alloc.vpos;

			vptr[0] = { 0, vec3(-1.0f, -1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };
			vptr[1] = { 0, vec3(1.0f, -1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };
			vptr[2] = { 0, vec3(1.0f,  1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };
			vptr[3] = { 0, vec3(-1.0f,  1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };

			iptr[0] = vpos;
			iptr[1] = vpos + 1;
			iptr[2] = vpos + 2;
			iptr[3] = vpos;
			iptr[4] = vpos + 2;
			iptr[5] = vpos + 3;

			UseVertices(4, 6);
		}

		DrawBatch(Commands->GetDrawCommands());
		if (CurrentFrame)
			SetSceneNode(CurrentFrame);
	}
}

void VulkanRenderDevice::SetSceneNode(FSceneNode* Frame)
{
	auto commands = Commands->GetDrawCommands();
	DrawBatch(commands);

	CurrentFrame = Frame;
	Aspect = Frame->FY / Frame->FX;
	RProjZ = (float)std::tan(radians(Frame->FovAngle) * 0.5);
	RFX2 = 2.0f * RProjZ / Frame->FX;
	RFY2 = 2.0f * RProjZ * Aspect / Frame->FY;

	viewportdesc = {};
	viewportdesc.x = (float)Frame->XB;
	viewportdesc.y = (float)Frame->YB;
	viewportdesc.width = (float)Frame->X;
	viewportdesc.height = (float)Frame->Y;
	viewportdesc.minDepth = 0.1f;
	viewportdesc.maxDepth = 1.0f;
	commands->setViewport(0, 1, &viewportdesc);

	pushconstants.objectToProjection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);

	// TBD; do this or do like UE1 does and do the transform on the CPU?
	// maybe optionally do one or the other? transform on CPU can be super slow --Xaleros
	pushconstants.objectToProjection = pushconstants.objectToProjection * Frame->WorldToView * Frame->ObjectToWorld;

	pushconstants.nearClip = vec4(Frame->NearClip.x, Frame->NearClip.y, Frame->NearClip.z, Frame->NearClip.w);
}

void VulkanRenderDevice::PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags)
{
	PolyFlags = ApplyPrecedenceRules(PolyFlags);
	Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
}

void VulkanRenderDevice::ClearTextureCache()
{
	DescriptorSets->ClearCache();
	Textures->ClearCache();
	Uploads->ClearCache();
}

void VulkanRenderDevice::BlitSceneToPostprocess()
{
	auto buffers = Textures->Scene.get();
	auto cmdbuffer = Commands->GetDrawCommands();

	PipelineBarrier barrer0;
	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrer0.AddImage(
		buffers->ColorBuffer.get(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_TRANSFER_READ_BIT);
	if (HitData)
	{
		barrer0.AddImage(
			buffers->HitBuffer.get(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT);
		barrer0.AddImage(
			buffers->PPHitBuffer.get(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT);
		srcStageMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	barrer0.AddImage(
		buffers->PPImage[0].get(),
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT);
	barrer0.Execute(
		Commands->GetDrawCommands(),
		srcStageMask,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	if (buffers->SceneSamples != VK_SAMPLE_COUNT_1_BIT)
	{
		VkImageResolve resolve = {};
		resolve.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolve.srcSubresource.layerCount = 1;
		resolve.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolve.dstSubresource.layerCount = 1;
		resolve.extent = { (uint32_t)buffers->ColorBuffer->width, (uint32_t)buffers->ColorBuffer->height, 1 };
		cmdbuffer->resolveImage(
			buffers->ColorBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			buffers->PPImage[0]->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &resolve);
		if (HitData)
		{
			cmdbuffer->resolveImage(
				buffers->HitBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				buffers->PPHitBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &resolve);
		}
	}
	else
	{
		auto colorBuffer = buffers->ColorBuffer.get();
		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { colorBuffer->width, colorBuffer->height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { colorBuffer->width, colorBuffer->height, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.layerCount = 1;
		cmdbuffer->blitImage(
			colorBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			buffers->PPImage[0]->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_NEAREST);
		if (HitData)
		{
			VkImageCopy copy = {};
			copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy.srcSubresource.layerCount = 1;
			copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy.dstSubresource.layerCount = 1;
			copy.extent = { (uint32_t)colorBuffer->width, (uint32_t)colorBuffer->height, (uint32_t)1 };
			cmdbuffer->copyImage(
				buffers->HitBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				buffers->PPHitBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &copy);
		}
	}

	PipelineBarrier barrier1;
	barrier1.AddImage(
		buffers->PPImage[0].get(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT);
	if (HitData)
	{
		barrier1.AddImage(
			buffers->PPHitBuffer.get(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT);
	}
	barrier1.Execute(
		Commands->GetDrawCommands(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		HitData ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	if (HitData)
	{
		VkBufferImageCopy copy = {};
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.layerCount = 1;
		copy.imageOffset = { (int32_t)HitX, (int32_t)HitY, (int32_t)0 };
		copy.imageExtent = { (uint32_t)HitWidth, (uint32_t)HitHeight, (uint32_t)1 };
		cmdbuffer->copyImageToBuffer(buffers->PPHitBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffers->StagingHitBuffer->buffer, 1, &copy);

		PipelineBarrier()
			.AddBuffer(buffers->StagingHitBuffer.get(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
	}
}

void VulkanRenderDevice::RunBloomPass()
{
	float blurAmount = 0.6f + BloomAmount * (1.9f / 255.0f);
	BloomPushConstants pushconstants;
	ComputeBlurSamples(7, blurAmount, pushconstants.SampleWeights);

	auto cmdbuffer = Commands->GetDrawCommands();

	PipelineBarrier()
		.AddImage(Textures->Scene->BloomBlurLevels[0].VTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	// Extract overbright pixels that we want to bloom:
	BloomStep(cmdbuffer,
		RenderPasses->Bloom.Extract.get(),
		DescriptorSets->GetBloomPPImageSet(),
		Framebuffers->BloomBlurLevels[0].VTextureFB.get(),
		Textures->Scene->BloomBlurLevels[0].Width,
		Textures->Scene->BloomBlurLevels[0].Height,
		pushconstants);

	// Blur and downscale:
	for (int i = 0; i < NumBloomLevels - 1; i++)
	{
		PipelineBarrier()
			.AddImage(Textures->Scene->BloomBlurLevels[i].HTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.AddImage(Textures->Scene->BloomBlurLevels[i].VTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		// Gaussian blur
		BloomStep(cmdbuffer,
			RenderPasses->Bloom.BlurVertical.get(),
			DescriptorSets->GetBloomVTextureSet(i),
			Framebuffers->BloomBlurLevels[i].HTextureFB.get(),
			Textures->Scene->BloomBlurLevels[i].Width,
			Textures->Scene->BloomBlurLevels[i].Height,
			pushconstants);

		PipelineBarrier()
			.AddImage(Textures->Scene->BloomBlurLevels[i].VTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.AddImage(Textures->Scene->BloomBlurLevels[i].HTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		BloomStep(cmdbuffer,
			RenderPasses->Bloom.BlurHorizontal.get(),
			DescriptorSets->GetBloomHTextureSet(i),
			Framebuffers->BloomBlurLevels[i].VTextureFB.get(),
			Textures->Scene->BloomBlurLevels[i].Width,
			Textures->Scene->BloomBlurLevels[i].Height,
			pushconstants);

		PipelineBarrier()
			.AddImage(Textures->Scene->BloomBlurLevels[i + 1].VTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.AddImage(Textures->Scene->BloomBlurLevels[i].VTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		// Linear downscale
		BloomStep(cmdbuffer,
			RenderPasses->Bloom.Scale.get(),
			DescriptorSets->GetBloomVTextureSet(i),
			Framebuffers->BloomBlurLevels[i + 1].VTextureFB.get(),
			Textures->Scene->BloomBlurLevels[i + 1].Width,
			Textures->Scene->BloomBlurLevels[i + 1].Height,
			pushconstants);
	}

	// Blur and upscale:
	for (int i = NumBloomLevels - 1; i >= 0; i--)
	{
		PipelineBarrier()
			.AddImage(Textures->Scene->BloomBlurLevels[i].HTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.AddImage(Textures->Scene->BloomBlurLevels[i].VTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		// Gaussian blur
		BloomStep(cmdbuffer,
			RenderPasses->Bloom.BlurVertical.get(),
			DescriptorSets->GetBloomVTextureSet(i),
			Framebuffers->BloomBlurLevels[i].HTextureFB.get(),
			Textures->Scene->BloomBlurLevels[i].Width,
			Textures->Scene->BloomBlurLevels[i].Height,
			pushconstants);

		PipelineBarrier()
			.AddImage(Textures->Scene->BloomBlurLevels[i].VTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			.AddImage(Textures->Scene->BloomBlurLevels[i].HTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
			.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		BloomStep(cmdbuffer,
			RenderPasses->Bloom.BlurHorizontal.get(),
			DescriptorSets->GetBloomHTextureSet(i),
			Framebuffers->BloomBlurLevels[i].VTextureFB.get(),
			Textures->Scene->BloomBlurLevels[i].Width,
			Textures->Scene->BloomBlurLevels[i].Height,
			pushconstants);

		// Linear upscale
		if (i > 0)
		{
			PipelineBarrier()
				.AddImage(Textures->Scene->BloomBlurLevels[i - 1].VTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
				.AddImage(Textures->Scene->BloomBlurLevels[i].VTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
				.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

			BloomStep(cmdbuffer,
				RenderPasses->Bloom.Scale.get(),
				DescriptorSets->GetBloomVTextureSet(i),
				Framebuffers->BloomBlurLevels[i - 1].VTextureFB.get(),
				Textures->Scene->BloomBlurLevels[i - 1].Width,
				Textures->Scene->BloomBlurLevels[i - 1].Height,
				pushconstants);
		}
	}

	PipelineBarrier()
		.AddImage(Textures->Scene->PPImage[0].get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
		.AddImage(Textures->Scene->BloomBlurLevels[0].VTexture.get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	// Add bloom back to frame post process texture:
	BloomStep(cmdbuffer,
		RenderPasses->Bloom.Combine.get(),
		DescriptorSets->GetBloomVTextureSet(0),
		Framebuffers->PPImageFB[0].get(),
		Textures->Scene->Width,
		Textures->Scene->Height,
		pushconstants);

	PipelineBarrier()
		.AddImage(Textures->Scene->PPImage[0].get(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_ACCESS_SHADER_READ_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void VulkanRenderDevice::BloomStep(VulkanCommandBuffer* cmdbuffer, VulkanPipeline* pipeline, VulkanDescriptorSet* input, VulkanFramebuffer* output, int width, int height, const BloomPushConstants& pushconstants)
{
	RenderPassBegin()
		.RenderPass(pipeline != RenderPasses->Bloom.Combine.get() ? RenderPasses->Postprocess.RenderPass.get() : RenderPasses->Postprocess.RenderPassCombine.get())
		.Framebuffer(output)
		.RenderArea(0, 0, width, height)
		.AddClearColor(0.0f, 0.0f, 0.0f, 1.0f)
		.Execute(cmdbuffer);

	VkViewport viewport = {};
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent.width = width;
	scissor.extent.height = height;

	cmdbuffer->setViewport(0, 1, &viewport);
	cmdbuffer->setScissor(0, 1, &scissor);
	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->Bloom.PipelineLayout.get(), 0, input);
	cmdbuffer->pushConstants(RenderPasses->Bloom.PipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(BloomPushConstants), &pushconstants);
	cmdbuffer->draw(6, 1, 0, 0);

	cmdbuffer->endRenderPass();
}

float VulkanRenderDevice::ComputeBlurGaussian(float n, float theta) // theta = Blur Amount
{
	return (float)((1.0f / sqrt(2 * 3.14159265359f * theta)) * expf(-(n * n) / (2.0f * theta * theta)));
}

void VulkanRenderDevice::ComputeBlurSamples(int sampleCount, float blurAmount, float* sampleWeights)
{
	sampleWeights[0] = ComputeBlurGaussian(0, blurAmount);

	float totalWeights = sampleWeights[0];

	for (int i = 0; i < sampleCount / 2; i++)
	{
		float weight = ComputeBlurGaussian(i + 1.0f, blurAmount);

		sampleWeights[i * 2 + 1] = weight;
		sampleWeights[i * 2 + 2] = weight;

		totalWeights += weight * 2;
	}

	for (int i = 0; i < sampleCount; i++)
	{
		sampleWeights[i] /= totalWeights;
	}
}

PresentPushConstants VulkanRenderDevice::GetPresentPushConstants()
{
	PresentPushConstants pushconstants;
	pushconstants.HdrScale = 0.8f + HdrScale * (3.0f / 255.0f);
	if (IsOrtho)
	{
		pushconstants.GammaCorrection = { 1.0f };
		pushconstants.Contrast = 1.0f;
		pushconstants.Saturation = 1.0f;
		pushconstants.Brightness = 0.0f;
	}
	else
	{
		float brightness = clamp(Brightness * 2.0f, 0.05f, 2.99f);

		if (GammaMode == 0)
		{
			float invGammaRed = 1.0f / std::max(brightness + GammaOffset + GammaOffsetRed, 0.001f);
			float invGammaGreen = 1.0f / std::max(brightness + GammaOffset + GammaOffsetGreen, 0.001f);
			float invGammaBlue = 1.0f / std::max(brightness + GammaOffset + GammaOffsetBlue, 0.001f);
			pushconstants.GammaCorrection = vec4(invGammaRed, invGammaGreen, invGammaBlue, 0.0f);
		}
		else
		{
			float invGammaRed = (GammaOffset + GammaOffsetRed + 2.0f) > 0.0f ? 1.0f / (GammaOffset + GammaOffsetRed + 1.0f) : 1.0f;
			float invGammaGreen = (GammaOffset + GammaOffsetGreen + 2.0f) > 0.0f ? 1.0f / (GammaOffset + GammaOffsetGreen + 1.0f) : 1.0f;
			float invGammaBlue = (GammaOffset + GammaOffsetBlue + 2.0f) > 0.0f ? 1.0f / (GammaOffset + GammaOffsetBlue + 1.0f) : 1.0f;
			pushconstants.GammaCorrection = vec4(invGammaRed, invGammaGreen, invGammaBlue, brightness);
		}

		// pushconstants.Contrast = clamp(Contrast, 0.1f, 3.f);
		if (Contrast >= 128)
		{
			pushconstants.Contrast = 1.0f + (Contrast - 128) / 127.0f * 3.0f;
		}
		else
		{
			pushconstants.Contrast = std::max(Contrast / 128.0f, 0.1f);
		}

		// pushconstants.Saturation = clamp(Saturation, -1.0f, 1.0f);
		pushconstants.Saturation = 1.0f - 2.0f * (255 - Saturation) / 255.0f;

		// pushconstants.Brightness = clamp(LinearBrightness, -1.8f, 1.8f);
		if (LinearBrightness >= 128)
		{
			pushconstants.Brightness = (LinearBrightness - 128) / 127.0f * 1.8f;
		}
		else
		{
			pushconstants.Brightness = (128 - LinearBrightness) / 128.0f * -1.8f;
		}
	}
	return pushconstants;
}

void VulkanRenderDevice::DrawPresentTexture(int width, int height)
{
	int vpWidth = Viewport->GetNativePixelWidth();
	int vpHeight = Viewport->GetNativePixelHeight();

	PresentPushConstants pushconstants = GetPresentPushConstants();

	bool ActiveHdr = (Commands->SwapChain->Format().colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT) ? 1 : 0;

	// Select present shader based on what the user is actually using
	int presentShader = 0;
	if (ActiveHdr) presentShader |= 1;
	if (GammaMode == 1) presentShader |= 2;
	if (pushconstants.Brightness != 0.0f || pushconstants.Contrast != 1.0f || pushconstants.Saturation != 1.0f) presentShader |= (clamp(GrayFormula, 0, 2) + 1) << 2;

	float scale = std::min(width / (float)vpWidth, height / (float)vpHeight);
	int letterboxWidth = (int)std::round(vpWidth * scale);
	int letterboxHeight = (int)std::round(vpHeight * scale);
	int letterboxX = (width - letterboxWidth) / 2;
	int letterboxY = (height - letterboxHeight) / 2;

	VkViewport viewport = {};
	viewport.x = (float)letterboxX;
	viewport.y = (float)letterboxY;
	viewport.width = (float)letterboxWidth;
	viewport.height = (float)letterboxHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset.x = letterboxX;
	scissor.offset.y = letterboxY;
	scissor.extent.width = letterboxWidth;
	scissor.extent.height = letterboxHeight;

	auto cmdbuffer = Commands->GetDrawCommands();

	PipelineBarrier()
		.AddImage(Commands->SwapChain->GetImage(Commands->PresentImageIndex), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	RenderPassBegin()
		.RenderPass(RenderPasses->Present.RenderPass.get())
		.Framebuffer(Framebuffers->GetSwapChainFramebuffer())
		.RenderArea(0, 0, Commands->SwapChain->Width(), Commands->SwapChain->Height())
		.AddClearColor(0.0f, 0.0f, 0.0f, 1.0f)
		.Execute(cmdbuffer);
	cmdbuffer->setViewport(0, 1, &viewport);
	cmdbuffer->setScissor(0, 1, &scissor);
	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->Present.Pipeline[presentShader].get());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->Present.PipelineLayout.get(), 0, DescriptorSets->GetPresentSet());
	cmdbuffer->pushConstants(RenderPasses->Present.PipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PresentPushConstants), &pushconstants);
	cmdbuffer->draw(6, 1, 0, 0);
	cmdbuffer->endRenderPass();

	PipelineBarrier()
		.AddImage(Commands->SwapChain->GetImage(Commands->PresentImageIndex), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}
