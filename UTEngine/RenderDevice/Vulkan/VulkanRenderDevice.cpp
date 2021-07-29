
#include "Precomp.h"
#include "VulkanRenderDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanPostprocess.h"
#include "VulkanTexture.h"
#include "SceneBuffers.h"
#include "SceneLights.h"
#include "SceneRenderPass.h"
#include "ShadowmapRenderPass.h"
#include "SceneSamplers.h"
#include "Window/Window.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "Engine.h"
#include "Renderer.h"
#include "UTF16.h"
#include <chrono>
#include <thread>
#include <iostream>

VulkanRenderDevice::VulkanRenderDevice(DisplayWindow* viewport)
{
	Viewport = viewport;
	renderer = std::make_unique<Renderer>(viewport, true);
	Flush(true);
}

VulkanRenderDevice::~VulkanRenderDevice()
{
}

void VulkanRenderDevice::Flush(bool AllowPrecache)
{
	renderer->ClearTextureCache();

	if (AllowPrecache && UsePrecache)
		PrecacheOnFlip = true;
}

void VulkanRenderDevice::BeginFrame()
{
	if (!renderer->SceneBuffers_ || renderer->SceneBuffers_->width != Viewport->SizeX || renderer->SceneBuffers_->height != Viewport->SizeY)
	{
		renderer->ShadowmapRenderPass_.reset();
		renderer->SceneRenderPass_.reset();
		renderer->SceneBuffers_.reset();
		renderer->SceneBuffers_ = std::make_unique<SceneBuffers>(renderer.get(), Viewport->SizeX, Viewport->SizeY, Multisample);
		renderer->SceneRenderPass_ = std::make_unique<SceneRenderPass>(renderer.get());
		renderer->ShadowmapRenderPass_ = std::make_unique<ShadowmapRenderPass>(renderer.get());
	}

	auto cmdbuffer = renderer->GetDrawCommands();

	renderer->Postprocess_->beginFrame();
	renderer->Postprocess_->imageTransitionScene(true);
}

void VulkanRenderDevice::BeginShadowmapUpdate()
{
	PipelineBarrier barrier;
	barrier.addImage(renderer->SceneLights_->Shadowmap.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	barrier.execute(renderer->GetDrawCommands(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

void VulkanRenderDevice::BeginShadowmapPass()
{
	renderer->ShadowmapRenderPass_->begin(renderer->GetDrawCommands());

	VkBuffer vertexBuffers[] = { renderer->SceneVertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	renderer->GetDrawCommands()->bindVertexBuffers(0, 1, vertexBuffers, offsets);

	IsShadowPass = true;
}

void VulkanRenderDevice::EndShadowmapPass(int slot)
{
	renderer->ShadowmapRenderPass_->end(renderer->GetDrawCommands());

	VkImageCopy region = {};
	region.extent.width = renderer->SceneLights_->ShadowmapSize;
	region.extent.height = renderer->SceneLights_->ShadowmapSize;
	region.extent.depth = 1;
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.layerCount = 1;
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.layerCount = 1;
	region.dstOffset.x = slot * renderer->SceneLights_->ShadowmapSize % 4096;
	region.dstOffset.y = slot * renderer->SceneLights_->ShadowmapSize / 4096;
	renderer->GetDrawCommands()->copyImage(renderer->SceneLights_->ShadowColorBuffer->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, renderer->SceneLights_->Shadowmap->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	IsShadowPass = false;
}

void VulkanRenderDevice::EndShadowmapUpdate()
{
	PipelineBarrier barrier;
	barrier.addImage(renderer->SceneLights_->Shadowmap.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
	barrier.execute(renderer->GetDrawCommands(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void VulkanRenderDevice::UpdateLights(const std::vector<std::pair<int, UActor*>>& LightUpdates)
{
	if (LightUpdates.empty())
		return;

	int minIndex = LightUpdates.front().first;
	int maxIndex = LightUpdates.front().first;
	for (auto& update : LightUpdates)
	{
		minIndex = std::min(update.first, minIndex);
		maxIndex = std::max(update.first, maxIndex);
	}
	int count = maxIndex - minIndex + 1;

	size_t offset = minIndex * sizeof(SceneLight);
	size_t size = count * sizeof(SceneLight);
	SceneLight* dest = (SceneLight*)renderer->SceneLights_->StagingLights->Map(offset, size);

	for (auto& update : LightUpdates)
	{
		int index = update.first;
		UActor* slight = update.second;
		SceneLight& dlight = dest[index - minIndex];

		dlight.Location = slight->Location();
		dlight.Shadowmap = (float)(index * 6);
		dlight.LightBrightness = (float)slight->LightBrightness();
		dlight.LightHue = (float)slight->LightHue();
		dlight.LightSaturation = (float)slight->LightSaturation();
		dlight.LightRadius = (float)slight->LightRadius();
	}

	renderer->SceneLights_->StagingLights->Unmap();

	auto cmdbuffer = renderer->GetTransferCommands();
	cmdbuffer->copyBuffer(renderer->SceneLights_->StagingLights.get(), renderer->SceneLights_->Lights.get(), offset, size);
}

void VulkanRenderDevice::UpdateSurfaceLights(const std::vector<int32_t>& SurfaceLights)
{
	if (SurfaceLights.empty())
		return;

	size_t size = SurfaceLights.size() * sizeof(int32_t);
	int32_t* dest = (int32_t*)renderer->SceneLights_->StagingSurfaceLights->Map(0, size);
	memcpy(dest, SurfaceLights.data(), size);
	renderer->SceneLights_->StagingSurfaceLights->Unmap();

	auto cmdbuffer = renderer->GetTransferCommands();
	cmdbuffer->copyBuffer(renderer->SceneLights_->StagingSurfaceLights.get(), renderer->SceneLights_->SurfaceLights.get(), 0, size);
}

void VulkanRenderDevice::BeginScenePass()
{
	renderer->SceneRenderPass_->begin(renderer->GetDrawCommands());

	VkBuffer vertexBuffers[] = { renderer->SceneVertexBuffer->buffer };
	VkDeviceSize offsets[] = { 0 };
	renderer->GetDrawCommands()->bindVertexBuffers(0, 1, vertexBuffers, offsets);
}

void VulkanRenderDevice::EndScenePass()
{
	renderer->SceneRenderPass_->end(renderer->GetDrawCommands());
}

void VulkanRenderDevice::EndFrame(bool Blit)
{
	renderer->Postprocess_->blitSceneToPostprocess();

	int sceneWidth = renderer->SceneBuffers_->colorBuffer->width;
	int sceneHeight = renderer->SceneBuffers_->colorBuffer->height;

	if (Blit)
		CheckFPSLimit();

	renderer->PostprocessModel->present.gamma = 2.5f * Viewport->Brightness;

	renderer->SubmitCommands(Blit ? true : false, Viewport->SizeX, Viewport->SizeY);
	renderer->SceneVertexPos = 0;
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

void VulkanRenderDevice::DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet)
{
	if (renderer->SceneVertexPos + Facet.Points.size() > renderer->MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	VulkanCommandBuffer* cmdbuffer = renderer->GetDrawCommands();

	VulkanTexture* tex = renderer->GetTexture(Surface.Texture, Surface.PolyFlags);
	VulkanTexture* lightmap = renderer->GetTexture(Surface.LightMap, 0);
	VulkanTexture* macrotex = renderer->GetTexture(Surface.MacroTexture, 0);
	VulkanTexture* detailtex = renderer->GetTexture(Surface.DetailTexture, 0);
	VulkanTexture* fogmap = renderer->GetTexture(Surface.FogMap, 0);

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

	if (!IsShadowPass)
		cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->SceneRenderPass_->getPipeline(Surface.PolyFlags));
	else
		cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->ShadowmapRenderPass_->getPipeline());

	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->ScenePipelineLayout.get(), 0, renderer->GetTextureDescriptorSet(Surface.PolyFlags, tex, lightmap, macrotex, detailtex));

	SceneVertex* vertexdata = &renderer->SceneVertices[renderer->SceneVertexPos];

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

	uint32_t start = (uint32_t)renderer->SceneVertexPos;
	uint32_t count = (uint32_t)Facet.Points.size();
	renderer->SceneVertexPos += count;

	cmdbuffer->draw(count, 1, start, 0);
}

void VulkanRenderDevice::DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo* Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags)
{
	if (renderer->SceneVertexPos + NumPts > renderer->MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	auto cmdbuffer = renderer->GetDrawCommands();

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->SceneRenderPass_->getPipeline(PolyFlags));

	VulkanTexture* tex = renderer->GetTexture(Info, PolyFlags);
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->ScenePipelineLayout.get(), 0, renderer->GetTextureDescriptorSet(PolyFlags, tex));

	float UMult = tex ? tex->UMult : 0.0f;
	float VMult = tex ? tex->VMult : 0.0f;

	SceneVertex* vertexdata = &renderer->SceneVertices[renderer->SceneVertexPos];

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

	uint32_t start = (uint32_t)renderer->SceneVertexPos;
	uint32_t count = NumPts;
	renderer->SceneVertexPos += count;

	cmdbuffer->draw(count, 1, start, 0);
}

void VulkanRenderDevice::DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	if (renderer->SceneVertexPos + 4 > renderer->MaxSceneVertices)
	{
		throw std::runtime_error("Scene vertex buffer is too small!");
	}

	if ((PolyFlags & (PF_Modulated)) == (PF_Modulated) && Info.Texture->ActualFormat == TextureFormat::P8)
		PolyFlags = PF_Modulated;

	auto cmdbuffer = renderer->GetDrawCommands();

	VulkanTexture* tex = renderer->GetTexture(&Info, PolyFlags);

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->SceneRenderPass_->getPipeline(PolyFlags));
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->ScenePipelineLayout.get(), 0, renderer->GetTextureDescriptorSet(PolyFlags, tex, nullptr, nullptr, nullptr, true));

	float UMult = tex ? tex->UMult : 0.0f;
	float VMult = tex ? tex->VMult : 0.0f;

	SceneVertex* v = &renderer->SceneVertices[renderer->SceneVertexPos];

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

	uint32_t start = (uint32_t)renderer->SceneVertexPos;
	uint32_t count = 4;
	renderer->SceneVertexPos += count;
	cmdbuffer->draw(count, 1, start, 0);
}

void VulkanRenderDevice::ClearZ(FSceneNode* Frame)
{
	VkClearAttachment attachment = {};
	VkClearRect rect = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	attachment.clearValue.depthStencil.depth = 1.0f;
	rect.layerCount = 1;
	rect.rect.extent.width = renderer->SceneBuffers_->width;
	rect.rect.extent.height = renderer->SceneBuffers_->height;
	renderer->GetDrawCommands()->clearAttachments(1, &attachment, 1, &rect);
}

void VulkanRenderDevice::ReadPixels(FColor* Pixels)
{
	renderer->CopyScreenToBuffer(Viewport->SizeX, Viewport->SizeY, Pixels, 2.5f * Viewport->Brightness);
}

void VulkanRenderDevice::EndFlash(float FlashScale, vec4 FlashFog)
{
	if (FlashScale == 0.5f || FlashFog == vec4(0.0f, 0.0f, 0.0f, 0.0f))
		return;

	if (renderer->SceneVertexPos + 4 > renderer->MaxSceneVertices)
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

	auto cmdbuffer = renderer->GetDrawCommands();

	ScenePushConstants pushconstants;
	pushconstants.objectToProjection = mat4::identity();
	cmdbuffer->pushConstants(renderer->ScenePipelineLayout.get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &pushconstants);

	cmdbuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->SceneRenderPass_->getEndFlashPipeline());
	cmdbuffer->bindDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->ScenePipelineLayout.get(), 0, renderer->GetTextureDescriptorSet(0, nullptr));

	SceneVertex* v = &renderer->SceneVertices[renderer->SceneVertexPos];

	v[0] = { 0, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };
	v[1] = { 0,  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };
	v[2] = { 0,  1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };
	v[3] = { 0, -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, r, g, b, a };

	uint32_t start = (uint32_t)renderer->SceneVertexPos;
	uint32_t count = 4;
	renderer->SceneVertexPos += count;
	cmdbuffer->draw(count, 1, start, 0);

	if (CurrentFrame)
		SetSceneNode(CurrentFrame);
}

void VulkanRenderDevice::SetSceneNode(FSceneNode* Frame)
{
	CurrentFrame = Frame;

	auto commands = renderer->GetDrawCommands();

	if (!IsShadowPass)
	{
		VkViewport viewportdesc = {};
		viewportdesc.x = (float)Frame->XB;
		viewportdesc.y = (float)Frame->YB;
		viewportdesc.width = (float)Frame->X;
		viewportdesc.height = (float)Frame->Y;
		viewportdesc.minDepth = 0.0f;
		viewportdesc.maxDepth = 1.0f;
		commands->setViewport(0, 1, &viewportdesc);
	}

	ScenePushConstants pushconstants;
	pushconstants.objectToProjection = Frame->Projection * Frame->Modelview;
	commands->pushConstants(renderer->ScenePipelineLayout.get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ScenePushConstants), &pushconstants);
}

void VulkanRenderDevice::PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags)
{
	renderer->GetTexture(&Info, PolyFlags);
}
