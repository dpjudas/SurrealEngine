#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

class Window;
class UTexture;
class UActor;

struct FCoords
{
	vec3 Origin;
	vec3 XAxis;
	vec3 YAxis;
	vec3 ZAxis;
};

struct FSceneNode
{
	int XB, YB; // viewport top left
	int X, Y; // viewport size
	float FX, FY;
	float FX2, FY2;
	Window* Viewport = nullptr;
	float FovAngle;

	mat4 Modelview;
	mat4 Projection;
	vec3 ViewLocation;
};

struct GouraudVertex
{
	vec3 Point;
	vec3 Light;
	vec2 UV;
};

struct FSurfaceFacet
{
	FCoords MapCoords;
	std::vector<vec3> Points;
};

struct FColor
{
	FColor() = default;
	FColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) { }

	uint8_t R, G, B, A;
};

struct FTextureInfo
{
	uint64_t CacheID = 0;
	bool bRealtimeChanged = false;

	UTexture* Texture = nullptr;
	float UScale = 1.0f;
	float VScale = 1.0f;
	vec2 Pan = { 0.0f };
};

struct FSurfaceInfo
{
	uint32_t PolyFlags = 0;
	FTextureInfo* Texture = nullptr;
	FTextureInfo* LightMap = nullptr;
	FTextureInfo* MacroTexture = nullptr;
	FTextureInfo* DetailTexture = nullptr;
	FTextureInfo* FogMap = nullptr;
};

class RenderDevice
{
public:
	static std::unique_ptr<RenderDevice> Create(::Window* viewport);

	virtual ~RenderDevice() = default;

	virtual void Flush(bool AllowPrecache) = 0;
	virtual void BeginFrame() = 0;
	virtual void BeginShadowmapUpdate() = 0;
	virtual void BeginShadowmapPass() = 0;
	virtual void EndShadowmapPass(int slot) = 0;
	virtual void EndShadowmapUpdate() = 0;
	virtual void BeginScenePass() = 0;
	virtual void EndScenePass() = 0;
	virtual void EndFrame(bool Blit) = 0;
	virtual void UpdateLights(const std::vector<std::pair<int, UActor*>>& LightUpdates) = 0;
	virtual void UpdateSurfaceLights(const std::vector<int32_t>& SurfaceLights) = 0;
	virtual void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) = 0;
	virtual void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo* Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) = 0;
	virtual void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) = 0;
	virtual void ClearZ(FSceneNode* Frame) = 0;
	virtual void ReadPixels(FColor* Pixels) = 0;
	virtual void EndFlash(float FlashScale, vec4 FlashFog) = 0;
	virtual void SetSceneNode(FSceneNode* Frame) = 0;
	virtual void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) = 0;

	Window* Viewport = nullptr;
	bool PrecacheOnFlip = false;
};
