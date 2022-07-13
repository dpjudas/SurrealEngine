
#include "Precomp.h"
#include "VulkanRenderDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanPostprocess.h"
#include "VulkanTexture.h"
#include "VulkanFrameBuffer.h"
#include "VulkanLight.h"
#include "VulkanRaytrace.h"
#include "VulkanRenderPass.h"
#include "VulkanSampler.h"
#include "VulkanDescriptorSet.h"
#include "VulkanCommandBuffer.h"
#include "VulkanShader.h"
#include "Window/Window.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "Engine.h"
#include "VulkanRenderDevice.h"
#include "UTF16.h"
#include <chrono>
#include <thread>
#include <iostream>

VulkanRenderDevice::VulkanRenderDevice(DisplayWindow* viewport)
{
	Viewport = viewport;
	Device = viewport->GetVulkanDevice();
	Commands = std::make_unique<VulkanCommandBufferManager>(this, /*vsync*/true);
	Shaders = std::make_unique<VulkanShaderManager>(this);
	Samplers = std::make_unique<VulkanSamplerManager>(Device);
	Textures = std::make_unique<VulkanTextureManager>(this);
	Lights = std::make_unique<VulkanLightManager>(this);
	Raytrace = std::make_unique<VulkanRaytraceManager>(this);
	DescriptorSets = std::make_unique<VulkanDescriptorSetManager>(this);
	PostprocessModel = std::make_unique<Postprocess>();
	Postprocessing = std::make_unique<VulkanPostprocess>(this);

	CreateSceneVertexBuffer();

	Flush(true);
}

VulkanRenderDevice::~VulkanRenderDevice()
{
	if (Device)
		vkDeviceWaitIdle(Device->device);
	if (SceneVertices)
	{
		SceneVertexBuffer->Unmap();
		SceneVertices = nullptr;
	}
}

void VulkanRenderDevice::Flush(bool AllowPrecache)
{
	Textures->ClearTextureCache();

	if (AllowPrecache && UsePrecache)
		PrecacheOnFlip = true;
}

void VulkanRenderDevice::BeginFrame()
{
	if (!FrameBuffers || FrameBuffers->width != Viewport->SizeX || FrameBuffers->height != Viewport->SizeY)
	{
		RenderPasses.reset();
		FrameBuffers.reset();
		FrameBuffers = std::make_unique<VulkanFrameBufferManager>(this, Viewport->SizeX, Viewport->SizeY, Multisample);
		RenderPasses = std::make_unique<VulkanRenderPassManager>(this);
	}

	DescriptorSets->UpdateLightSet();

	auto cmdbuffer = Commands->GetDrawCommands();

	Postprocessing->beginFrame();
	Postprocessing->imageTransitionScene(true);
}

void VulkanRenderDevice::UpdateLights(const std::vector<std::pair<int, UActor*>>& LightUpdates)
{
	Lights->UpdateLights(LightUpdates);
}

void VulkanRenderDevice::UpdateSurfaceLights(const std::vector<int32_t>& SurfaceLights)
{
	Lights->UpdateSurfaceLights(SurfaceLights);
}

void VulkanRenderDevice::BeginScenePass()
{
	RenderPasses->begin(Commands->GetDrawCommands());

	VkBuffer vertexBuffers[] = { SceneVertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	Commands->GetDrawCommands()->bindVertexBuffers(0, 1, vertexBuffers, offsets);
}

void VulkanRenderDevice::EndScenePass()
{
	RenderPasses->end(Commands->GetDrawCommands());
}

void VulkanRenderDevice::EndFrame(bool Blit)
{
	Postprocessing->blitSceneToPostprocess();

	int sceneWidth = FrameBuffers->colorBuffer->width;
	int sceneHeight = FrameBuffers->colorBuffer->height;

	if (Blit)
		CheckFPSLimit();

	PostprocessModel->present.gamma = 2.5f * Viewport->Brightness;

	Commands->SubmitCommands(Blit ? true : false, Viewport->SizeX, Viewport->SizeY);
	SceneVertexPos = 0;
}

void VulkanRenderDevice::CheckFPSLimit()
{
	using namespace std::chrono;
	using namespace std::this_thread;

	if (UseVSync || FPSLimit <= 0)
		return;

	uint64_t targetWakeTime = fpsLimitTime + 1'000'000 / FPSLimit;

	while (true)
	{
		fpsLimitTime = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
		int64_t timeToWait = targetWakeTime - fpsLimitTime;

		if (timeToWait > 1'000'000 || timeToWait <= 0)
		{
			break;
		}

		if (timeToWait <= 2'000)
		{
			// We are too close to the deadline. OS sleep is not precise enough to wake us before it elapses.
			// Yield execution and check time again.
			sleep_for(nanoseconds(0));
		}
		else
		{
			// Sleep, but try to wake before deadline.
			sleep_for(microseconds(timeToWait - 2'000));
		}
	}
}

void VulkanRenderDevice::DrawComplexSurface(FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	if (SceneVertexPos + Facet.Points.size() > MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	VulkanCommandBuffer* cmdbuffer = Commands->GetDrawCommands();

	VulkanTexture* tex = Textures->GetTexture(Surface.Texture, Surface.PolyFlags);
	VulkanTexture* lightmap = Textures->GetTexture(Surface.LightMap, 0);
	VulkanTexture* macrotex = Textures->GetTexture(Surface.MacroTexture, 0);
	VulkanTexture* detailtex = Textures->GetTexture(Surface.DetailTexture, 0);
	VulkanTexture* fogmap = Textures->GetTexture(Surface.FogMap, 0);

	if ((Surface.DetailTexture && Surface.FogMap)) detailtex = nullptr;

	float UDot = dot(Facet.MapCoords.XAxis, Facet.MapCoords.Origin);
	float VDot = dot(Facet.MapCoords.YAxis, Facet.MapCoords.Origin);

	float UPan = tex ? UDot + Surface.Texture->Pan.x : 0.0f;
	float VPan = tex ? VDot + Surface.Texture->Pan.y : 0.0f;
	float UMult = tex ? tex->UMult : 0.0f;
	float VMult = tex ? tex->VMult : 0.0f;
	float LMUPan = lightmap ? UDot + Surface.LightMap->Pan.x - 0.5f * Surface.LightMap->UScale : 0.0f;
	float LMVPan = lightmap ? VDot + Surface.LightMap->Pan.y - 0.5f * Surface.LightMap->VScale : 0.0f;
	float LMUMult = lightmap ? lightmap->UMult : 0.0f;
	float LMVMult = lightmap ? lightmap->VMult : 0.0f;
	float MacroUPan = macrotex ? UDot + Surface.MacroTexture->Pan.x : 0.0f;
	float MacroVPan = macrotex ? VDot + Surface.MacroTexture->Pan.y : 0.0f;
	float MacroUMult = macrotex ? macrotex->UMult : 0.0f;
	float MacroVMult = macrotex ? macrotex->VMult : 0.0f;
	float DetailUPan = detailtex ? UDot + Surface.DetailTexture->Pan.x : 0.0f;
	float DetailVPan = detailtex ? VDot + Surface.DetailTexture->Pan.y : 0.0f;
	float DetailUMult = detailtex ? detailtex->UMult : 0.0f;
	float DetailVMult = detailtex ? detailtex->VMult : 0.0f;

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
		DetailUMult = fogmap->UMult;
		DetailVMult = fogmap->VMult;
	}

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->getPipeline(Surface.PolyFlags));
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 0, DescriptorSets->GetLightSet());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 1, DescriptorSets->GetTextureSet(Surface.PolyFlags, tex, lightmap, macrotex, detailtex));

	SceneVertex* vertexdata = &SceneVertices[SceneVertexPos];

	for (const vec3& point : Facet.Points)
	{
		SceneVertex& vtx = *(vertexdata++);

		float u = dot(Facet.MapCoords.XAxis, point);
		float v = dot(Facet.MapCoords.YAxis, point);

		vtx.flags = flags;
		vtx.x = point.x;
		vtx.y = point.y;
		vtx.z = point.z;
		vtx.u = (u - UPan) * UMult;
		vtx.v = (v - VPan) * VMult;
		vtx.u2 = (u - LMUPan) * LMUMult;
		vtx.v2 = (v - LMVPan) * LMVMult;
		vtx.u3 = (u - MacroUPan) * MacroUMult;
		vtx.v3 = (v - MacroVPan) * MacroVMult;
		vtx.u4 = (u - DetailUPan) * DetailUMult;
		vtx.v4 = (v - DetailVPan) * DetailVMult;
		vtx.r = 1.0f;
		vtx.g = 1.0f;
		vtx.b = 1.0f;
		vtx.a = 1.0f;
	}

	uint32_t start = (uint32_t)SceneVertexPos;
	uint32_t count = (uint32_t)Facet.Points.size();
	SceneVertexPos += count;

	cmdbuffer->draw(count, 1, start, 0);
}

void VulkanRenderDevice::DrawGouraudPolygon(FTextureInfo* Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	if (SceneVertexPos + NumPts > MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	VulkanTexture* tex = Textures->GetTexture(Info, PolyFlags);

	auto cmdbuffer = Commands->GetDrawCommands();

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->getPipeline(PolyFlags));
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 0, DescriptorSets->GetLightSet());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 1, DescriptorSets->GetTextureSet(PolyFlags, tex));

	float UMult = tex ? tex->UMult : 0.0f;
	float VMult = tex ? tex->VMult : 0.0f;

	SceneVertex* vertexdata = &SceneVertices[SceneVertexPos];

	for (int i = 0; i < NumPts; i++)
	{
		const GouraudVertex* P = &Pts[i];
		SceneVertex& v = vertexdata[i];
		v.flags = 0;
		v.x = P->Point.x;
		v.y = P->Point.y;
		v.z = P->Point.z;
		v.u = P->UV.x * UMult;
		v.v = P->UV.y * VMult;
		v.u2 = 0.0f;
		v.v2 = 0.0f;
		v.u3 = 0.0f;
		v.v3 = 0.0f;
		v.u4 = 0.0f;
		v.v4 = 0.0f;

		if (PolyFlags & PF_Modulated)
		{
			v.r = 1.0f;
			v.g = 1.0f;
			v.b = 1.0f;
			v.a = 1.0f;
		}
		else
		{
			v.r = P->Light.x;
			v.g = P->Light.y;
			v.b = P->Light.z;
			v.a = 1.0f;
		}
	}

	uint32_t start = (uint32_t)SceneVertexPos;
	uint32_t count = NumPts;
	SceneVertexPos += count;

	cmdbuffer->draw(count, 1, start, 0);
}

void VulkanRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	if (SceneVertexPos + 4 > MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	if ((PolyFlags & (PF_Modulated)) == (PF_Modulated) && Info.Texture->ActualFormat == TextureFormat::P8)
		PolyFlags = PF_Modulated;

	auto cmdbuffer = Commands->GetDrawCommands();

	VulkanTexture* tex = Textures->GetTexture(&Info, PolyFlags);

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->getPipeline(PolyFlags));
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 0, DescriptorSets->GetLightSet());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 1, DescriptorSets->GetTextureSet(PolyFlags, tex, nullptr, nullptr, nullptr, true));

	float UMult = tex ? tex->UMult : 0.0f;
	float VMult = tex ? tex->VMult : 0.0f;

	SceneVertex* v = &SceneVertices[SceneVertexPos];

	float r, g, b;
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

	vec4 quad[4] =
	{
		{ X - Frame->FX2,      Y - Frame->FY2,      U,      V },
		{ X + XL - Frame->FX2, Y - Frame->FY2,      U + UL, V },
		{ X + XL - Frame->FX2, Y + YL - Frame->FY2, U + UL, V + VL },
		{ X - Frame->FX2,      Y + YL - Frame->FY2, U,      V + VL }
	};

	for (int i = 0; i < 4; i++)
	{
		vec3 p = { quad[i].x / Frame->FX2, quad[i].y / Frame->FY2, 0.0f };
		v[i] = { 16, p.x, p.y, p.z, quad[i].z * UMult, quad[i].w * VMult, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, 1.0f };
	}

	uint32_t start = (uint32_t)SceneVertexPos;
	uint32_t count = 4;
	SceneVertexPos += count;
	cmdbuffer->draw(count, 1, start, 0);
}

void VulkanRenderDevice::ClearZ(FSceneNode* Frame)
{
	VkClearAttachment attachment = {};
	VkClearRect rect = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	attachment.clearValue.depthStencil.depth = 1.0f;
	rect.layerCount = 1;
	rect.rect.extent.width = FrameBuffers->width;
	rect.rect.extent.height = FrameBuffers->height;
	Commands->GetDrawCommands()->clearAttachments(1, &attachment, 1, &rect);
}

void VulkanRenderDevice::ReadPixels(FColor* Pixels)
{
	CopyScreenToBuffer(Viewport->SizeX, Viewport->SizeY, Pixels, 2.5f * Viewport->Brightness);
}

void VulkanRenderDevice::EndFlash(float FlashScale, vec4 FlashFog)
{
	if (FlashScale == 0.5f || FlashFog == vec4(0.0f, 0.0f, 0.0f, 0.0f))
		return;

	if (SceneVertexPos + 4 > MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	float r = FlashFog.x;
	float g = FlashFog.y;
	float b = FlashFog.z;
	float a = 1.0f - std::min(FlashScale * 2.0f, 1.0f);
	r *= a;
	g *= a;
	b *= a;

	auto cmdbuffer = Commands->GetDrawCommands();

	ScenePushConstants pushconstants;
	pushconstants.objectToWorld = mat4::identity();
	pushconstants.worldToProjection = mat4::identity();
	cmdbuffer->pushConstants(RenderPasses->PipelineLayout.get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &pushconstants);

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->getEndFlashPipeline());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 0, DescriptorSets->GetLightSet());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, RenderPasses->PipelineLayout.get(), 1, DescriptorSets->GetTextureSet(0, nullptr));

	SceneVertex* v = &SceneVertices[SceneVertexPos];

	v[0] = { 0, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };
	v[1] = { 0,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };
	v[2] = { 0,  1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };
	v[3] = { 0, -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };

	uint32_t start = (uint32_t)SceneVertexPos;
	uint32_t count = 4;
	SceneVertexPos += count;
	cmdbuffer->draw(count, 1, start, 0);

	if (CurrentFrame)
		SetSceneNode(CurrentFrame);
}

void VulkanRenderDevice::SetSceneNode(FSceneNode* Frame)
{
	CurrentFrame = Frame;

	auto commands = Commands->GetDrawCommands();

	VkViewport viewportdesc = {};
	viewportdesc.x = (float)Frame->XB;
	viewportdesc.y = (float)Frame->YB;
	viewportdesc.width = (float)Frame->X;
	viewportdesc.height = (float)Frame->Y;
	viewportdesc.minDepth = 0.0f;
	viewportdesc.maxDepth = 1.0f;
	commands->setViewport(0, 1, &viewportdesc);

	ScenePushConstants pushconstants;
	pushconstants.objectToWorld = Frame->ObjectToWorld;
	pushconstants.worldToProjection = Frame->Projection * Frame->WorldToView;
	commands->pushConstants(RenderPasses->PipelineLayout.get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &pushconstants);
}

void VulkanRenderDevice::PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags)
{
	Textures->GetTexture(&Info, PolyFlags);
}

void VulkanRenderDevice::CreateSceneVertexBuffer()
{
	size_t size = sizeof(SceneVertex) * MaxSceneVertices;

	BufferBuilder builder;
	builder.setUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_UNKNOWN, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
	builder.setMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	builder.setSize(size);

	SceneVertexBuffer = builder.create(Device);
	SceneVertices = (SceneVertex*)SceneVertexBuffer->Map(0, size);
	SceneVertexPos = 0;
}

void VulkanRenderDevice::CopyScreenToBuffer(int w, int h, void* data, float gamma)
{
	// Convert from rgba16f to bgra8 using the GPU:
	ImageBuilder imgbuilder;
	imgbuilder.setFormat(VK_FORMAT_B8G8R8A8_UNORM);
	imgbuilder.setUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	imgbuilder.setSize(w, h);
	auto image = imgbuilder.create(Device);
	VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	Postprocessing->blitCurrentToImage(image.get(), &imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// Staging buffer for download
	BufferBuilder bufbuilder;
	bufbuilder.setSize((size_t)w * h * 4);
	bufbuilder.setUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_TO_CPU);
	auto staging = bufbuilder.create(Device);

	// Copy from image to buffer
	VkBufferImageCopy region = {};
	region.imageExtent.width = w;
	region.imageExtent.height = h;
	region.imageExtent.depth = 1;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Commands->GetDrawCommands()->copyImageToBuffer(image->image, imageLayout, staging->buffer, 1, &region);

	// Submit command buffers and wait for device to finish the work
	Commands->SubmitCommands(false, 0, 0);

	uint8_t* pixels = (uint8_t*)staging->Map(0, (size_t)w * h * 4);
	if (gamma != 1.0f)
	{
		float invGamma = 1.0f / gamma;

		uint8_t gammatable[256];
		for (int i = 0; i < 256; i++)
			gammatable[i] = (int)clamp(std::round(std::pow(i / 255.0f, invGamma) * 255.0f), 0.0f, 255.0f);

		uint8_t* dest = (uint8_t*)data;
		for (int i = 0; i < w * h * 4; i += 4)
		{
			dest[i] = gammatable[pixels[i]];
			dest[i + 1] = gammatable[pixels[i + 1]];
			dest[i + 2] = gammatable[pixels[i + 2]];
			dest[i + 3] = pixels[i + 3];
		}
	}
	else
	{
		memcpy(data, pixels, (size_t)w * h * 4);
	}
	staging->Unmap();
}
