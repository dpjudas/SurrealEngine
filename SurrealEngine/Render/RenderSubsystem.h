#pragma once

#include "VisibleFrame.h"
#include "Lightmap/LightmapBuilder.h"
#include "VR/VRSubsystem.h"
#include "VR/VRWheel.h"

class UInventory;

class RenderDevice;
class UWindow;
class UFont;

struct LightmapTexture
{
	TextureFormat Format;
	UnrealMipmap Mip;
};

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
	void Draw2DLine(vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2);
	void Draw3DLine(vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2);
	void UpdateTexture(UTexture* tex);

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

	FTextureInfo GetBrushLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetSurfaceLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor);

	FTextureInfo GetBrushFogmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetSurfaceFogmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetFogmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor);

	vec3 GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit, UZoneInfo* zoneActor);
	vec4 GetVertexFog(UActor* actor, const vec3& location);

	void UpdateTextureInfo(FTextureInfo& info, BspSurface& surface, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed);
	void UpdateTextureInfo(FTextureInfo& info, const Poly& poly, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed);
	void UpdateTextureInfo(FTextureInfo& info, UTexture* texture);

	void UpdateActorLightList(UActor* actor);

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
	} Stats;

	VisibleFrame MainFrame;

private:
	void DrawGameFrame(vec4 flashScale, vec4 flashFog, bool presentToDesktop);
	void DrawScene();
	void BuildVREyeView(mat4& worldToView, mat4& projection, Coords& headRotation, vec3& headPosition, const vec3& cameraLocation, const Rotator& cameraRotation) const;
	vec3 RemoveRoomScaleOffset(const vec3& playSpacePosition) const;

	// VR menu: when the console/menu takes over the whole frame (bNoDrawWorld()), the flat 2D canvas is
	// unreadable/out of the headset lenses' visible range if stretched across an eye's full (very wide)
	// viewport like normal gameplay HUD is. Instead it's rendered once to an offscreen canvas (DrawUICanvas)
	// and then drawn as a single quad positioned in 3D world space (DrawVRMenuPlane), through the same
	// per-eye VR projection used for real 3D geometry, so it's fully visible with correct stereo depth.
	// transparentBackground clears the canvas alpha to 0 (the HUD tablet, composited over the world) rather
	// than opaque black (the pause menu, which blacks the world out).
	void DrawUICanvas(bool transparentBackground);
	void CaptureVRMenuPlaneAnchor(const VRSubsystem::EyeView eyeViews[2]);
	// Casts the pointer hand's laser at the (already anchored) menu plane, drives the menu cursor from
	// where it hits, and stores the beam for DrawVRMenuPlane to draw per eye. Call once per frame while the
	// menu is up, after the plane anchor exists and before the canvas is drawn.
	void UpdateVRMenuLaser();
	void DrawVRMenuPlane();
	void DrawVRMenuEyeFrame(vec4 flashScale, vec4 flashFog, bool presentToDesktop);

	// The gameplay HUD as a wrist tablet: draws the offscreen UI canvas (DrawUICanvas) as a world-space
	// plane anchored to the off-hand controller, composited on top of the world in the current eye. Called
	// from DrawGameFrame in place of the flat PostRender HUD whenever a VR eye is current. No-op on desktop.
	void DrawVRHudPlane();

	// Draws where the motion controllers are, so the player can see what their hand collider is about to
	// touch (see VRHands). No-op outside a VR eye.
	void DrawVRHands();

	// Draws the weapon/item wheel (VR/VRWheel.h) when one is open: each entry laid out around the wheel's
	// captured centre - weapons as 3D meshes (the phase-4 look), items as camera-facing icon billboards -
	// plus a highlight ring so "centred = cancel" and "which entry is about to be picked" both read at a
	// glance. No-op outside a VR eye or when no wheel is open.
	void DrawVRWheel();
	// One weapon entry: saves Location/Rotation/DrawScale/Mesh, points Mesh() at PickupViewMesh() (sized
	// for "lying in the world", not the tiny welded-to-camera PlayerViewMesh), places it at the slot
	// transform oriented by (forward, up), draws via VisibleMesh into MainFrame, restores.
	void DrawWheelEntryMesh(UInventory* item, const vec3& position, const vec3& forward, const vec3& up, bool highlighted);
	// One item entry: a camera-facing textured quad at the slot, using the item's Icon() - the same
	// texture the 2D HUD shows, so it is the representation players already recognise. Reuses the
	// DrawGouraudPolygon sprite technique VisibleSprite uses for world-space billboards.
	void DrawWheelItemIcon(UInventory* item, const vec3& position, bool highlighted);
	// The disc outline, a centre "cancel" marker, and a brighter spoke toward the highlighted entry.
	void DrawWheelHighlightRing(const VRWheel& wheel);

	// Rides the current pawn->SelectedItem() (set via the item wheel) on the off hand, the way the equipped
	// weapon rides the weapon hand (phase 4) - its own offset/scale knobs (LauncherSettings::VR Item*),
	// since an item mesh's origin/size has no reason to match a weapon's. Falls back to the icon billboard
	// when the item has no held mesh; draws nothing when no item is selected, or while a wheel has the off
	// hand busy. No-op outside a VR eye.
	void DrawVRActiveItem();

	std::unique_ptr<LightmapTexture> CreateLightmapTexture();

	void UpdateFogmapTexture(uint32_t* texels, UModel* model, const Coords& mapCoords, int lightMap, UZoneInfo* zoneActor);

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
	float AmbientGlowTime = 0.0f;
	float AmbientGlowAmount = 0.0f;

	const VRSubsystem::EyeView* CurrentVREye = nullptr; // Set only while DrawScene() is rendering a VR eye (see DrawGame)

	// See DrawUICanvas() / CaptureVRMenuPlaneAnchor() / DrawVRMenuPlane().
	bool DrawingVRMenuCanvas = false;
	// True only while rendering the HUD tablet's canvas (not the pause menu's). ResetCanvas reads it to
	// boost uiscale so HUD elements are larger on the tablet; the menu keeps the plain derived scale, which
	// its cursor mapping (UpdateVRMenuLaser) also assumes.
	bool DrawingVRHudCanvas = false;
	// Whether the menu plane's world-space anchor has been captured for the currently-open menu. Set on the
	// frame the menu opens (freezing the plane in front of the player's gaze) and cleared when it closes, so
	// the anchor is taken exactly once per menu session. Deliberately tracked as "anchor captured" rather
	// than "menu was open last frame": a one-frame VR dropout draws the desktop fallback without touching
	// this, so the frozen plane survives the dropout instead of jumping to the new gaze when VR resumes.
	bool VRMenuAnchorCaptured = false;
	vec3 VRMenuPlaneCorners[4] = {};
	vec2 VRMenuPlaneUVs[4] = { vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 1.0f) };
	// The camera transform at the moment the menu opened, frozen for as long as it stays open - a scripted
	// cinematic camera (e.g. the intro flythrough) keeps ticking/animating even while bNoDrawWorld() is
	// true, so re-deriving the live camera transform every frame would make the (world-space-fixed) menu
	// plane appear to drift as the cinematic keeps moving, even though the player's own head hasn't.
	vec3 VRMenuFrozenCameraLocation = vec3(0.0f);
	Rotator VRMenuFrozenCameraRotation = Rotator(0, 0, 0);

	// The pointer laser for the current frame, in the same world space as VRMenuPlaneCorners. Recomputed by
	// UpdateVRMenuLaser every frame the menu is open (the hand moves), drawn by DrawVRMenuPlane per eye.
	bool VRMenuLaserValid = false;
	vec3 VRMenuLaserStart = vec3(0.0f);
	vec3 VRMenuLaserEnd = vec3(0.0f);

	// A member, not a local in DrawVRHands: RenderDevice::SetSceneNode keeps the pointer it is handed
	// (VulkanRenderDevice::CurrentFrame) and derefs it again from EndFlash() later in the same frame, so
	// the scene node has to outlive the call that set it.
	FSceneNode VRHandsFrame;

	// Same lifetime reason as VRHandsFrame, for the wrist HUD tablet (DrawVRHudPlane).
	FSceneNode VRHudFrame;

	struct
	{
		int uiscale = 1;
		int fps = 0;
		int framesDrawn = 0;
		uint64_t startFPSTime = 0;
		FSceneNode Frame;
	} Canvas;

	struct
	{
		std::map<uint64_t, std::unique_ptr<LightmapTexture>> lmtextures;
		std::map<uint64_t, std::pair<int, std::unique_ptr<LightmapTexture>>> fogtextures;
		Array<UActor*> FogBalls;
		LightmapBuilder Builder;
		int FogFrameCounter = 0;
	} Light;

	Array<vec3> VertexBuffer;
	Array<GouraudVertex> GouraudVertexBuffer;
};
