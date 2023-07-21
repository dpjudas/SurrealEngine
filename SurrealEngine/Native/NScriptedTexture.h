#pragma once

#include "UObject/UObject.h"

class NScriptedTexture
{
public:
	static void RegisterFunctions();

	static void DrawColoredText(UObject* Self, float X, float Y, const std::string& Text, UObject* Font, const Color& FontColor);
	static void DrawText(UObject* Self, float X, float Y, const std::string& Text, UObject* Font);
	static void DrawTile(UObject* Self, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UObject* Tex, bool bMasked);
	static void ReplaceTexture(UObject* Self, UObject* Tex);
	static void TextSize(UObject* Self, const std::string& Text, float& XL, float& YL, UObject* Font);
};
