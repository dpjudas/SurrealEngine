#pragma once

#include "RenderDevice/RenderDevice.h"

class Renderer;

class VulkanRenderDevice : public RenderDevice
{
public:
	VulkanRenderDevice(::Window* InViewport);
	~VulkanRenderDevice();

	void Flush(bool AllowPrecache) override;
	void BeginFrame() override;
	void BeginShadowmapUpdate() override;
	void BeginShadowmapPass() override;
	void EndShadowmapPass(int slot) override;
	void EndShadowmapUpdate() override;
	void BeginScenePass() override;
	void EndScenePass() override;
	void EndFrame(bool Blit) override;
	void UpdateLights(const std::vector<std::pair<int, UActor*>>& LightUpdates) override;
	void UpdateSurfaceLights(const std::vector<int32_t>& SurfaceLights) override;
	void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) override;
	void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo* Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) override;
	void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) override;
	void ClearZ(FSceneNode* Frame) override;
	void ReadPixels(FColor* Pixels) override;
	void EndFlash(float FlashScale, vec4 FlashFog) override;
	void SetSceneNode(FSceneNode* Frame) override;
	void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) override;

	std::unique_ptr<Renderer> renderer;

private:
	void CheckFPSLimit();

	bool UsePrecache = false;
	FSceneNode* CurrentFrame = nullptr;

	bool IsShadowPass = false;

	// Configuration.
	bool UseVSync = true;
	int FPSLimit = 400;
	uint64_t fpsLimitTime = 0;
	int VkDeviceIndex = 0;
	bool VkDebug = false;
	int Multisample = 0;
};
