#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

#include "UObject/UTexture.h"

class DisplayWindow;
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
	DisplayWindow* Viewport = nullptr;
	float FovAngle;

	mat4 ObjectToWorld;
	mat4 WorldToView;
	mat4 Projection;
	vec3 ViewLocation;
};

struct GouraudVertex
{
	vec3 Point;
	vec3 Light;
	vec2 UV;
	vec4 Fog;
};

struct FSurfaceFacet
{
	FCoords MapCoords;
	std::vector<std::vector<vec3>> Polys;
};

struct FColor
{
	FColor() = default;
	FColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) { }

	uint8_t R, G, B, A;
};

enum class TextureFormat : uint32_t;

struct FTextureInfo
{
	uint64_t CacheID = 0;
	bool bRealtimeChanged = false;

	UTexture* Texture = nullptr;
	float UScale = 1.0f;
	float VScale = 1.0f;
	vec2 Pan = { 0.0f };

	// to do: give these correct values
	TextureFormat Format = {};
	int USize = 1;
	int VSize = 1;
	int NumMips = 0;
	UnrealMipmap* Mips = nullptr;
	FColor* Palette = nullptr;
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

class OutputDevice
{
public:
	void Log(const std::string& text) { }
};

class RenderDevice
{
public:
	static std::unique_ptr<RenderDevice> Create(DisplayWindow* viewport);

	virtual ~RenderDevice() = default;

	virtual void Flush(bool AllowPrecache) = 0;
	virtual bool Exec(std::string Cmd, OutputDevice& Ar) { return false; }
	virtual void Lock(vec4 FlashScale, vec4 FlashFog, vec4 ScreenClear) = 0;
	virtual void Unlock(bool Blit) = 0;
	virtual void DrawComplexSurface(FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet) = 0;
	virtual void DrawGouraudPolygon(FSceneNode* Frame, FTextureInfo& Info, const GouraudVertex* Pts, int NumPts, uint32_t PolyFlags) = 0;
	virtual void DrawTile(FSceneNode* Frame, FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags) = 0;
	virtual void ClearZ(FSceneNode* Frame) = 0;
	virtual void ReadPixels(FColor* Pixels) = 0;
	virtual void EndFlash() = 0;
	virtual void SetSceneNode(FSceneNode* Frame) = 0;
	virtual void PrecacheTexture(FTextureInfo& Info, uint32_t PolyFlags) = 0;
	virtual bool SupportsTextureFormat(TextureFormat Format) = 0;
	virtual void UpdateTextureRect(FTextureInfo& Info, int U, int V, int UL, int VL) = 0;

	bool ParseCommand(std::string* cmd, const std::string& keyword) { return false; }

	DisplayWindow* Viewport = nullptr;
	bool PrecacheOnFlip = false;
};
