#pragma once

#include "Math/vec.h"
#include "LightmapBuilder.h"
#include "Packages/Engine/Resources/Textures/UBitmap.h"
#include <unordered_map>
#include <list>

class ULevel;
class UActor;
class CollisionHitList;
class CollisionHit;
struct FTextureInfo;
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
	void AddLight(UActor* light);
	void RemoveLight(UActor* light);

	FTextureInfo GetMoverLightmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetLevelLightmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model);

	FTextureInfo GetMoverFogmap(UMover* mover, const Poly& poly, UZoneInfo* zoneActor, UModel* model);
	FTextureInfo GetLevelFogmap(BspSurface& surface, UZoneInfo* zoneActor, UModel* model);

	vec3 GetVertexLight(UActor* actor, const vec3& location, const vec3& normal, bool unlit, UZoneInfo* zoneActor);
	vec4 GetVertexFog(UActor* actor, const vec3& location);

private:
	FTextureInfo GetLightmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor);
	FTextureInfo GetFogmap(UModel* model, int lightmapIndex, const Coords& coords, UZoneInfo* zoneActor);

	void UpdateFogmapTexture(uint32_t* texels, UModel* model, const Coords& mapCoords, int lightMap, UZoneInfo* zoneActor);

	static ivec3 GetStartExtents(const vec3& location, const vec3& extents)
	{
		int xx = (int)std::floor((location.x - extents.x) * (1.0f / 256.0f));
		int yy = (int)std::floor((location.y - extents.y) * (1.0f / 256.0f));
		int zz = (int)std::floor((location.z - extents.z) * (1.0f / 256.0f));
		return { xx, yy, zz };
	}

	static ivec3 GetStartExtents(const vec3& location, float radius)
	{
		int xx = (int)std::floor((location.x - radius) * (1.0f / 256.0f));
		int yy = (int)std::floor((location.y - radius) * (1.0f / 256.0f));
		int zz = (int)std::floor((location.z - radius) * (1.0f / 256.0f));
		return { xx, yy, zz };
	}

	static ivec3 GetEndExtents(const vec3& location, const vec3& extents)
	{
		int xx = (int)std::floor((location.x + extents.x) * (1.0f / 256.0f)) + 1;
		int yy = (int)std::floor((location.y + extents.y) * (1.0f / 256.0f)) + 1;
		int zz = (int)std::floor((location.z + extents.z) * (1.0f / 256.0f)) + 1;
		return { xx, yy, zz };
	}

	static ivec3 GetEndExtents(const vec3& location, float radius)
	{
		int xx = (int)std::floor((location.x + radius) * (1.0f / 256.0f)) + 1;
		int yy = (int)std::floor((location.y + radius) * (1.0f / 256.0f)) + 1;
		int zz = (int)std::floor((location.z + radius) * (1.0f / 256.0f)) + 1;
		return { xx, yy, zz };
	}

	static uint32_t GetBucketId(int x, int y, int z)
	{
		return ((x & 0x3ff) << 20) | ((y & 0x3ff) << 10) | (z & 0x3ff);
	}

	static int NextCheckCounter() { return CheckCounter++; }

	const std::list<UActor*>& GetActors(int x, int y, int z) const
	{
		auto it = LightActors.find(GetBucketId(x, y, z));
		if (it != LightActors.end())
			return it->second;
		else
			return emptyList;
	}

	ULevel* Level = nullptr;
	std::unordered_map<uint32_t, std::list<UActor*>> LightActors;

	inline static std::list<UActor*> emptyList;
	inline static int CheckCounter = 0;

	LightmapBuilder Builder;
	std::map<uint64_t, std::unique_ptr<LightmapTexture>> lmtextures;
	std::map<uint64_t, std::pair<int, std::unique_ptr<LightmapTexture>>> fogtextures;
	Array<UActor*> FogBalls;
	int FogFrameCounter = 0;

	float AmbientGlowTime = 0.0f;
	float AmbientGlowAmount = 0.0f;
};
