#pragma once

#include "UObject/UObject.h"

class NCanvas
{
public:
	static void RegisterFunctions();

	static void DrawActor(UObject* Self, UObject* A, bool WireFrame, bool* ClearZ);
	static void DrawClippedActor(UObject* Self, UObject* A, bool WireFrame, int X, int Y, int XB, int YB, bool* ClearZ);
	static void DrawPortal(UObject* Self, int X, int Y, int Width, int Height, UObject* CamActor, const vec3& CamLocation, const Rotator& CamRotation, int* FOV, bool* ClearZ);
	static void DrawText(UObject* Self, const std::string& Text, bool* CR);
	static void DrawTextClipped(UObject* Self, const std::string& Text, bool* bCheckHotKey);
	static void DrawTile(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL);
	static void DrawTileClipped(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL);
	static void StrLen(UObject* Self, const std::string& String, float& XL, float& YL);
	static void TextSize(UObject* Self, const std::string& String, float& XL, float& YL);
};
