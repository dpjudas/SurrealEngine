#pragma once

#include "RenderDevice/RenderDevice.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

#include "Math/vec.h"
#include "Math/mat.h"
#include <set>
#include <string>

#include "GLTextureManager.h"
#include "GLUploadManager.h"
#include "GLCachedTexture.h"

#include "GLHandles.h"

struct GLSceneVertex
{
	uint32_t Flags;
	vec3 Position;
	vec2 TexCoord;
	vec2 TexCoord2;
	vec2 TexCoord3;
	vec2 TexCoord4;
	vec4 Color;
};

struct GLScenePushConstants
{
	mat4 ObjectToProjection;
	mat4 ObjectToView;
	vec4 NearClip;
	int HitIndex;
	int Padding1, Padding2, Padding3;
};

struct GLPresentPushConstants
{
	float Contrast;
	float Saturation;
	float Brightness;
	float HdrScale;
	vec4 GammaCorrection;
};

struct GLBloomPushConstants
{
	float SampleWeights[8];
};

struct GLViewport
{
	float TopLeftX;
	float TopLeftY;
	float Width;
	float Height;
	float MinDepth;
	float MaxDepth;
};

class GLRenderDevice : public RenderDevice
{
public:
	GLRenderDevice(Widget* viewport);
	~GLRenderDevice();

	// To do: port this
	bool Init(int NewX, int NewY, bool Fullscreen);
	bool SetRes(int NewX, int NewY, bool Fullscreen);
	void Exit();

	void Flush(bool AllowPrecache) override;
	void Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear, uint8_t* HitData, int* HitSize) override;
	void Unlock(bool Blit) override;
	void DrawComplexSurface(SceneNode* Frame, SurfaceInfo& Surface, SurfaceFacet& Facet) override;
	void DrawGouraudPolygon(SceneNode* Frame, TextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) override;
	void DrawTile(SceneNode* Frame, TextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) override;
	void Draw3DLine(SceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 OrigP, vec3 OrigQ) override;
	void Draw2DLine(SceneNode* Frame, vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2) override;
	void Draw2DPoint(SceneNode* Frame, vec4 Color, uint32_t LineFlags, float X1, float Y1, float X2, float Y2, float Z) override;
	void ClearZ() override;
	void PushHit(const uint8_t* Data, int Count) override;
	void PopHit(int Count, bool bForce) override;
	void ReadPixels(TextureColor* Pixels) override;
	void EndFlash() override;
	void SetSceneNode(SceneNode* Frame) override;
	void PrecacheTexture(TextureInfo& Info, uint32_t PolyFlags) override;
	bool SupportsTextureFormat(TextureFormat Format) override;
	void UpdateTextureRect(TextureInfo& Info, int U, int V, int UL, int VL) override;

	void SetHitLocation();

	void SetDebugName(GLenum type, GLuint handle, const char* name);

	template<typename T> void SetDebugName(const T& object, const char* name)
	{
		if (object)
			SetDebugName(object->Type, object->Handle, name);
	}

	int BufferCount = 2;

	struct PPBlurLevel
	{
		std::shared_ptr<GLTexture2D> VTexture;
		std::shared_ptr<GLRenderTargetView> VTextureRTV;
		std::shared_ptr<GLShaderResourceView> VTextureSRV;
		std::shared_ptr<GLTexture2D> HTexture;
		std::shared_ptr<GLRenderTargetView> HTextureRTV;
		std::shared_ptr<GLShaderResourceView> HTextureSRV;
		int Width = 0;
		int Height = 0;
	};

	struct
	{
		std::shared_ptr<GLTexture2D> ColorBuffer;
		std::shared_ptr<GLTexture2D> HitBuffer;
		std::shared_ptr<GLTexture2D> DepthBuffer;
		std::shared_ptr<GLTexture2D> PPImage[2];
		std::shared_ptr<GLTexture2D> PPHitBuffer;
		std::shared_ptr<GLTexture2D> StagingHitBuffer;
		std::shared_ptr<GLRenderTargetView> ColorBufferView;
		std::shared_ptr<GLRenderTargetView> HitBufferView;
		std::shared_ptr<GLDepthStencilView> DepthBufferView;
		std::shared_ptr<GLRenderTargetView> PPHitBufferView;
		std::shared_ptr<GLRenderTargetView> PPImageView[2];
		std::shared_ptr<GLShaderResourceView> HitBufferShaderView;
		std::shared_ptr<GLShaderResourceView> PPImageShaderView[2];
		enum { NumBloomLevels = 4 };
		PPBlurLevel BlurLevels[NumBloomLevels];
		int Width = 0;
		int Height = 0;
		int Multisample = 0;
	} SceneBuffers;

	struct ScenePipelineState
	{
		GLenum PrimitiveTopology = GL_TRIANGLES;
		GLProgram* ShaderProgram = nullptr;
		std::shared_ptr<GLBlendState> BlendState;
		std::shared_ptr<GLDepthStencilState> DepthStencilState;
		float MinDepth = 0.1f;
		float MaxDepth = 1.0f;
	};

	struct
	{
		std::shared_ptr<GLVertexShader> VertexShader;
		std::shared_ptr<GLInputLayout> InputLayout;
		std::shared_ptr<GLBuffer> VertexBuffer;
		std::shared_ptr<GLBuffer> IndexBuffer;
		std::shared_ptr<GLBuffer> ConstantBuffer;
		std::shared_ptr<GLRasterizerState> RasterizerState[2];
		std::shared_ptr<GLFragmentShader> FragmentShader;
		std::shared_ptr<GLFragmentShader> FragmentShaderAlphaTest;
		std::shared_ptr<GLProgram> ShaderProgram;
		std::shared_ptr<GLProgram> ShaderProgramAlphaTest;
		std::shared_ptr<GLSampler> Samplers[16];
		ScenePipelineState Pipelines[33];
		ScenePipelineState LinePipeline[2];
		ScenePipelineState PointPipeline[2];
		float LODBias = 0.0f;
	} ScenePass;

	static const int SceneVertexBufferSize = 16 * 1024;
	static const int SceneIndexBufferSize = 32 * 1024;

	struct DrawBatchEntry
	{
		size_t SceneIndexStart = 0;
		size_t SceneIndexEnd = 0;
		ScenePipelineState* Pipeline = nullptr;
		GLCachedTexture* Tex = nullptr;
		GLCachedTexture* Lightmap = nullptr;
		GLCachedTexture* Detailtex = nullptr;
		GLCachedTexture* Macrotex = nullptr;
		uint32_t TexSamplerMode = 0;
		uint32_t DetailtexSamplerMode = 0;
		uint32_t MacrotexSamplerMode = 0;
		float BlendConstants[4] = {};
	} Batch;
	std::vector<DrawBatchEntry> QueuedBatches;
	GLCachedTexture* nulltex = nullptr;

	GLSceneVertex* SceneVertices = nullptr;
	size_t GLSceneVertexPos = 0;

	uint32_t* SceneIndexes = nullptr;
	size_t SceneIndexPos = 0;

	struct
	{
		std::shared_ptr<GLVertexShader> PPStep;
		std::shared_ptr<GLInputLayout> PPStepLayout;
		std::shared_ptr<GLBuffer> PPStepVertexBuffer;
		std::shared_ptr<GLFragmentShader> HitResolve;
		std::shared_ptr<GLFragmentShader> Present[16];
		std::shared_ptr<GLProgram> HitResolveProgram;
		std::shared_ptr<GLProgram> PresentProgram[16];
		std::shared_ptr<GLBuffer> PresentConstantBuffer;
		std::shared_ptr<GLTexture2D> DitherTexture;
		std::shared_ptr<GLShaderResourceView> DitherTextureView;
		std::shared_ptr<GLBlendState> BlendState;
		std::shared_ptr<GLDepthStencilState> DepthStencilState;
		std::shared_ptr<GLRasterizerState> RasterizerState;
	} PresentPass;

	struct
	{
		std::shared_ptr<GLFragmentShader> Extract;
		std::shared_ptr<GLFragmentShader> Combine;
		std::shared_ptr<GLFragmentShader> BlurVertical;
		std::shared_ptr<GLFragmentShader> BlurHorizontal;
		std::shared_ptr<GLProgram> ExtractProgram;
		std::shared_ptr<GLProgram> CombineProgram;
		std::shared_ptr<GLProgram> BlurVerticalProgram;
		std::shared_ptr<GLProgram> BlurHorizontalProgram;
		std::shared_ptr<GLBuffer> ConstantBuffer;
		std::shared_ptr<GLBlendState> AdditiveBlendState;
	} BloomPass;

	std::unique_ptr<GLTextureManager> Textures;
	std::unique_ptr<GLUploadManager> Uploads;

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
		SurfaceFacet* facet;
		GLCachedTexture* tex;
		GLCachedTexture* lightmap;
		GLCachedTexture* macrotex;
		GLCachedTexture* detailtex;
		GLCachedTexture* fogmap;
		vec4* editorcolor;
	};
	void DrawComplexSurfaceFaces(const ComplexSurfaceInfo& info);

	void ReleaseSwapChainResources();
	bool UpdateSwapChain();
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
	void BlurStep(GLShaderResourceView* input, GLRenderTargetView* output, bool vertical);
	float ComputeBlurGaussian(float n, float theta);
	void ComputeBlurSamples(int sampleCount, float blurAmount, float* sampleWeights);

	void SetPipeline(ScenePipelineState* pipeline);
	void SetPipeline(uint32_t polyflags);
	void SetDescriptorSet(uint32_t polyflags, GLCachedTexture* tex = nullptr, bool clamp = false);
	void SetDescriptorSet(uint32_t polyflags, const ComplexSurfaceInfo& info);

	void AddDrawBatch();
	void DrawBatches(bool nextBuffer = false);
	void DrawEntry(const DrawBatchEntry& entry);

	struct VertexReserveInfo
	{
		GLSceneVertex* vptr;
		uint32_t* iptr;
		uint32_t vpos;
	};

	VertexReserveInfo ReserveVertices(size_t vcount, size_t icount)
	{
		if (!SceneVertices || !SceneIndexes)
			return { nullptr, nullptr, 0 };

		// If buffers are full, flush and wait for room.
		if (GLSceneVertexPos + vcount > (size_t)SceneVertexBufferSize || SceneIndexPos + icount > (size_t)SceneIndexBufferSize)
		{
			// If the request is larger than our buffers we can't draw this.
			if (vcount > (size_t)SceneVertexBufferSize || icount > (size_t)SceneIndexBufferSize)
				return { nullptr, nullptr, 0 };

			DrawBatches(true);
		}

		return { SceneVertices + GLSceneVertexPos, SceneIndexes + SceneIndexPos, (uint32_t)GLSceneVertexPos };
	}

	void UseVertices(size_t vcount, size_t icount)
	{
		GLSceneVertexPos += vcount;
		SceneIndexPos += icount;
	}

	int GetSettingsMultisample();

	ScenePipelineState* GetPipeline(uint32_t PolyFlags);

	std::shared_ptr<GLBlendState> CreateBlendState(const GLBlendDesc& desc);
	std::shared_ptr<GLRasterizerState> CreateRasterizerState(const GLRasterizerDesc& desc);
	std::shared_ptr<GLDepthStencilState> CreateDepthStencilState(const GLDepthStencilDesc& desc);

	std::shared_ptr<GLBuffer> CreateBuffer(GLenum target, GLenum usage, const void* data, size_t size, const char* debugName);

	std::shared_ptr<GLProgram> CreateProgram(const std::string& programName, std::shared_ptr<GLVertexShader> vertexShader, std::shared_ptr<GLFragmentShader> fragmentShader);
	std::shared_ptr<GLVertexShader> CreateVertexShader(const std::string& shaderName, const std::string& filename, const std::vector<std::string> defines = {});
	std::shared_ptr<GLFragmentShader> CreateFragmentShader(const std::string& shaderName, const std::string& filename, const std::vector<std::string> defines = {});
	void CompileGlsl(GLShader* shader, const std::string& filename, const std::vector<std::string> defines = {});

	vec4 ApplyInverseGamma(vec4 color);

	GLPresentPushConstants GetGLPresentPushConstants();

	bool VerticesMapped() const { return SceneVertices && SceneIndexes; }
	void MapVertices(bool nextBuffer);
	void UnmapVertices();

	void SetViewport(const GLViewport& viewport);
	void SetBufferData(GLenum target, GLenum usage, GLBuffer* buffer, const void* data, size_t size);
	void SetBlendState(GLBlendState* blendState, const float* BlendConstants = nullptr);
	void SetDepthStencilState(GLDepthStencilState* depthStencilState);
	void SetRasterizerState(GLRasterizerState* rasterizerState);
	void SetUniformBuffers(int start, int count, GLBuffer** buffers);
	void SetTextures(int start, int count, GLTexture** textures);
	void SetSamplers(int start, int count, GLSampler** samplers);

	bool UsePrecache = true;
	vec4 FlashScale = vec4(0.0f);
	vec4 FlashFog = vec4(0.0f);
	SceneNode* CurrentFrame = nullptr;
	float Aspect = 0.0f;
	float RProjZ = 0.0f;
	float RFX2 = 0.0f;
	float RFY2 = 0.0f;
	GLScenePushConstants SceneConstants = {};
	GLViewport SceneViewport = {};
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

	bool CurrentFullscreen = 0;
	int CurrentSizeX = 0;
	int CurrentSizeY = 0;
};

inline void GLRenderDevice::SetPipeline(ScenePipelineState* pipeline)
{
	if (pipeline != Batch.Pipeline)
	{
		AddDrawBatch();
		Batch.Pipeline = pipeline;
	}
}

inline void GLRenderDevice::SetPipeline(uint32_t PolyFlags)
{
	auto pipeline = GetPipeline(PolyFlags);
	if (pipeline != Batch.Pipeline)
	{
		AddDrawBatch();
		Batch.Pipeline = pipeline;
	}
}

inline void GLRenderDevice::SetDescriptorSet(uint32_t PolyFlags, GLCachedTexture* tex, bool clamp)
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

inline void GLRenderDevice::SetDescriptorSet(uint32_t PolyFlags, const ComplexSurfaceInfo& info)
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

inline void ThrowIfGLError(const char* msg)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		throw std::runtime_error(msg);
}
