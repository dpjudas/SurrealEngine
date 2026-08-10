#pragma once

#include "Math/vec.h"
#include "LightActorTree.h"
#include "LightmapBuilder.h"
#include "Packages/Engine/Resources/Textures/UBitmap.h"
#include <unordered_map>
#include <list>

class ULevel;
class UActor;
class CollisionHitList;
class CollisionHit;
struct TextureInfo;
class UMover;

struct LightmapTexture
{
	TextureFormat Format;
	UnrealMipmap Mip;
	int LastUpdate = 0;
};

class LightSystem
{
public:
	LightSystem();
	~LightSystem();

	void Tick(float levelTimeElapsed);

	void OnMapLoaded();

	void BeginFrame();
	void UpdateLightList(UActor* actor);

	void SetLevel(ULevel* level);

	TextureInfo GetMoverLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model);
	TextureInfo GetLevelLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model);

	TextureInfo GetMoverFogmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model);
	TextureInfo GetLevelFogmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model);

	vec3 GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit, UZoneInfo* zoneActor);
	vec4 GetVertexFog(UActor* actor, const vec3& location);

private:
	TextureInfo GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor, const vec3& worldLocation, float radius, UMover* mover);
	TextureInfo GetFogmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor);
	void CheckLight(UActor* light);
	void UpdateFogmapTexture(uint32_t* texels, UModel* model, const Coords& mapCoords, int lightMap, UZoneInfo* zoneActor);

	ULevel* Level = nullptr;

	LightmapBuilder Builder;
	std::map<uint64_t, std::unique_ptr<LightmapTexture>> lmtextures;
	std::map<uint64_t, std::pair<int, std::unique_ptr<LightmapTexture>>> fogtextures;
	Array<UActor*> FogBalls;
	int FrameCounter = 0;
	int LightmapCheckCounter = 0;
	int NextMoverID = 1;

	float AmbientGlowTime = 0.0f;
	float AmbientGlowAmount = 0.0f;

	Array<UActor*> TempDynLightList;
	LightActorTree LightTree;
};
