#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

class PackageManager;
class TextureManager;
class Level;
class UViewport;
class UnrealTexture;
class UnrealMipmap;
class BspSurface;
class BspNode;
class LightMapIndex;
class FrustumPlanes;
class Font;
struct FTextureInfo;
struct FSceneNode;
struct FSurfaceFacet;

enum EInputKey
{
	IK_LeftMouse,
	IK_MiddleMouse,
	IK_RightMouse,
	IK_MouseWheelUp,
	IK_MouseWheelDown,
	IK_MouseX,
	IK_MouseY
};

enum EInputType
{
	IST_Press,
	IST_Release,
	IST_Axis
};

struct Light
{
	vec3 Location = { 0.0f };
	int LightBrightness = 64;
	int LightHue = 0;
	int LightSaturation = 255;
	int LightRadius = 64;
	bool bCorona = false;
	UnrealTexture* Skin = nullptr;
};

enum class CubeSide
{
	XPositive,
	XNegative,
	YPositive,
	YNegative,
	ZPositive,
	ZNegative
};

enum class TextAlignment
{
	left,
	center,
	right
};

class Engine
{
public:
	Engine();
	~Engine();

	void Run();

	void WindowClose(UViewport* viewport);
	void Key(UViewport* viewport, std::string key);
	void InputEvent(UViewport* viewport, EInputKey key, EInputType type, int delta = 0);
	void SetPause(bool value);

private:
	void Tick(float timeElapsed);
	void GenerateShadowmaps();
	void DrawShadowmap(int index, const Light& light, CubeSide side);
	void DrawScene();
	void DrawCoronas(FSceneNode *frame);
	void DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass);
	void DrawNodeSurface(FSceneNode* frame, const BspNode& node, int pass);
	void DrawFontTextWithShadow(FSceneNode* frame, Font* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	void DrawFontText(FSceneNode* frame, Font* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	ivec2 GetFontTextSize(Font* font, const std::string& text);

	bool TraceAnyHit(vec3 from, vec3 to);
	bool TraceAnyHit(const vec4& from, const vec4& to, BspNode* node, BspNode* nodes);
	bool HitTestNodePolygon(const vec4& from, const vec4& to, BspNode* node);

	int FindZoneAt(vec3 location);
	int FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes);

	uint64_t FindRenderZoneMask(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int zone);
	uint64_t FindRenderZoneMaskForPortal(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int portalzone);

	float CalcTimeElapsed();

	mat4 CoordsMatrix();

	FSceneNode CreateSceneFrame();
	FSceneNode CreateSkyFrame();
	FSceneNode CreateShadowmapFrame(const Light& light, CubeSide side);

	void LoadMap(const std::string& packageName);

	FTextureInfo GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet);
	std::unique_ptr<UnrealTexture> CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface);
	void DrawLightmapSpan(vec3* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, const Light& light, const vec3& N, const uint8_t* bits, int& bitpos);

	std::unique_ptr<PackageManager> packages;
	std::unique_ptr<TextureManager> textures;
	std::unique_ptr<Font> bigfont, largefont, medfont, smallfont;
	std::unique_ptr<Level> level;

	std::unique_ptr<UViewport> viewport;

	std::map<int, std::unique_ptr<Light>> lightactors;
	std::vector<Light*> lmlights;

	std::map<int, std::unique_ptr<UnrealTexture>> lmtextures;

	std::vector<int> portalsvisited;

	struct
	{
		float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;
		vec3 Location = { 200.0f, 1200.0f, 1300.0f };
	} Camera;

	bool HasSkyZoneInfo = false;
	vec3 SkyLocation = { 0.0f };
	float AutoUVTime = 0.0f;

	struct
	{
		float SpeedX = 0.08f;
		float SpeedY = 0.08f;
	} Mouse;

	struct
	{
		bool Forward = false;
		bool Backward = false;
		bool StrafeLeft = false;
		bool StrafeRight = false;
		bool Jump = false;
		bool Crouch = false;
	} Buttons;

	static vec3 hsbtorgb(double hue, double saturation, double brightness);

	bool quit = false;

	uint64_t lastTime = 0;

	uint64_t startFPSTime = 0;
	int framesDrawn = 0;
	int fps = 0;
};
