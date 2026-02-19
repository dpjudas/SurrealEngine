#pragma once

#include "UObject/UObject.h"

class NPawn
{
public:
	static void RegisterFunctions();

	static void AddPawn(UObject* Self);
	static void AIPickRandomDestination(UObject* Self, float minDist, float maxDist, int centralYaw, float yawDistribution, int centralPitch, float pitchDistribution, int tries, float multiplier, vec3& dest);
	static void CanSee(UObject* Self, UObject* Other, BitfieldBool& ReturnValue);
	static void CheckValidSkinPackage(const std::string& SkinPack, const std::string& MeshName, BitfieldBool& ReturnValue);
	static void ClearPaths(UObject* Self);
	static void ClientHearSound(UObject* Self, UObject* Actor, int Id, UObject* S, const vec3& SoundLocation, const vec3& Parameters);
	static void EAdjustJump(UObject* Self, vec3& ReturnValue);
	static void FindBestInventoryPath(UObject* Self, float& MinWeight, bool bPredictRespawns, UObject*& ReturnValue);
	static void FindPathTo(UObject* Self, const vec3& aPoint, BitfieldBool* bSinglePath, BitfieldBool* bClearPaths, UObject*& ReturnValue);
	static void FindPathToward(UObject* Self, UObject* anActor, BitfieldBool* bSinglePath, BitfieldBool* bClearPaths, UObject*& ReturnValue);
	static void FindRandomDest(UObject* Self, BitfieldBool* bClearPaths, UObject*& ReturnValue);
	static void FindStairRotation(UObject* Self, float DeltaTime, int& ReturnValue);
	static void LineOfSightTo(UObject* Self, UObject* Other, BitfieldBool& ReturnValue);
	static void MoveTo(UObject* Self, const vec3& NewDestination, float* speed);
	static void MoveToward(UObject* Self, UObject* NewTarget, float* speed);
	static void PickAnyTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue);
	static void PickTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue);
	static void PickWallAdjust(UObject* Self, BitfieldBool& ReturnValue);
	static void RemovePawn(UObject* Self);
	static void StopWaiting(UObject* Self);
	static void StrafeFacing(UObject* Self, const vec3& NewDestination, UObject* NewTarget);
	static void StrafeTo(UObject* Self, const vec3& NewDestination, const vec3& NewFocus);
	static void TurnTo(UObject* Self, const vec3& NewFocus);
	static void TurnToward(UObject* Self, UObject* NewTarget);
	static void WaitForLanding(UObject* Self);
	static void actorReachable(UObject* Self, UObject* anActor, BitfieldBool& ReturnValue);
	static void pointReachable(UObject* Self, const vec3& aPoint, BitfieldBool& ReturnValue);

	static void AICanHear(UObject* Self, UObject* Other, float* Volume, float* Radius, float& ReturnValue);
	static void AICanSee(UObject* Self, UObject* Other, float* Visibility, BitfieldBool* bCheckVisibility, BitfieldBool* bCheckDir, BitfieldBool* bCheckCylinder, BitfieldBool* bCheckLOS, float& ReturnValue);
	static void AICanSmell(UObject* Self, UObject* Other, float* Smell, float& ReturnValue);
	static void AIDirectionReachable(UObject* Self, const vec3& Focus, int Yaw, int Pitch, float minDist, float maxDist, vec3& bestDest, BitfieldBool& ReturnValue);
	static void AIPickRandomDestination_Deus(UObject* Self, float minDist, float maxDist, int centralYaw, float yawDistribution, int centralPitch, float pitchDistribution, int tries, float multiplier, vec3& dest, BitfieldBool& ReturnValue);
	static void ComputePathnodeDistances(UObject* Self, UObject** startActor);
	static void LineOfSightTo_Deus(UObject* Self, UObject* Other, BitfieldBool* bIgnoreDistance, BitfieldBool& ReturnValue);
	static void ReachablePathnodes(UObject* Self, UObject* BaseClass, UObject*& NavPoint, UObject* FromPoint, float& distance, BitfieldBool* bUsePrunedPaths);
	static void StrafeFacing_Deus(UObject* Self, const vec3& NewDestination, UObject* NewTarget, float* speed);
	static void StrafeTo_Deus(UObject* Self, const vec3& NewDestination, const vec3& NewFocus, float* speed);
};
