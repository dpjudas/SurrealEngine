#pragma once

#include "RenderDevice/RenderDevice.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"

#include "Math/vec.h"
#include "Math/mat.h"
#include "Utils/ComPtr.h"
#include <set>
#include <string>

#include "D3D11TextureManager.h"
#include "D3D11UploadManager.h"
#include "D3D11CachedTexture.h"

#include <D3D11.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <dxgi1_5.h>

struct D3D11SceneVertex
{
	uint32_t Flags;
	vec3 Position;
	vec2 TexCoord;
	vec2 TexCoord2;
	vec2 TexCoord3;
	vec2 TexCoord4;
	vec4 Color;
};

struct D3D11ScenePushConstants
{
	mat4 ObjectToProjection;
	mat4 ObjectToView;
	vec4 NearClip;
	int HitIndex;
	int Padding1, Padding2, Padding3;
};

struct D3D11PresentPushConstants
{
	float Contrast;
	float Saturation;
	float Brightness;
	float HdrScale;
	vec4 GammaCorrection;
};

struct D3D11BloomPushConstants
{
	float SampleWeights[8];
};

class D3D11RenderDevice : public RenderDevice
{
public:
	D3D11RenderDevice(Widget* viewport);
	~D3D11RenderDevice();

	// To do: port this
	bool Init(int NewX, int NewY, bool Fullscreen);
	bool SetRes(int NewX, int NewY, bool Fullscreen);
	void Exit();

	void Flush(bool AllowPrecache) override;
	void Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear, uint8_t* HitData, int* HitSize) override;
	void Unlock(bool Blit) override;
	void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) override;
	void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) override;
	void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) override;
	void Draw3DLine(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 OrigP, vec3 OrigQ) override;
	void Draw2DLine(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2) override;
	void Draw2DPoint(FSceneNode* Frame, vec4 Color, uint32_t LineFlags, float X1, float Y1, float X2, float Y2, float Z) override;
	void ClearZ(FSceneNode* Frame) override;
	void PushHit(const uint8_t* Data, int Count) override;
	void PopHit(int Count, bool bForce) override;
	void ReadPixels(FColor* Pixels) override;
	void EndFlash() override;
	void SetSceneNode(FSceneNode* Frame) override;
	void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) override;
	bool SupportsTextureFormat(TextureFormat Format) override;
	void UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL) override;

	void SetHitLocation();

	void SetDebugName(ID3D11Device* obj, const char* name);
	void SetDebugName(ID3D11DeviceChild* obj, const char* name);

	HWND WindowHandle = 0;
	ComPtr<ID3D11Device> Device;
	D3D_FEATURE_LEVEL FeatureLevel = {};
	ComPtr<ID3D11DeviceContext> Context;
	ComPtr<IDXGISwapChain> SwapChain;
	ComPtr<IDXGISwapChain1> SwapChain1;
	ComPtr<ID3D11Debug> DebugLayer;
	ComPtr<ID3D11InfoQueue> InfoQueue;
	ComPtr<ID3D11Texture2D> BackBuffer;
	ComPtr<ID3D11RenderTargetView> BackBufferView;
	bool DxgiSwapChainAllowTearing = false;
	int BufferCount = 2;
	std::set<std::string> SeenDebugMessages;
	int TotalSeenDebugMessages = 0;

	struct PPBlurLevel
	{
		ComPtr<ID3D11Texture2D> VTexture;
		ComPtr<ID3D11RenderTargetView> VTextureRTV;
		ComPtr<ID3D11ShaderResourceView> VTextureSRV;
		ComPtr<ID3D11Texture2D> HTexture;
		ComPtr<ID3D11RenderTargetView> HTextureRTV;
		ComPtr<ID3D11ShaderResourceView> HTextureSRV;
		int Width = 0;
		int Height = 0;
	};

	struct
	{
		ComPtr<ID3D11Texture2D> ColorBuffer;
		ComPtr<ID3D11Texture2D> HitBuffer;
		ComPtr<ID3D11Texture2D> DepthBuffer;
		ComPtr<ID3D11Texture2D> PPImage[2];
		ComPtr<ID3D11Texture2D> PPHitBuffer;
		ComPtr<ID3D11Texture2D> StagingHitBuffer;
		ComPtr<ID3D11RenderTargetView> ColorBufferView;
		ComPtr<ID3D11RenderTargetView> HitBufferView;
		ComPtr<ID3D11DepthStencilView> DepthBufferView;
		ComPtr<ID3D11RenderTargetView> PPHitBufferView;
		ComPtr<ID3D11RenderTargetView> PPImageView[2];
		ComPtr<ID3D11ShaderResourceView> HitBufferShaderView;
		ComPtr<ID3D11ShaderResourceView> PPImageShaderView[2];
		enum { NumBloomLevels = 4 };
		PPBlurLevel BlurLevels[NumBloomLevels];
		int Width = 0;
		int Height = 0;
		int Multisample = 0;
	} SceneBuffers;

	struct ScenePipelineState
	{
		D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		ID3D11PixelShader* PixelShader = nullptr;
		ComPtr<ID3D11BlendState> BlendState;
		ComPtr<ID3D11DepthStencilState> DepthStencilState;
		float MinDepth = 0.1f;
		float MaxDepth = 1.0f;
	};

	struct
	{
		ComPtr<ID3D11VertexShader> VertexShader;
		ComPtr<ID3D11InputLayout> InputLayout;
		ComPtr<ID3D11Buffer> VertexBuffer;
		ComPtr<ID3D11Buffer> IndexBuffer;
		ComPtr<ID3D11Buffer> ConstantBuffer;
		ComPtr<ID3D11RasterizerState> RasterizerState[2];
		ComPtr<ID3D11PixelShader> PixelShader;
		ComPtr<ID3D11PixelShader> PixelShaderAlphaTest;
		ComPtr<ID3D11SamplerState> Samplers[16];
		ScenePipelineState Pipelines[33];
		ScenePipelineState LinePipeline[2];
		ScenePipelineState PointPipeline[2];
		float LODBias = 0.0f;
	} ScenePass;

	static const int D3D11SceneVertexBufferSize = 16 * 1024;
	static const int SceneIndexBufferSize = 32 * 1024;

	struct DrawBatchEntry
	{
		size_t SceneIndexStart = 0;
		size_t SceneIndexEnd = 0;
		ScenePipelineState* Pipeline = nullptr;
		D3D11CachedTexture* Tex = nullptr;
		D3D11CachedTexture* Lightmap = nullptr;
		D3D11CachedTexture* Detailtex = nullptr;
		D3D11CachedTexture* Macrotex = nullptr;
		uint32_t TexSamplerMode = 0;
		uint32_t DetailtexSamplerMode = 0;
		uint32_t MacrotexSamplerMode = 0;
		float BlendConstants[4] = {};
	} Batch;
	std::vector<DrawBatchEntry> QueuedBatches;
	D3D11CachedTexture* nulltex = nullptr;

	D3D11SceneVertex* SceneVertices = nullptr;
	size_t D3D11SceneVertexPos = 0;

	uint32_t* SceneIndexes = nullptr;
	size_t SceneIndexPos = 0;

	struct
	{
		ComPtr<ID3D11VertexShader> PPStep;
		ComPtr<ID3D11InputLayout> PPStepLayout;
		ComPtr<ID3D11Buffer> PPStepVertexBuffer;
		ComPtr<ID3D11PixelShader> HitResolve;
		ComPtr<ID3D11PixelShader> Present[16];
		ComPtr<ID3D11Buffer> PresentConstantBuffer;
		ComPtr<ID3D11Texture2D> DitherTexture;
		ComPtr<ID3D11ShaderResourceView> DitherTextureView;
		ComPtr<ID3D11BlendState> BlendState;
		ComPtr<ID3D11DepthStencilState> DepthStencilState;
		ComPtr<ID3D11RasterizerState> RasterizerState;
	} PresentPass;

	struct
	{
		ComPtr<ID3D11PixelShader> Extract;
		ComPtr<ID3D11PixelShader> Combine;
		ComPtr<ID3D11PixelShader> BlurVertical;
		ComPtr<ID3D11PixelShader> BlurHorizontal;
		ComPtr<ID3D11Buffer> ConstantBuffer;
		ComPtr<ID3D11BlendState> AdditiveBlendState;
	} BloomPass;

	std::unique_ptr<D3D11TextureManager> Textures;
	std::unique_ptr<D3D11UploadManager> Uploads;

	int RefreshRate = 0;

	struct
	{
		int ComplexSurfaces = 0;
		int GouraudPolygons = 0;
		int Tiles = 0;
		int DrawCalls = 0;
		int Uploads = 0;
		int RectUploads = 0;
		int BuffersUsed = 0;
	} Stats;

private:
	struct ComplexSurfaceInfo
	{
		FSurfaceFacet* facet;
		D3D11CachedTexture* tex;
		D3D11CachedTexture* lightmap;
		D3D11CachedTexture* macrotex;
		D3D11CachedTexture* detailtex;
		D3D11CachedTexture* fogmap;
		vec4* editorcolor;
	};
	void DrawComplexSurfaceFaces(const ComplexSurfaceInfo& info);

	void ReleaseSwapChainResources();
	bool UpdateSwapChain();
	void SetColorSpace();
	void ResizeSceneBuffers(int width, int height, int multisample);
	void ClearTextureCache();

	void CreatePresentPass();
	void CreateBloomPass();
	void CreateScenePass();
	void ReleaseScenePass();
	void ReleaseBloomPass();
	void ReleasePresentPass();

	void CreateSceneSamplers();
	void ReleaseSceneSamplers();
	void UpdateLODBias();

	void ReleaseSceneBuffers();

	void RunBloomPass();
	void BlurStep(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output, bool vertical);
	float ComputeBlurGaussian(float n, float theta);
	void ComputeBlurSamples(int sampleCount, float blurAmount, float* sampleWeights);

	void SetPipeline(ScenePipelineState* pipeline);
	void SetPipeline(uint32_t polyflags);
	void SetDescriptorSet(uint32_t polyflags, D3D11CachedTexture* tex = nullptr, bool clamp = false);
	void SetDescriptorSet(uint32_t polyflags, const ComplexSurfaceInfo& info);

	void AddDrawBatch();
	void DrawBatches(bool nextBuffer = false);
	void DrawEntry(const DrawBatchEntry& entry);

	void PrintDebugLayerMessages();

	HWND GetWindowHandle();

	struct VertexReserveInfo
	{
		D3D11SceneVertex* vptr;
		uint32_t* iptr;
		uint32_t vpos;
	};

	VertexReserveInfo ReserveVertices(size_t vcount, size_t icount)
	{
		if (!SceneVertices || !SceneIndexes)
			return { nullptr, nullptr, 0 };

		// If buffers are full, flush and wait for room.
		if (D3D11SceneVertexPos + vcount > (size_t)D3D11SceneVertexBufferSize || SceneIndexPos + icount > (size_t)SceneIndexBufferSize)
		{
			// If the request is larger than our buffers we can't draw this.
			if (vcount > (size_t)D3D11SceneVertexBufferSize || icount > (size_t)SceneIndexBufferSize)
				return { nullptr, nullptr, 0 };

			DrawBatches(true);
		}

		return { SceneVertices + D3D11SceneVertexPos, SceneIndexes + SceneIndexPos, (uint32_t)D3D11SceneVertexPos };
	}

	void UseVertices(size_t vcount, size_t icount)
	{
		D3D11SceneVertexPos += vcount;
		SceneIndexPos += icount;
	}

	int GetSettingsMultisample();

	ScenePipelineState* GetPipeline(uint32_t PolyFlags);

	void CreateVertexShader(ComPtr<ID3D11VertexShader>& outShader, const std::string& shaderName, ComPtr<ID3D11InputLayout>& outInputLayout, const std::string& inputLayoutName, const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, const std::string& filename, const std::vector<std::string> defines = {});
	void CreatePixelShader(ComPtr<ID3D11PixelShader>& outShader, const std::string& shaderName, const std::string& filename, const std::vector<std::string> defines = {});
	std::vector<uint8_t> CompileHlsl(const std::string& filename, const std::string& shadertype, const std::vector<std::string> defines = {});

	vec4 ApplyInverseGamma(vec4 color);

	D3D11PresentPushConstants GetD3D11PresentPushConstants();

	bool VerticesMapped() const { return SceneVertices && SceneIndexes; }
	void MapVertices(bool nextBuffer);
	void UnmapVertices();

	bool UsePrecache = true;
	vec4 FlashScale = vec4(0.0f);
	vec4 FlashFog = vec4(0.0f);
	FSceneNode* CurrentFrame = nullptr;
	float Aspect = 0.0f;
	float RProjZ = 0.0f;
	float RFX2 = 0.0f;
	float RFY2 = 0.0f;
	D3D11ScenePushConstants SceneConstants = {};
	D3D11_VIEWPORT SceneViewport = {};
	bool DepthCuedActive = false;

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

	bool IsLocked = false;
	bool ActiveHdr = false;

	struct
	{
		int Width = 0;
		int Height = 0;
	} DesktopResolution;

	bool CurrentFullscreen = 0;
	int CurrentSizeX = 0;
	int CurrentSizeY = 0;
};

inline void ThrowIfFailed(HRESULT result, const char* msg) { if (FAILED(result)) throw std::runtime_error(msg); }

inline void D3D11RenderDevice::SetPipeline(ScenePipelineState* pipeline)
{
	if (pipeline != Batch.Pipeline)
	{
		AddDrawBatch();
		Batch.Pipeline = pipeline;
	}
}

inline void D3D11RenderDevice::SetPipeline(uint32_t PolyFlags)
{
	auto pipeline = GetPipeline(PolyFlags);
	if (pipeline != Batch.Pipeline)
	{
		AddDrawBatch();
		Batch.Pipeline = pipeline;
	}
}

inline void D3D11RenderDevice::SetDescriptorSet(uint32_t PolyFlags, D3D11CachedTexture* tex, bool clamp)
{
	if (!tex) tex = nulltex;

	uint32_t samplermode = 0;
	if (PolyFlags & PF_NoSmooth) samplermode |= 1;
	if (clamp) samplermode |= 2;
	samplermode |= (tex->DummyMipmapCount << 2);

	if (Batch.Tex != tex || Batch.TexSamplerMode != samplermode || Batch.Lightmap != nulltex || Batch.Detailtex != nulltex || Batch.DetailtexSamplerMode != 0 || Batch.Macrotex != nulltex || Batch.MacrotexSamplerMode != 0)
	{
		AddDrawBatch();
		Batch.Tex = tex;
		Batch.Lightmap = nulltex;
		Batch.Detailtex = nulltex;
		Batch.Macrotex = nulltex;
		Batch.TexSamplerMode = samplermode;
		Batch.DetailtexSamplerMode = 0;
		Batch.MacrotexSamplerMode = 0;
	}
}

inline void D3D11RenderDevice::SetDescriptorSet(uint32_t PolyFlags, const ComplexSurfaceInfo& info)
{
	uint32_t samplermode = 0;
	if (PolyFlags & PF_NoSmooth) samplermode |= 1;
	samplermode |= (info.tex->DummyMipmapCount << 2);

	int detailsamplermode = info.detailtex->DummyMipmapCount << 2;
	int macrosamplermode = info.macrotex->DummyMipmapCount << 2;

	if (Batch.Tex != info.tex || Batch.TexSamplerMode != samplermode || Batch.Lightmap != info.lightmap || Batch.Detailtex != info.detailtex || Batch.DetailtexSamplerMode != detailsamplermode || Batch.Macrotex != info.macrotex || Batch.MacrotexSamplerMode != macrosamplermode)
	{
		AddDrawBatch();
		Batch.Tex = info.tex;
		Batch.Lightmap = info.lightmap;
		Batch.Detailtex = info.detailtex;
		Batch.Macrotex = info.macrotex;
		Batch.TexSamplerMode = samplermode;
		Batch.DetailtexSamplerMode = detailsamplermode;
		Batch.MacrotexSamplerMode = macrosamplermode;
	}
}
