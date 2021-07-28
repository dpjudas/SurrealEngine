#pragma once

#include "Math/vec.h"
#include "RenderDevice/RenderDevice.h"

class UActor;
class UFont;
class UTexture;
struct FSceneNode;

class CanvasRender
{
public:
	CanvasRender();

	void DrawActor(UActor* actor, bool WireFrame, bool ClearZ);
	void DrawClippedActor(UActor* actor, bool WireFrame, int X, int Y, int XB, int YB, bool ClearZ);
	void DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags);
	void DrawTileClipped(UTexture* Tex, float orgX, float orgY, float curX, float curY, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags, float clipX, float clipY);
	void DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t flags, bool center);
	void DrawTextClipped(UFont* font, vec4 color, float orgX, float orgY, float curX, float curY, const std::string& text, uint32_t flags, bool checkHotKey, float clipX, float clipY, bool center);
	ivec2 GetTextSize(UFont* font, const std::string& text);
	ivec2 GetTextClippedSize(UFont* font, const std::string& text, float clipX);

	void DrawTile(RenderDevice* device, FTextureInfo& texinfo, const Rectf& dest, const Rectf& src, const Rectf& clipBox, float Z, vec4 color, vec4 fog, uint32_t flags, int uiscale);

	UFont* bigfont = nullptr;
	UFont* largefont = nullptr;
	UFont* medfont = nullptr;
	UFont* smallfont = nullptr;

	FSceneNode SceneFrame;
};
