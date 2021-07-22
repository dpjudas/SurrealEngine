#pragma once

#include "Math/vec.h"
#include "RenderDevice/RenderDevice.h"

class UFont;
class UTexture;
struct FSceneNode;

class CanvasRender
{
public:
	CanvasRender();

	void DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags);
	void DrawTileClipped(UTexture* Tex, float orgX, float orgY, float curX, float curY, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags, float clipX, float clipY);
	void DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t flags);
	void DrawTextClipped(UFont* font, vec4 color, float orgX, float orgY, float curX, float curY, const std::string& text, uint32_t flags, bool checkHotKey, float clipX, float clipY);
	ivec2 GetTextSize(UFont* font, const std::string& text);
	ivec2 GetTextClippedSize(UFont* font, const std::string& text, float clipX);

	void DrawTile(RenderDevice* device, FTextureInfo& texinfo, const Rectf& dest, const Rectf& src, const Rectf& clipBox, float Z, vec4 color, vec4 fog, uint32_t flags, int uiscale);

	UFont* bigfont = nullptr;
	UFont* largefont = nullptr;
	UFont* medfont = nullptr;
	UFont* smallfont = nullptr;

	FSceneNode SceneFrame;
};
