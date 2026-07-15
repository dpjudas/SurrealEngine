#pragma once

#include "RenderDevice/RenderDevice.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"
#include "CommandBufferManager.h"
#include "BufferManager.h"
#include "DescriptorSetManager.h"
#include "FramebufferManager.h"
#include "RenderPassManager.h"
#include "SamplerManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "UploadManager.h"
#include "Math/vec.h"
#include "Math/mat.h"
#include "VR/VRSubsystem.h"

class CachedTexture;

class VulkanRenderDevice : public RenderDevice
{
public:
	VulkanRenderDevice(Widget* viewport, VRSubsystem* vr = nullptr);
	~VulkanRenderDevice();

	void Flush(bool AllowPrecache) override;
	void Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear, uint8_t* HitData, int* HitSize) override;
	void Unlock(bool Blit) override;
	void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) override;
	void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) override;
	void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) override;
	void Draw3DLine(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 OrigP, vec3 OrigQ) override;
	void Draw2DLine(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2) override;
	void Draw2DPoint(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, float X1, float Y1, float X2, float Y2, float Z) override;
	void ClearZ() override;
	void PushHit(const uint8_t* Data, int Count) override;
	void PopHit(int Count, bool bForce) override;
	void ReadPixels(FColor* Pixels) override;
	void EndFlash() override;
	void SetSceneNode(FSceneNode* Frame) override;
	void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) override;

	bool IsVRActive() const override { return VRSys && VRSys->IsActive(); }
	void BeginEyeFrame(int eyeIndex) override;
	void EndEyeFrame(int eyeIndex) override;
	void BeginUICanvasFrame() override;
	void EndUICanvasFrame() override;
	void DrawVRMenuPlane(FSceneNode* Frame, const vec3 Corners[4], const vec2 UVs[4]) override;

	void SetHitLocation();

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

	int VkDeviceIndex = 0;

	void RunBloomPass();
	void BloomStep(VulkanCommandBuffer* cmdbuffer, VulkanPipeline* pipeline, VulkanDescriptorSet* input, VulkanFramebuffer* output, int width, int height, const BloomPushConstants &pushconstants);
	static float ComputeBlurGaussian(float n, float theta);
	static void ComputeBlurSamples(int sampleCount, float blurAmount, float* sampleWeights);

	void DrawPresentTexture(int width, int height);
	PresentPushConstants GetPresentPushConstants();

	struct
	{
		int ComplexSurfaces = 0;
		int GouraudPolygons = 0;
		int Tiles = 0;
		int DrawCalls = 0;
		int Uploads = 0;
		int RectUploads = 0;
	} Stats;

	int GetSettingsMultisample()
	{
		switch (AntialiasMode)
		{
		default:
		case 0: return 0;
		case 1: return 2;
		case 2: return 4;
		}
	}

private:
	void ClearTextureCache();
	void BlitSceneToPostprocess();

	int GetSceneWidth() const;
	int GetSceneHeight() const;

	VRSubsystem* VRSys = nullptr;

	// Which render target the scene draw calls currently go to. These used to be sign-encoded into a
	// single int, which is what let "is this a VR eye?" quietly also match the menu canvas. Which eye is
	// current doesn't need tracking - Begin/EndEyeFrame are both handed the index.
	enum class RenderTarget
	{
		Desktop,      // the game window
		Eye,          // one of the headset's eyes
		VRMenuCanvas  // the offscreen canvas the VR menu plane samples (see BeginUICanvasFrame)
	};
	RenderTarget CurrentTarget = RenderTarget::Desktop;

	// Textures->Scene and all of Framebuffers' scene-derived framebuffers are swapped between the sets
	// below by SwapSceneResources(), so every other draw call in this file keeps addressing "the current
	// scene" without needing to know about VR at all. VR eyes and the menu canvas don't use bloom, but
	// the framebuffers still need swapping or CreateSceneFramebuffer() would permanently clobber the
	// desktop's with ones pointing at an eye's (later destroyed) images.
	struct SceneResources
	{
		std::unique_ptr<SceneTextures> Scene;
		std::unique_ptr<VulkanFramebuffer> SceneFramebuffer;
		std::unique_ptr<VulkanFramebuffer> PPImageFB[2];
		struct
		{
			std::unique_ptr<VulkanFramebuffer> VTextureFB;
			std::unique_ptr<VulkanFramebuffer> HTextureFB;
		} BloomBlurLevels[NumBloomLevels];
	};
	void SwapSceneResources(SceneResources& resources);
	SceneResources VREye[2];
	SceneResources UICanvas; // see BeginUICanvasFrame/EndUICanvasFrame/DrawVRMenuPlane

	// The scene the Present/Bloom descriptor sets are currently bound to. They sample whichever scene
	// textures are swapped in, so the binding has to follow SwapSceneResources rather than only being
	// refreshed when a target's textures happen to get (re)created - see Lock().
	SceneTextures* FrameDescriptorsScene = nullptr;

	struct VertexReserveInfo
	{
		SceneVertex* vptr;
		uint32_t* iptr;
		uint32_t vpos;
	};

	VertexReserveInfo ReserveVertices(size_t vcount, size_t icount)
	{
		// If buffers are full, flush and wait for room.
		if (SceneVertexPos + vcount > (size_t)BufferManager::SceneVertexBufferSize || SceneIndexPos + icount > (size_t)BufferManager::SceneIndexBufferSize)
		{
			// If the request is larger than our buffers we can't draw this.
			if (vcount > (size_t)BufferManager::SceneVertexBufferSize || icount > (size_t)BufferManager::SceneIndexBufferSize)
				return { nullptr, nullptr, 0 };

			FlushDrawBatchAndWait();
		}

		return { Buffers->SceneVertices + SceneVertexPos, Buffers->SceneIndexes + SceneIndexPos, (uint32_t)SceneVertexPos };
	}

	void FlushDrawBatchAndWait();

	void UseVertices(size_t vcount, size_t icount)
	{
		SceneVertexPos += vcount;
		SceneIndexPos += icount;
	}

	VkViewport viewportdesc = {};

	bool UsePrecache = true;
	vec4 FlashScale;
	vec4 FlashFog;
	FSceneNode* CurrentFrame = nullptr;
	float Aspect = 0.0f;
	float RProjZ = 0.0f;
	float RFX2 = 0.0f;
	float RFY2 = 0.0f;

	bool IsLocked = false;

	void SetPipeline(PipelineState* pipeline);
	ivec4 GetTextureIndexes(uint32_t PolyFlags, CachedTexture* tex, bool clamp = false);
	ivec4 GetTextureIndexes(uint32_t PolyFlags, CachedTexture* tex, CachedTexture* lightmap, CachedTexture* macrotex, CachedTexture* detailtex);
	void DrawBatch(VulkanCommandBuffer* cmdbuffer);
	void SubmitAndWait(bool present, int presentWidth, int presentHeight, bool presentFullscreen);

	vec4 ApplyInverseGamma(vec4 color);

	struct
	{
		size_t SceneIndexStart = 0;
		PipelineState* Pipeline = nullptr;
		float BlendConstants[4] = {};
	} Batch;

	ScenePushConstants pushconstants = {};

	size_t SceneVertexPos = 0;
	size_t SceneIndexPos = 0;

	struct HitQuery
	{
		int Start = 0;
		int Count = 0;
	};

	uint8_t* HitData = nullptr;
	int* HitSize = nullptr;
	std::vector<uint8_t> HitQueryStack;
	std::vector<HitQuery> HitQueries;
	std::vector<uint8_t> HitBuffer;

	int ForceHitIndex = -1;
	HitQuery ForceHit;
};

inline void VulkanRenderDevice::SetPipeline(PipelineState* pipeline)
{
	if (pipeline != Batch.Pipeline)
	{
		DrawBatch(Commands->GetDrawCommands());
		Batch.Pipeline = pipeline;
	}
}

inline ivec4 VulkanRenderDevice::GetTextureIndexes(uint32_t PolyFlags, CachedTexture* tex, bool clamp)
{
	return ivec4(DescriptorSets->GetTextureArrayIndex(PolyFlags, tex, clamp), 0, 0, 0);
}

inline ivec4 VulkanRenderDevice::GetTextureIndexes(uint32_t PolyFlags, CachedTexture* tex, CachedTexture* lightmap, CachedTexture* macrotex, CachedTexture* detailtex)
{
	if (DescriptorSets->IsTextureArrayFull())
	{
		FlushDrawBatchAndWait();
		DescriptorSets->ClearCache();
		Textures->ClearAllBindlessIndexes();
	}

	ivec4 textureBinds;
	textureBinds.x = DescriptorSets->GetTextureArrayIndex(PolyFlags, tex);
	textureBinds.y = DescriptorSets->GetTextureArrayIndex(0, macrotex);
	textureBinds.z = DescriptorSets->GetTextureArrayIndex(0, detailtex);
	textureBinds.w = DescriptorSets->GetTextureArrayIndex(0, lightmap);
	return textureBinds;
}
