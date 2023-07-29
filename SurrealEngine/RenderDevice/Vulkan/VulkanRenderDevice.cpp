
#include "Precomp.h"
#include "VulkanRenderDevice.h"
#include "CachedTexture.h"
#include <zvulkan/vulkanbuilders.h>
#include <zvulkan/vulkansurface.h>
#include <zvulkan/vulkanswapchain.h>
#include <zvulkan/vulkancompatibledevice.h>
#include "UTF16.h"
#include "UObject/ULevel.h"
#include "UObject/UClient.h"
#include "UObject/UTexture.h"
#include "Window/Window.h"
#include <set>

void VulkanPrintLog(const char* typestr, const std::string& msg)
{
	//debugf("[%s] %s", typestr, msg.c_str());
}

void VulkanError(const char* text)
{
	throw std::runtime_error(text);
}

VulkanRenderDevice::VulkanRenderDevice(DisplayWindow* InViewport, std::shared_ptr<VulkanSurface> surface)
{
	Viewport = InViewport;

	try
	{
		Device = VulkanDeviceBuilder()
			.OptionalDescriptorIndexing()
			.OptionalRayQuery()
			.Surface(surface)
			.SelectDevice(0)
			.Create(surface->Instance);

		SupportsBindless =
			Device->EnabledFeatures.DescriptorIndexing.descriptorBindingPartiallyBound &&
			Device->EnabledFeatures.DescriptorIndexing.runtimeDescriptorArray &&
			Device->EnabledFeatures.DescriptorIndexing.shaderSampledImageArrayNonUniformIndexing;

		Commands.reset(new CommandBufferManager(this));
		Samplers.reset(new SamplerManager(this));
		Textures.reset(new TextureManager(this));
		Buffers.reset(new BufferManager(this));
		Shaders.reset(new ShaderManager(this));
		Uploads.reset(new UploadManager(this));
		DescriptorSets.reset(new DescriptorSetManager(this));
		RenderPasses.reset(new RenderPassManager(this));
		Framebuffers.reset(new FramebufferManager(this));

		UsesBindless = SupportsBindless;

		/*if (VkDebug)
		{
			const auto& props = Device->PhysicalDevice.Properties;

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

			std::string apiVersion, driverVersion;
			apiVersion = FString::Printf("%d.%d.%d", VK_VERSION_MAJOR(props.apiVersion), VK_VERSION_MINOR(props.apiVersion), VK_VERSION_PATCH(props.apiVersion));
			driverVersion = FString::Printf("%d.%d.%d", VK_VERSION_MAJOR(props.driverVersion), VK_VERSION_MINOR(props.driverVersion), VK_VERSION_PATCH(props.driverVersion));

			debugf("Vulkan device: %s", to_utf16(props.deviceName).c_str());
			debugf("Vulkan device type: %s", *deviceType);
			debugf("Vulkan version: %s (api) %s (driver)", *apiVersion, *driverVersion);
		}*/
	}
	catch (...)
	{
		Dispose();
		throw;
	}
}

VulkanRenderDevice::~VulkanRenderDevice()
{
	Dispose();
}

void VulkanRenderDevice::Dispose()
{
	if (Device) vkDeviceWaitIdle(Device->device);

	Framebuffers.reset();
	RenderPasses.reset();
	DescriptorSets.reset();
	Uploads.reset();
	Shaders.reset();
	Buffers.reset();
	Textures.reset();
	Samplers.reset();
	Commands.reset();
}

void VulkanRenderDevice::SubmitAndWait(bool present, int presentWidth, int presentHeight)
{
	if (UsesBindless)
		DescriptorSets->UpdateBindlessDescriptorSet();

	Commands->SubmitCommands(present, presentWidth, presentHeight);

	Batch.Pipeline = nullptr;
	Batch.DescriptorSet = nullptr;
	Batch.SceneIndexStart = 0;
	SceneVertexPos = 0;
	SceneIndexPos = 0;
}

void VulkanRenderDevice::Flush(bool AllowPrecache)
{
	if (IsLocked)
	{
		DrawBatch(Commands->GetDrawCommands());
		RenderPasses->EndScene(Commands->GetDrawCommands());
		SubmitAndWait(false, 0, 0);

		ClearTextureCache();

		auto cmdbuffer = Commands->GetDrawCommands();
		RenderPasses->BeginScene(cmdbuffer, vec4(0.0f, 0.0f, 0.0f, 1.0f));

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

bool VulkanRenderDevice::Exec(std::string Cmd, OutputDevice& Ar)
{
	if (RenderDevice::Exec(Cmd, Ar))
	{
		return true;
	}
	else if (ParseCommand(&Cmd, "DGL"))
	{
		if (ParseCommand(&Cmd, "BUFFERTRIS"))
		{
			return true;
		}
		else if (ParseCommand(&Cmd, "BUILD"))
		{
			return true;
		}
		else if (ParseCommand(&Cmd, "AA"))
		{
			return true;
		}
		return true;
	}
	else if (ParseCommand(&Cmd, "GetRes"))
	{
		struct Resolution
		{
			int X;
			int Y;

			// For sorting highest resolution first
			bool operator<(const Resolution& other) const { if (X != other.X) return X > other.X; else return Y > other.Y; }
		};

		std::set<Resolution> resolutions;

#ifdef WIN32
		// Always include what the monitor is currently using
		HDC screenDC = GetDC(0);
		int screenWidth = GetDeviceCaps(screenDC, HORZRES);
		int screenHeight = GetDeviceCaps(screenDC, VERTRES);
		resolutions.insert({ screenWidth, screenHeight });
		ReleaseDC(0, screenDC);

		// Get what else is available according to Windows
		DEVMODE devmode = {};
		devmode.dmSize = sizeof(DEVMODE);
		int i = 0;
		while (EnumDisplaySettingsEx(nullptr, i++, &devmode, 0) != 0)
		{
			if ((devmode.dmFields & (DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT)) == (DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT) && devmode.dmBitsPerPel >= 24)
			{
				resolutions.insert({ (int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight });
			}

			devmode = {};
			devmode.dmSize = sizeof(DEVMODE);
		}

		// Add a letterboxed 4:3 mode for widescreen monitors
		resolutions.insert({ (screenHeight * 4 + 2) / 3, screenHeight });

		// Include a few classics from the era
		resolutions.insert({ 640, 480 });
		resolutions.insert({ 800, 600 });
		resolutions.insert({ 1024, 768 });
		resolutions.insert({ 1600, 1200 });
#else
		resolutions.insert({ 640, 480 });
		resolutions.insert({ 800, 600 });
		resolutions.insert({ 1280, 720 });
		resolutions.insert({ 1024, 768 });
		resolutions.insert({ 1280, 768 });
		resolutions.insert({ 1152, 864 });
		resolutions.insert({ 1280, 900 });
		resolutions.insert({ 1280, 1024 });
		resolutions.insert({ 1400, 1024 });
		resolutions.insert({ 1920, 1080 });
		resolutions.insert({ 2560, 1440 });
		resolutions.insert({ 2560, 1600 });
		resolutions.insert({ 3840, 2160 });
#endif

		std::string Str;
		for (const Resolution& resolution : resolutions)
		{
			Str += std::to_string(resolution.X) + "x" + std::to_string(resolution.Y);
		}
		Ar.Log(Str);
		return true;
	}
	else if (ParseCommand(&Cmd, "VSTAT"))
	{
		if (ParseCommand(&Cmd, "Memory"))
		{
			StatMemory = !StatMemory;
			return true;
		}
		else if (ParseCommand(&Cmd, "Resources"))
		{
			StatResources = !StatResources;
			return true;
		}
		else if (ParseCommand(&Cmd, "Draw"))
		{
			StatDraw = !StatDraw;
			return true;
		}
		return false;
	}
	else if (ParseCommand(&Cmd, "GetVkDevices"))
	{
		std::vector<VulkanCompatibleDevice> supportedDevices = VulkanDeviceBuilder()
			.OptionalDescriptorIndexing()
			.OptionalRayQuery()
			.Surface(Device->Surface)
			.FindDevices(Device->Instance);
		for (size_t i = 0; i < supportedDevices.size(); i++)
		{
			Ar.Log("#" + std::to_string(i) + " - " + supportedDevices[i].Device->Properties.deviceName + "\r\n");
		}
		return true;
	}
	else if (ParseCommand(&Cmd, "VkMemStats"))
	{
		VmaStats stats = {};
		vmaCalculateStats(Device->allocator, &stats);
		Ar.Log("Allocated objects: " + std::to_string(stats.total.allocationCount) + ", used bytes: " + std::to_string(stats.total.usedBytes / (1024 * 1024)) + " MB\r\n");
		Ar.Log("Unused range count: " + std::to_string(stats.total.unusedRangeCount) + ", unused bytes: " + std::to_string(stats.total.unusedBytes / (1024 * 1024)) + " MB\r\n");
		return true;
	}
	else
	{
		return false;
	}
}

void VulkanRenderDevice::Lock(vec4 InFlashScale, vec4 InFlashFog, vec4 ScreenClear)
{
	FlashScale = InFlashScale;
	FlashFog = InFlashFog;

	if (!Textures->Scene || Textures->Scene->width != Viewport->SizeX || Textures->Scene->height != Viewport->SizeY)
	{
		Framebuffers->DestroySceneFramebuffer();
		Textures->Scene.reset();
		Textures->Scene.reset(new SceneTextures(this, Viewport->SizeX, Viewport->SizeY, Multisample));
		RenderPasses->CreateRenderPass();
		RenderPasses->CreatePipelines();
		Framebuffers->CreateSceneFramebuffer();

		auto descriptors = DescriptorSets->GetPresentDescriptorSet();
		WriteDescriptors()
			.AddCombinedImageSampler(descriptors, 0, Textures->Scene->PPImageView.get(), Samplers->PPLinearClamp.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.AddCombinedImageSampler(descriptors, 1, Textures->DitherImageView.get(), Samplers->PPNearestRepeat.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.Execute(Device.get());
	}

	PipelineBarrier()
		.AddImage(
			Textures->Scene->ColorBuffer.get(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT)
		.Execute(Commands->GetDrawCommands(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	auto cmdbuffer = Commands->GetDrawCommands();
	RenderPasses->BeginScene(cmdbuffer, ScreenClear);

	VkBuffer vertexBuffers[] = { Buffers->SceneVertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	cmdbuffer->bindVertexBuffers(0, 1, vertexBuffers, offsets);
	cmdbuffer->bindIndexBuffer(Buffers->SceneIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

	IsLocked = true;
}

void VulkanRenderDevice::Unlock(bool Blit)
{
#if 0
	if (Blit && (StatMemory || StatResources || StatDraw))
	{
		UCanvas* canvas = Viewport->Canvas;
		canvas->CurX = 16;
		canvas->CurY = 94;
		canvas->WrappedPrintf(canvas->SmallFont, 0, "Vulkan Statistics");

		int y = 110;

		if (StatMemory)
		{
			VmaStats stats = {};
			vmaCalculateStats(Device->allocator, &stats);
			canvas->CurX = 16;
			canvas->CurY = y;
			canvas->WrappedPrintf(canvas->SmallFont, 0, "Allocated objects: %d, used bytes: %d MB\r\n", (int)stats.total.allocationCount, (int)stats.total.usedBytes / (1024 * 1024));
			y += 8;
		}

		if (StatResources)
		{
			VmaStats stats = {};
			vmaCalculateStats(Device->allocator, &stats);
			canvas->CurX = 16;
			canvas->CurY = y;
			canvas->WrappedPrintf(canvas->SmallFont, 0, "Textures in cache: %d\r\n", Textures->GetTexturesInCache());
			y += 8;
		}

		if (StatDraw)
		{
			VmaStats stats = {};
			vmaCalculateStats(Device->allocator, &stats);
			canvas->CurX = 16;
			canvas->CurY = y;
			canvas->WrappedPrintf(canvas->SmallFont, 0, "Draw calls: %d, Complex surfaces: %d, Gouraud polygons: %d, Tiles: %d; Uploads: %d, Rect Uploads: %d\r\n", Stats.DrawCalls, Stats.ComplexSurfaces, Stats.GouraudPolygons, Stats.Tiles, Stats.Uploads, Stats.RectUploads);
			y += 8;
		}
	}
#endif

	if (Blit)
	{
		Stats.DrawCalls = 0;
		Stats.ComplexSurfaces = 0;
		Stats.GouraudPolygons = 0;
		Stats.Tiles = 0;
		Stats.Uploads = 0;
		Stats.RectUploads = 0;
	}

	DrawBatch(Commands->GetDrawCommands());
	RenderPasses->EndScene(Commands->GetDrawCommands());

	BlitSceneToPostprocess();
	SubmitAndWait(Blit, Viewport->SizeX, Viewport->SizeY);

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
	uint32_t icount = SceneIndexPos - Batch.SceneIndexStart;
	if (icount > 0)
	{
		auto layout = Batch.Bindless ? RenderPasses->SceneBindlessPipelineLayout.get() : RenderPasses->ScenePipelineLayout.get();
		cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, Batch.Pipeline);
		cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, Batch.DescriptorSet);
		cmdbuffer->pushConstants(layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &pushconstants);
		cmdbuffer->drawIndexed(icount, 1, Batch.SceneIndexStart, 0, 0);
		Batch.SceneIndexStart = SceneIndexPos;
		Stats.DrawCalls++;
	}
}

void VulkanRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	CachedTexture* tex = Textures->GetTexture(Surface.Texture, !!(Surface.PolyFlags & PF_Masked));
	CachedTexture* lightmap = Textures->GetTexture(Surface.LightMap, false);
	CachedTexture* macrotex = Textures->GetTexture(Surface.MacroTexture, false);
	CachedTexture* detailtex = Textures->GetTexture(Surface.DetailTexture, false);
	CachedTexture* fogmap = Textures->GetTexture(Surface.FogMap, false);

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
	float DetailUPan = detailtex ? UDot + Surface.DetailTexture->Pan.x : 0.0f;
	float DetailVPan = detailtex ? VDot + Surface.DetailTexture->Pan.y : 0.0f;
	float DetailUMult = detailtex ? GetUMult(*Surface.DetailTexture) : 0.0f;
	float DetailVMult = detailtex ? GetVMult(*Surface.DetailTexture) : 0.0f;

	uint32_t flags = 0;
	if (lightmap) flags |= 1;
	if (macrotex) flags |= 2;
	if (detailtex && !fogmap) flags |= 4;
	if (fogmap) flags |= 8;

	if (fogmap) // if Surface.FogMap exists, use instead of detail texture
	{
		detailtex = fogmap;
		DetailUPan = UDot + Surface.FogMap->Pan.x - 0.5f * Surface.FogMap->UScale;
		DetailVPan = VDot + Surface.FogMap->Pan.y - 0.5f * Surface.FogMap->VScale;
		DetailUMult = GetUMult(*Surface.FogMap);
		DetailVMult = GetVMult(*Surface.FogMap);
	}

	SetPipeline(RenderPasses->getPipeline(Surface.PolyFlags, UsesBindless));

	ivec4 textureBinds;
	if (UsesBindless)
	{
		textureBinds.x = DescriptorSets->GetTextureArrayIndex(Surface.PolyFlags, tex);
		textureBinds.y = DescriptorSets->GetTextureArrayIndex(0, macrotex);
		textureBinds.z = DescriptorSets->GetTextureArrayIndex(0, detailtex);
		textureBinds.w = DescriptorSets->GetTextureArrayIndex(0, lightmap);

		SetDescriptorSet(DescriptorSets->GetBindlessDescriptorSet(), true);
	}
	else
	{
		textureBinds.x = 0;
		textureBinds.y = 0;
		textureBinds.z = 0;
		textureBinds.w = 0;

		SetDescriptorSet(DescriptorSets->GetTextureDescriptorSet(Surface.PolyFlags, tex, lightmap, macrotex, detailtex), false);
	}

	uint32_t vpos = SceneVertexPos;
	uint32_t ipos = SceneIndexPos;

	SceneVertex* vptr = Buffers->SceneVertices + vpos;
	uint32_t* iptr = Buffers->SceneIndexes + ipos;

	uint32_t istart = ipos;
	uint32_t icount = 0;

	for (const std::vector<vec3>& Poly : Facet.Polys)
	{
		auto pts = Poly.data();
		uint32_t vcount = (uint32_t)Poly.size();
		if (vcount < 3) continue;

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
			vptr->Color.r = 1.0f;
			vptr->Color.g = 1.0f;
			vptr->Color.b = 1.0f;
			vptr->Color.a = 1.0f;
			vptr->TextureBinds = textureBinds;
			vptr++;
		}

		for (uint32_t i = vpos + 2; i < vpos + vcount; i++)
		{
			*(iptr++) = vpos;
			*(iptr++) = i - 1;
			*(iptr++) = i;
		}

		vpos += vcount;
		icount += (vcount - 2) * 3;
	}

	Stats.ComplexSurfaces++;

	SceneVertexPos = vpos;
	SceneIndexPos = ipos + icount;
}

void VulkanRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	if (NumPts < 3) return; // This can apparently happen!!

	SetPipeline(RenderPasses->getPipeline(PolyFlags, UsesBindless));

	CachedTexture* tex = Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
	ivec4 textureBinds;
	if (UsesBindless)
	{
		textureBinds.x = DescriptorSets->GetTextureArrayIndex(PolyFlags, tex);
		textureBinds.y = 0;
		textureBinds.z = 0;
		textureBinds.w = 0;
		SetDescriptorSet(DescriptorSets->GetBindlessDescriptorSet(), true);
	}
	else
	{
		textureBinds.x = 0;
		textureBinds.y = 0;
		textureBinds.z = 0;
		textureBinds.w = 0;
		SetDescriptorSet(DescriptorSets->GetTextureDescriptorSet(PolyFlags, tex), false);
	}

	float UMult = GetUMult(Info);
	float VMult = GetVMult(Info);
	int flags = (PolyFlags & (PF_RenderFog | PF_Translucent | PF_Modulated)) == PF_RenderFog ? 16 : 0;

	if (PolyFlags & PF_Modulated)
	{
		SceneVertex* vertex = &Buffers->SceneVertices[SceneVertexPos];
		for (int i = 0; i < NumPts; i++)
		{
			const GouraudVertex* P = Pts + i;
			vertex->Flags = flags;
			vertex->Position.x = P->Point.x;
			vertex->Position.y = P->Point.y;
			vertex->Position.z = P->Point.z;
			vertex->TexCoord.s = P->UV.s * UMult;
			vertex->TexCoord.t = P->UV.t * VMult;
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
		SceneVertex* vertex = &Buffers->SceneVertices[SceneVertexPos];
		for (int i = 0; i < NumPts; i++)
		{
			const GouraudVertex* P = Pts + i;
			vertex->Flags = flags;
			vertex->Position.x = P->Point.x;
			vertex->Position.y = P->Point.y;
			vertex->Position.z = P->Point.z;
			vertex->TexCoord.s = P->UV.s * UMult;
			vertex->TexCoord.t = P->UV.t * VMult;
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

	uint32_t vstart = SceneVertexPos;
	uint32_t vcount = NumPts;
	uint32_t istart = SceneIndexPos;
	uint32_t icount = (vcount - 2) * 3;

	uint32_t* iptr = Buffers->SceneIndexes + istart;
	for (uint32_t i = vstart + 2; i < vstart + vcount; i++)
	{
		*(iptr++) = vstart;
		*(iptr++) = i - 1;
		*(iptr++) = i;
	}

	SceneVertexPos += vcount;
	SceneIndexPos += icount;

	Stats.GouraudPolygons++;
}

void VulkanRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	if ((PolyFlags & (PF_Modulated)) == PF_Modulated && Info.Format == TextureFormat::P8)
		PolyFlags = PF_Modulated;

	CachedTexture* tex = Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));

	SetPipeline(RenderPasses->getPipeline(PolyFlags, UsesBindless));

	ivec4 textureBinds;
	if (UsesBindless)
	{
		textureBinds.x = DescriptorSets->GetTextureArrayIndex(PolyFlags, tex, true);
		textureBinds.y = 0;
		textureBinds.z = 0;
		textureBinds.w = 0;

		SetDescriptorSet(DescriptorSets->GetBindlessDescriptorSet(), true);
	}
	else
	{
		textureBinds.x = 0;
		textureBinds.y = 0;
		textureBinds.z = 0;
		textureBinds.w = 0;

		SetDescriptorSet(DescriptorSets->GetTextureDescriptorSet(PolyFlags, tex, nullptr, nullptr, nullptr, true), false);
	}

	float UMult = tex ? GetUMult(Info) : 0.0f;
	float VMult = tex ? GetVMult(Info) : 0.0f;

	SceneVertex* v = &Buffers->SceneVertices[SceneVertexPos];

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

	v[0] = { 0, vec3(RFX2 * Z * (X - Frame->FX2),      RFY2 * Z * (Y - Frame->FY2),      Z), vec2(U * UMult,        V * VMult),        vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };
	v[1] = { 0, vec3(RFX2 * Z * (X + XL - Frame->FX2), RFY2 * Z * (Y - Frame->FY2),      Z), vec2((U + UL) * UMult, V * VMult),        vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };
	v[2] = { 0, vec3(RFX2 * Z * (X + XL - Frame->FX2), RFY2 * Z * (Y + YL - Frame->FY2), Z), vec2((U + UL) * UMult, (V + VL) * VMult), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };
	v[3] = { 0, vec3(RFX2 * Z * (X - Frame->FX2),      RFY2 * Z * (Y + YL - Frame->FY2), Z), vec2(U * UMult,        (V + VL) * VMult), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(r, g, b, a), textureBinds };

	uint32_t vstart = SceneVertexPos;
	uint32_t vcount = 4;
	uint32_t istart = SceneIndexPos;
	uint32_t icount = (vcount - 2) * 3;

	uint32_t* iptr = Buffers->SceneIndexes + istart;
	for (uint32_t i = vstart + 2; i < vstart + vcount; i++)
	{
		*(iptr++) = vstart;
		*(iptr++) = i - 1;
		*(iptr++) = i;
	}

	SceneVertexPos += vcount;
	SceneIndexPos += icount;

	Stats.Tiles++;
}

void VulkanRenderDevice::ClearZ(FSceneNode* Frame)
{
	DrawBatch(Commands->GetDrawCommands());

	VkClearAttachment attachment = {};
	VkClearRect rect = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	attachment.clearValue.depthStencil.depth = 1.0f;
	rect.layerCount = 1;
	rect.rect.extent.width = Textures->Scene->width;
	rect.rect.extent.height = Textures->Scene->height;
	Commands->GetDrawCommands()->clearAttachments(1, &attachment, 1, &rect);
}

void VulkanRenderDevice::ReadPixels(FColor* Pixels)
{
	int w = Viewport->SizeX;
	int h = Viewport->SizeY;
	void* data = Pixels;
	float gamma = 1.5f * Viewport->Brightness;

	auto dstimage = ImageBuilder()
		.Format(VK_FORMAT_B8G8R8A8_UNORM)
		.Usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.Size(w, h)
		.DebugName("ReadPixelsDstImage")
		.Create(Device.get());

	// Convert from rgba16f to bgra8 using the GPU:
	auto srcimage = Textures->Scene->PPImage.get();
	auto cmdbuffer = Commands->GetDrawCommands();

	DrawBatch(cmdbuffer);

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
	SubmitAndWait(false, 0, 0);

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

		SetPipeline(RenderPasses->getEndFlashPipeline());
		SetDescriptorSet(DescriptorSets->GetTextureDescriptorSet(0, nullptr), false);

		SceneVertex* v = &Buffers->SceneVertices[SceneVertexPos];

		v[0] = { 0, vec3(-1.0f, -1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };
		v[1] = { 0, vec3( 1.0f, -1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };
		v[2] = { 0, vec3( 1.0f,  1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };
		v[3] = { 0, vec3(-1.0f,  1.0f, 0.0f), zero2, zero2, zero2, zero2, color, zero4 };

		uint32_t vstart = SceneVertexPos;
		uint32_t vcount = 4;
		uint32_t istart = SceneIndexPos;
		uint32_t icount = (vcount - 2) * 3;

		uint32_t* iptr = Buffers->SceneIndexes + istart;
		for (uint32_t i = vstart + 2; i < vstart + vcount; i++)
		{
			*(iptr++) = vstart;
			*(iptr++) = i - 1;
			*(iptr++) = i;
		}

		SceneVertexPos += vcount;
		SceneIndexPos += icount;

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

	VkViewport viewportdesc = {};
	viewportdesc.x = (float)Frame->XB;
	viewportdesc.y = (float)Frame->YB;
	viewportdesc.width = (float)Frame->X;
	viewportdesc.height = (float)Frame->Y;
	viewportdesc.minDepth = 0.0f;
	viewportdesc.maxDepth = 1.0f;
	commands->setViewport(0, 1, &viewportdesc);

	pushconstants.objectToProjection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);

	// TBD; do this or do like UE1 does and do the transform on the CPU?
	pushconstants.objectToProjection = pushconstants.objectToProjection * Frame->WorldToView * Frame->ObjectToWorld;
}

void VulkanRenderDevice::PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags)
{
	Textures->GetTexture(&Info, !!(PolyFlags & PF_Masked));
}

void VulkanRenderDevice::ClearTextureCache()
{
	DescriptorSets->ClearCache();
	Textures->ClearCache();
}

void VulkanRenderDevice::BlitSceneToPostprocess()
{
	auto buffers = Textures->Scene.get();
	auto cmdbuffer = Commands->GetDrawCommands();

	PipelineBarrier()
		.AddImage(
			buffers->ColorBuffer.get(),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT)
		.AddImage(
			buffers->PPImage.get(),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT)
		.Execute(
			Commands->GetDrawCommands(),
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
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
			buffers->PPImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &resolve);
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
			buffers->PPImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_NEAREST);
	}

	PipelineBarrier()
		.AddImage(
			buffers->PPImage.get(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT)
		.Execute(
			Commands->GetDrawCommands(),
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void VulkanRenderDevice::DrawPresentTexture(int x, int y, int width, int height)
{
	float gamma = Viewport->Brightness * 2.0f;

	PresentPushConstants pushconstants;
	pushconstants.InvGamma = 1.0f / gamma;

	VkViewport viewport = {};
	viewport.x = (float)x;
	viewport.y = (float)y;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent.width = width;
	scissor.extent.height = height;

	auto cmdbuffer = Commands->GetDrawCommands();

	RenderPasses->BeginPresent(cmdbuffer);
	cmdbuffer->setViewport(0, 1, &viewport);
	cmdbuffer->setScissor(0, 1, &scissor);
	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PresentPipeline.get());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PresentPipelineLayout.get(), 0, DescriptorSets->GetPresentDescriptorSet());
	cmdbuffer->pushConstants(RenderPasses->PresentPipelineLayout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PresentPushConstants), &pushconstants);
	cmdbuffer->draw(6, 1, 0, 0);
	RenderPasses->EndPresent(cmdbuffer);
}
