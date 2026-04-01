#pragma once

#include "UObject/UObject.h"

class UTexture;
enum class ERenderZTest : uint8_t;

class NCanvas
{
public:
	static void RegisterFunctions();

	static void DrawActor(UObject* Self, UObject* A, bool WireFrame, std::optional<bool> ClearZ);
	static void DrawClippedActor(UObject* Self, UObject* A, bool WireFrame, int X, int Y, int XB, int YB, std::optional<bool> ClearZ);
	static void DrawPortal(UObject* Self, int X, int Y, int Width, int Height, UObject* CamActor, const vec3& CamLocation, const Rotator& CamRotation, std::optional<int> FOV, std::optional<bool> ClearZ);
	static void DrawText(UObject* Self, const std::string& Text, std::optional<bool> CR);
	static void DrawTextClipped(UObject* Self, const std::string& Text, std::optional<bool> bCheckHotKey);
	static void DrawTile(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL);
	static void DrawTileClipped(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL);
	static void StrLen(UObject* Self, const std::string& String, float& XL, float& YL);
	static void StrLen_219(UObject* Self, const std::string& Text, int NumChars, int StartIndex, int& XL, int& YL);
	static void TextSize(UObject* Self, const std::string& String, float& XL, float& YL);

	// 227 Additions
	static void Draw2DLine_U227(UObject* Self, Color& Col, vec3& Start, vec3& End);
	static void Draw3DLine_U227(UObject* Self, Color& Col, vec3& Start, vec3& End);
	static void WorldToScreen_U227(UObject* Self, vec3& WorldPos, std::optional<float> ZDistance, vec3& ReturnValue);
	static void ScreenToWorld_U227(UObject* Self, vec3& ScreenPos, vec3& ReturnValue);
	static void DrawPathNetwork_U227(UObject* Self, BitfieldBool& bOnlyWalkable, std::optional<float> MaxDistance); // Def MaxDistance = 2000
	static void DrawCircle_U227(UObject* Self, Color& Col, int LineFlags, vec3& WorldPosition, float Radius);
	static void DrawBox_U227(UObject* Self, Color& Col, int LineFlags, vec3& Start, vec3& End);
	static void GetCameraCoords_U227(UObject* Self, Coords& ReturnValue);
	static void SetTile3DOffset_U227(UObject* Self, BitfieldBool& bEnable, std::optional<vec3> Offset, std::optional<Rotator> RotOffset, std::optional<bool> bFlatZ, std::optional<float> Scale, std::optional<bool> bWorldOffset);
	// static void DrawTris(UObject* Self, UTexture*& Tex, Array<CanvasPoly> Polys, std::optional<int> PolyFlags, std::optional<bool> bFogColor);
	static void DrawRotatedTile_U227(UObject* Self, UObject*& Tex, float Roll, float XL, float YL, float U, float V, float UL, float VL, float PivotX, float PivotY, std::optional<bool> bRotateUV, std::optional<int> PolyFlags);
	static void PushClipPlane_U227(UObject* Self, vec3& Dir, float W, std::optional<bool> bLocal, BitfieldBool& ReturnValue);
	static void PopClipPlane_U227(UObject* Self);
	static void SetZTest_U227(UObject* Self, uint8_t& ZTest, uint8_t& ReturnValue); // Returns the old ZTest value
	static void PushCanvasScale_U227(UObject* Self, float Scale, std::optional<bool> bAbsolute);
	static void PopCanvasScale_U227(UObject* Self);
	static void SetCustomLighting_U227(UObject* Self, std::optional<bool> bDisableEngineLighting, std::optional<vec3> customAmbientLight);
	static void AddCustomLightSource_U227(UObject* Self, vec3& Pos, std::optional<Rotator> Dir, std::optional<bool> bDefault);
	static void ClearCustomLightSources_U227(UObject* Self);
	static void Reset_U227(UObject* Self);
};
