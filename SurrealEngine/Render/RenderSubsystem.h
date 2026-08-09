#pragma once

#include "VisibleFrame.h"

class RenderDevice;
class UWindow;
class UFont;
class UMover;
class UViewportWindow;

class RenderSubsystem
{
public:
	RenderSubsystem(RenderDevice* renderdevice);

	void PreRenderWindows(UCanvas* canvas);
	void PostRenderWindows(UCanvas* canvas);
	void DrawWindowInfo(UFont* font, UWindow* window, int depth, float& curY);
	void DrawWindow(UWindow* window, float offsetX, float offsetY);
	void ResetWindowGC(UWindow* window, float offsetX, float offsetY);

	void DrawEditorViewport();
	void DrawVideoFrame(FTextureInfo* frame, FTextureInfo* background);

	void DrawGame(float levelTimeElapsed);
	void OnMapLoaded();

	void DrawActor(UActor* actor, bool WireFrame, bool ClearZ);
	void DrawClippedActor(UActor* actor, bool WireFrame, int X, int Y, int XB, int YB, bool ClearZ);
	void DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags);
	void DrawTileClipped(UTexture* Tex, float orgX, float orgY, float curX, float curY, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags, float clipX, float clipY);
	void DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curXL, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t polyflags, bool center, float spaceX = 0.0f, float spaceY = 0.0f, float clipX = 100000.0f, float clipY = 100000.0f, bool noDraw = false);
	void DrawTextClipped(UFont* font, vec4 color, float orgX, float orgY, float curX, float curY, const std::string& text, uint32_t polyflags, bool checkHotKey, float clipX, float clipY, bool center);
	vec2 GetTextSize(UFont* font, const std::string& text, float spaceX = 0.0f, float spaceY = 0.0f);

	void DrawTile(FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags);
	void Draw2DLine(vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2, bool useUIScale);
	void Draw3DLine(vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2);
	void UpdateTexture(UTexture* tex);

	void DrawViewport(UViewportWindow* viewport);

	bool ShowTimedemoStats = false;
	bool ShowRenderStats = false;
	bool ShowCollisionDebug = false;

	int TextureFrameCounter = 0;
	int FrameCounter = 0;

	vec3* GetTempVertexBuffer(size_t count)
	{
		if (VertexBuffer.size() < count)
			VertexBuffer.resize(count);
		return VertexBuffer.data();
	}

	GouraudVertex* GetTempGouraudVertexBuffer(size_t count)
	{
		if (GouraudVertexBuffer.size() < count)
			GouraudVertexBuffer.resize(count);
		return GouraudVertexBuffer.data();
	}

	void UpdateTextureInfo(FTextureInfo& info, BspSurface& surface, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed);
	void UpdateTextureInfo(FTextureInfo& info, const Poly& poly, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed);
	void UpdateTextureInfo(FTextureInfo& info, UTexture* texture);

	void SetTile3DOffset(bool enabled, std::optional<vec3> offset, std::optional<Rotator> rotOffset, std::optional<bool> bFlatZ, std::optional<float> Scale, std::optional<bool> bWorldOffset);

	RenderDevice* Device = nullptr;

	struct
	{
		Array<UTexture*> textures;
		UTexture* envmap = nullptr;
	} Mesh;

	struct
	{
		int Frames = 0;
		int Surfaces = 0;
		int Actors = 0;
		int LightmapsUpdated = 0;
	} Stats;

	VisibleFrame MainFrame;

private:
	void DrawScene();

	void ResetCanvas();
	void PreRender();
	void RenderOverlays();
	void PostRender();
	void PostRenderFlash();
	void DrawTimedemoStats();
	void DrawCollisionDebug();
	void DrawTile(FTextureInfo& texinfo, const Rectf& dest, const Rectf& src, const Rectf& clipBox, float Z, vec4 color, vec4 fog, uint32_t flags);

	static Array<std::string> FindTextBlocks(const std::string& text);
	void DrawTextBlockRange(float x, float y, const Array<std::string>& textBlocks, size_t start, size_t end, UFont* font, vec4 color, uint32_t polyflags, float spaceX);

	float LevelTimeElapsed = 0.0f;
	float AutoUV = 0.0f;

	struct
	{
		int uiscale = 1;
		int fps = 0;
		int framesDrawn = 0;
		uint64_t startFPSTime = 0;
		FSceneNode Frame;
	} Canvas;

	// Unreal 227
	struct
	{
		vec3 Offset = vec3();
		Rotator RotOffset = Rotator();
		float Scale = 1.0f;
		bool Enabled = false;
		bool FlatZ = false;
		bool bWorldOffset = false; // 227k - Offset and RotOffset are absolute world coordinates
	} Tile3DOffset;

	Array<vec3> VertexBuffer;
	Array<GouraudVertex> GouraudVertexBuffer;
};
