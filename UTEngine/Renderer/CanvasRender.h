#pragma once

#include "Math/vec.h"
#include "RenderDevice/RenderDevice.h"

class UFont;
class UTexture;
struct FSceneNode;

enum class TextAlignment
{
	left,
	center,
	right
};

class CanvasRender
{
public:
	CanvasRender();

	void DrawFontTextWithShadow(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	void DrawFontText(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	ivec2 GetFontTextSize(UFont* font, const std::string& text);
	void DrawTile(FSceneNode* frame, UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags);

	UFont* bigfont = nullptr;
	UFont* largefont = nullptr;
	UFont* medfont = nullptr;
	UFont* smallfont = nullptr;

	FSceneNode SceneFrame;
};
