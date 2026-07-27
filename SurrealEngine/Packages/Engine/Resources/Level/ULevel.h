#pragma once

#include "ULevelBase.h"
#include "Math/bbox.h"
#include "Collision/TopLevel/CollisionSystem.h"
#include "Collision/TopLevel/CollisionHit.h"
#include "Light/LightSystem.h"

class UNavigationPoint;
class UModel;

enum EReachSpecFlags
{
	R_WALK = 1,
	R_FLY = 2,
	R_SWIM = 4,
	R_JUMP = 8,
	R_DOOR = 16,
	R_SPECIAL = 32,
	R_PLAYERONLY = 64
};

class LevelReachSpec
{
public:
	int32_t distance = 0;
	UNavigationPoint* startActor = nullptr;
	UNavigationPoint* endActor = nullptr;
	int32_t collisionRadius = 0;
	int32_t collisionHeight = 0;
	int32_t reachFlags = 0;
	int8_t bPruned = 0;
};

class ULevel : public ULevelBase
{
public:
	ULevel(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void Tick(float elapsed, bool gamePaused);

	Array<LevelReachSpec> ReachSpecs;
	UModel* Model = nullptr;

	CollisionSystem Collision;
	LightSystem Light;
	std::map<std::string, std::string> TravelInfo;

private:
	void TickActor(float elapsed, UActor* actor);

	bool ticked = false;
};
