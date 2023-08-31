#pragma once

#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"
#include "UObject/UClient.h"
#include "RenderDevice/RenderDevice.h"
#include "BspClipper.h"
#include "Lightmap/LightmapBuilder.h"

class RenderDevice;

struct DrawNodeInfo
{
	BspNode* Node;
	uint32_t PolyFlags;
};

struct LightmapTexture
{
	TextureFormat Format;
	UnrealMipmap Mip;
};

class RenderSubsystem
{
public:
	RenderSubsystem(RenderDevice* renderdevice);

	void DrawEditorViewport();

	void DrawGame(float levelTimeElapsed);
	void OnMapLoaded();

	void DrawActor(UActor* actor, bool WireFrame, bool ClearZ);
	void DrawClippedActor(UActor* actor, bool WireFrame, int X, int Y, int XB, int YB, bool ClearZ);
	void DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags);
	void DrawTileClipped(UTexture* Tex, float orgX, float orgY, float curX, float curY, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags, float clipX, float clipY);
	void DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t flags, bool center, float spaceX = 0.0f, float spaceY = 0.0f);
	void DrawTextClipped(UFont* font, vec4 color, float orgX, float orgY, float curX, float curY, const std::string& text, uint32_t flags, bool checkHotKey, float clipX, float clipY, bool center);
	ivec2 GetTextSize(UFont* font, const std::string& text);
	ivec2 GetTextClippedSize(UFont* font, const std::string& text, float clipX);

	void UpdateTexture(UTexture* tex);

	bool ShowTimedemoStats = false;
	bool ShowCollisionDebug = false;

private:
	void DrawScene();
	void DrawFrame(const vec3& location, const mat4& worldToView);
	int FindZoneAt(const vec3& location);
	int FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes);
	void ProcessNode(BspNode* node);
	void ProcessNodeSurface(BspNode* node);
	void DrawNodeSurface(const DrawNodeInfo& nodeInfo);
	void DrawActors();
	void SetupSceneFrame(const mat4& worldToView);

	FTextureInfo GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model);
	std::unique_ptr<LightmapTexture> CreateLightmapTexture(const BspSurface& surface, UZoneInfo* zoneActor, UModel* model);
	void UpdateActorLightList(UActor* actor);
	vec3 GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit);

	FTextureInfo GetSurfaceFogmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model);
	void UpdateFogmapTexture(const LightMapIndex& lmindex, uint32_t* texels, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model);

	void ResetCanvas();
	void PreRender();
	void RenderOverlays();
	void PostRender();
	void DrawTimedemoStats();
	void DrawCollisionDebug();
	void DrawTile(FTextureInfo& texinfo, const Rectf& dest, const Rectf& src, const Rectf& clipBox, float Z, vec4 color, vec4 fog, uint32_t flags);

	void DrawMesh(FSceneNode* frame, UActor* actor, bool wireframe = false);
	void DrawMesh(FSceneNode* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld);
	void DrawLodMesh(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld);
	void DrawLodMeshFace(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, int baseVertexOffset, const int* vertexOffsets, float t0, float t1);
	void DrawSkeletalMesh(FSceneNode* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld);
	void SetupMeshTextures(UActor* actor, UMesh* mesh);
	void SetupLodMeshTextures(UActor* actor, ULodMesh* mesh);

	void DrawBrush(FSceneNode* frame, UActor* actor);
	void DrawNodeSurfaceGouraud(FSceneNode* frame, UModel* model, const BspNode& node, int pass, UActor* actor);

	void DrawSprite(FSceneNode* frame, UActor* actor);
	void DrawCoronas(FSceneNode* frame);
	void DrawDecals(FSceneNode* frame);

	RenderDevice* Device = nullptr;

	float LevelTimeElapsed = 0.0f;
	float AutoUV = 0.0f;
	int FrameCounter = 0;

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
		std::vector<UTexture*> textures;
		UTexture* envmap = nullptr;
	} Mesh;

	struct
	{
		FSceneNode Frame;
		BspClipper Clipper;
		vec4 ViewLocation;
		Coords ViewRotation;
		int ViewZone = 0;
		uint64_t ViewZoneMask = 0;
		std::vector<DrawNodeInfo> OpaqueNodes;
		std::vector<DrawNodeInfo> TranslucentNodes;
		std::vector<UActor*> Coronas;
		std::vector<UActor*> Actors;
		int FrameCounter = 0;
	} Scene;

	struct
	{
		std::map<uint64_t, std::unique_ptr<LightmapTexture>> lmtextures;
		std::map<uint64_t, std::pair<int, std::unique_ptr<LightmapTexture>>> fogtextures;
		std::vector<UActor*> Lights;
		LightmapBuilder Builder;
		int FogFrameCounter = 0;
	} Light;

	std::vector<vec3> VertexBuffer;

	vec3* GetTempVertexBuffer(size_t count)
	{
		if (VertexBuffer.size() < count)
			VertexBuffer.resize(count);
		return VertexBuffer.data();
	}
};
