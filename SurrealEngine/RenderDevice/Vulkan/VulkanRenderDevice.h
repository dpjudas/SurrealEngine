#pragma once

#include <zvulkan/vulkandevice.h>
#include <zvulkan/vulkanobjects.h>
#include "RenderDevice/RenderDevice.h"
#include "CommandBufferManager.h"
#include "BufferManager.h"
#include "DescriptorSetManager.h"
#include "FramebufferManager.h"
#include "RenderPassManager.h"
#include "SamplerManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UploadManager.h"
#include "Math/mat.h"
#include "Math/vec.h"

class CachedTexture;

class VulkanRenderDevice : public RenderDevice
{
public:
	VulkanRenderDevice(DisplayWindow* InViewport, std::shared_ptr<VulkanSurface> surface);
	~VulkanRenderDevice();

	void Flush(bool AllowPrecache) override;
	bool Exec(std::string Cmd, OutputDevice& Ar) override;
	void Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear) override;
	void Unlock(bool Blit) override;
	void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) override;
	void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) override;
	void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) override;
	void ClearZ(FSceneNode* Frame) override;
	void ReadPixels(FColor* Pixels) override;
	void EndFlash() override;
	void SetSceneNode(FSceneNode* Frame) override;
	void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) override;
	bool SupportsTextureFormat(TextureFormat Format) override;
	void UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL) override;

	std::shared_ptr<VulkanDevice> Device;

	std::unique_ptr<CommandBufferManager> Commands;

	std::unique_ptr<SamplerManager> Samplers;
	std::unique_ptr<TextureManager> Textures;
	std::unique_ptr<BufferManager> Buffers;
	std::unique_ptr<ShaderManager> Shaders;
	std::unique_ptr<UploadManager> Uploads;

	std::unique_ptr<DescriptorSetManager> DescriptorSets;
	std::unique_ptr<RenderPassManager> RenderPasses;
	std::unique_ptr<FramebufferManager> Framebuffers;

	bool SupportsBindless = false;
	bool UsesBindless = false;

	bool UseVSync = true;
	int VkDeviceIndex = 0;
	bool VkDebug = false;
	int Multisample = 4;

	void DrawPresentTexture(int x, int y, int width, int height);

	struct
	{
		int ComplexSurfaces = 0;
		int GouraudPolygons = 0;
		int Tiles = 0;
		int DrawCalls = 0;
		int Uploads = 0;
		int RectUploads = 0;
	} Stats;

private:
	void Dispose();
	void ClearTextureCache();
	void BlitSceneToPostprocess();

	bool UsePrecache = false;
	vec4 FlashScale = vec4(0.0f);
	vec4 FlashFog = vec4(0.0f);
	FSceneNode* CurrentFrame = nullptr;
	float Aspect = 0.0f;
	float RProjZ = 0.0f;
	float RFX2 = 0.0f;
	float RFY2 = 0.0f;

	bool IsLocked = false;
	bool StatMemory = false;
	bool StatResources = false;
	bool StatDraw = false;

	void SetPipeline(VulkanPipeline* pipeline);
	void SetDescriptorSet(VulkanDescriptorSet* descriptorSet, bool bindless);
	void DrawBatch(VulkanCommandBuffer* cmdbuffer);
	void SubmitAndWait(bool present, int presentWidth, int presentHeight);

	struct
	{
		uint32_t SceneIndexStart = 0;
		VulkanPipeline* Pipeline = nullptr;
		VulkanDescriptorSet* DescriptorSet = nullptr;
		bool Bindless = false;
	} Batch;

	ScenePushConstants pushconstants;

	uint32_t SceneVertexPos = 0;
	uint32_t SceneIndexPos = 0;
};

inline void VulkanRenderDevice::SetPipeline(VulkanPipeline* pipeline)
{
	if (pipeline != Batch.Pipeline)
	{
		DrawBatch(Commands->GetDrawCommands());
		Batch.Pipeline = pipeline;
	}
}

inline void VulkanRenderDevice::SetDescriptorSet(VulkanDescriptorSet* descriptorSet, bool bindless)
{
	if (descriptorSet != Batch.DescriptorSet)
	{
		DrawBatch(Commands->GetDrawCommands());
		Batch.DescriptorSet = descriptorSet;
		Batch.Bindless = bindless;
	}
}

inline float GetUMult(const FTextureInfo& Info) { return 1.0f / (Info.UScale * Info.USize); }
inline float GetVMult(const FTextureInfo& Info) { return 1.0f / (Info.VScale * Info.VSize); }
