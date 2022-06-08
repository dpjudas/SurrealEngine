#pragma once

#include "RenderDevice/RenderDevice.h"
#include "VulkanObjects.h"

class DisplayWindow;
class VulkanDevice;
class VulkanCommandBufferManager;
class VulkanFrameBufferManager;
class VulkanLightManager;
class VulkanRenderPassManager;
class VulkanSamplerManager;
class VulkanDescriptorSetManager;
class VulkanTextureManager;
class VulkanShaderManager;
class VulkanPostprocess;
class Postprocess;
struct SceneVertex;
struct FTextureInfo;

class VulkanRenderDevice : public RenderDevice
{
public:
	VulkanRenderDevice(DisplayWindow* InViewport);
	~VulkanRenderDevice();

	void Flush(bool AllowPrecache) override;
	void BeginFrame() override;
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

	std::unique_ptr<Postprocess> PostprocessModel;

	VulkanDevice* Device = nullptr;

	std::unique_ptr<VulkanCommandBufferManager> Commands;
	std::unique_ptr<VulkanShaderManager> Shaders;
	std::unique_ptr<VulkanPostprocess> Postprocessing;
	std::unique_ptr<VulkanFrameBufferManager> FrameBuffers;
	std::unique_ptr<VulkanSamplerManager> Samplers;
	std::unique_ptr<VulkanTextureManager> Textures;
	std::unique_ptr<VulkanLightManager> Lights;
	std::unique_ptr<VulkanDescriptorSetManager> DescriptorSets;
	std::unique_ptr<VulkanRenderPassManager> RenderPasses;

private:
	void CheckFPSLimit();
	void CopyScreenToBuffer(int w, int h, void* data, float gamma);
	void CreateSceneVertexBuffer();

	bool UsePrecache = false;
	FSceneNode* CurrentFrame = nullptr;

	// Configuration.
	bool UseVSync = true;
	int FPSLimit = 400;
	uint64_t fpsLimitTime = 0;
	int Multisample = 0;

	std::unique_ptr<VulkanBuffer> SceneVertexBuffer;
	SceneVertex* SceneVertices = nullptr;
	size_t SceneVertexPos = 0;
	static const int MaxSceneVertices = 1'000'000;
};
