#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

class PackageManager;
class UObject;
class ULevel;
class UModel;
class Viewport;
class UTexture;
class UActor;
class UFont;
class UMesh;
class ULodMesh;
class USkeletalMesh;
class BspSurface;
class BspNode;
class LightMapIndex;
class FrustumPlanes;
class AudioPlayer;
class Rotator;
struct FTextureInfo;
struct FSceneNode;
struct FSurfaceFacet;
struct MeshFace;

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

	void WindowClose(Viewport* viewport);
	void Key(Viewport* viewport, std::string key);
	void InputEvent(Viewport* viewport, EInputKey key, EInputType type, int delta = 0);
	void SetPause(bool value);

private:
	void Tick(float timeElapsed);
	void GenerateShadowmaps();
	void DrawShadowmap(int index, UActor* light, CubeSide side);
	void DrawScene();
	void DrawCoronas(FSceneNode *frame);
	void DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass);
	void DrawNodeSurface(FSceneNode* frame, UModel* model, const BspNode& node, int pass);
	void DrawFontTextWithShadow(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	void DrawFontText(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	ivec2 GetFontTextSize(UFont* font, const std::string& text);

	void DrawMesh(FSceneNode* frame, UMesh* mesh, const vec3& location, const Rotator& rotation, float drawscale);
	void DrawMesh(FSceneNode* frame, UMesh* mesh, const mat4& ObjectToWorld, vec3 color);
	void DrawLodMesh(FSceneNode* frame, ULodMesh* mesh, const mat4& ObjectToWorld, vec3 color);
	void DrawLodMeshFace(FSceneNode* frame, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, vec3 color, int vertexOffset);
	void DrawSkeletalMesh(FSceneNode* frame, USkeletalMesh* mesh, const mat4& ObjectToWorld, vec3 color);

	void DrawSprite(FSceneNode* frame, UTexture* texture, const vec3& location, const Rotator& rotation, float drawscale);
	void DrawBrush(FSceneNode* frame, UModel* brush, const vec3& location, const Rotator& rotation, float drawscale);

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
	FSceneNode CreateShadowmapFrame(UActor* light, CubeSide side);

	void LoadMap(const std::string& packageName);

	FTextureInfo GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet);
	std::unique_ptr<UTexture> CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface);
	void DrawLightmapSpan(vec3* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light, const vec3& N, const uint8_t* bits, int& bitpos);

	std::unique_ptr<PackageManager> packages;

	UFont* bigfont = nullptr;
	UFont* largefont = nullptr;
	UFont* medfont = nullptr;
	UFont* smallfont = nullptr;

	UObject* LevelSummary = nullptr;
	UObject* LevelInfo = nullptr;
	ULevel* level = nullptr;

	std::vector<UActor*> Lights;
	std::vector<UTexture*> Textures;

	std::unique_ptr<Viewport> viewport;
	std::unique_ptr<AudioPlayer> audioplayer;

	std::map<int, std::unique_ptr<UTexture>> lmtextures;

	std::vector<int> portalsvisited;

	struct ActorPos
	{
		float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;
		vec3 Location = { 200.0f, 1200.0f, 1300.0f };
	};
	
	ActorPos Camera;

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
