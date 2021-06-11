#pragma once

#include "Math/vec.h"
#include "Math/mat.h"
#include "RenderDevice/RenderDevice.h"
#include <set>

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
class ULevelSummary;
class ULevelInfo;
class UZoneInfo;
class BspSurface;
class BspNode;
class LightMapIndex;
class FrustumPlanes;
class AudioPlayer;
class Rotator;
class ExpressionValue;
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

	static Engine* Instance;

	void Run();

	void WindowClose(Viewport* viewport);
	void Key(Viewport* viewport, std::string key);
	void InputEvent(Viewport* viewport, EInputKey key, EInputType type, int delta = 0);
	void SetPause(bool value);

	void DrawFontTextWithShadow(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	void DrawFontText(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment = TextAlignment::left);
	ivec2 GetFontTextSize(UFont* font, const std::string& text);
	void DrawTile(FSceneNode* frame, UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags);

	ULevelSummary* LevelSummary = nullptr;
	ULevelInfo* LevelInfo = nullptr;
	ULevel* level = nullptr;

	FSceneNode SceneFrame;
	std::unique_ptr<PackageManager> packages;

	template<typename T>
	T* NewObject(const std::string& name, const std::string& package, const std::string& className);
	ExpressionValue InvokeEvent(UObject* obj, const std::string& name, std::vector<ExpressionValue> args);

private:
	void Tick(float timeElapsed);
	void GenerateShadowmaps();
	void DrawShadowmap(int index, UActor* light, CubeSide side);
	void DrawScene();
	void DrawCoronas(FSceneNode *frame);
	void DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass);
	void DrawNodeSurface(FSceneNode* frame, UModel* model, const BspNode& node, int pass);
	void DrawNodeSurfaceGouraud(FSceneNode* frame, UModel* model, const BspNode& node, int pass, const vec3& color = { 0.0f });

	void DrawMesh(FSceneNode* frame, UMesh* mesh, const vec3& location, const Rotator& rotation, float drawscale, int zoneIndex);
	void DrawMesh(FSceneNode* frame, UMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
	void DrawLodMesh(FSceneNode* frame, ULodMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
	void DrawLodMeshFace(FSceneNode* frame, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& color, int vertexOffset);
	void DrawSkeletalMesh(FSceneNode* frame, USkeletalMesh* mesh, const mat4& ObjectToWorld, const vec3& color);

	void DrawSprite(FSceneNode* frame, UTexture* texture, const vec3& location, const Rotator& rotation, float drawscale);
	void DrawBrush(FSceneNode* frame, UModel* brush, const vec3& location, const Rotator& rotation, float drawscale, int zoneIndex);

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

	vec3 FindLightAt(const vec3& location, int zoneIndex);
	FTextureInfo GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model);
	UTexture* CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model);
	void DrawLightmapSpan(vec3* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light, const vec3& N, const uint8_t* bits, int& bitpos);

	UFont* bigfont = nullptr;
	UFont* largefont = nullptr;
	UFont* medfont = nullptr;
	UFont* smallfont = nullptr;

	std::vector<UActor*> Lights;
	std::set<UTexture*> Textures;

	std::unique_ptr<Viewport> viewport;
	std::unique_ptr<AudioPlayer> audioplayer;

	std::vector<int> portalsvisited;

	struct ActorPos
	{
		float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;
		vec3 Location = { 200.0f, 1200.0f, 1300.0f };
	};
	
	ActorPos Camera;

	UActor* SkyZoneInfo = nullptr;
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
