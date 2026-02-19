#pragma once

#include "UObject/UObject.h"

class NActor
{
public:
	static void RegisterFunctions();

	static void Add_ColorColor(const Color& A, const Color& B, Color& ReturnValue);
	static void AllActors(UObject* Self, UObject* BaseClass, UObject*& Actor, NameString* MatchTag);
	static void AutonomousPhysics(UObject* Self, float DeltaSeconds);
	static void BasedActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void ChildActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue);
	static void CycleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, int& index);
	static void DemoPlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, BitfieldBool* bNoOverride, float* Radius, float* Pitch);
	static void Destroy(UObject* Self, BitfieldBool& ReturnValue);
	static void Error(UObject* Self, const std::string& S);
	static void FastTrace(UObject* Self, const vec3& TraceEnd, vec3* TraceStart, BitfieldBool& ReturnValue);
	static void FinishAnim(UObject* Self);
	static void FinishInterpolation(UObject* Self);
	static void GetAnimGroup(UObject* Self, const NameString& Sequence, NameString& ReturnValue);
	static void GetBoundingBox(UObject* Self, vec3& MinVect, vec3& MaxVect, BitfieldBool* bExact, vec3* testLocation, Rotator* testRotation, BitfieldBool& ReturnValue);
	static void GetCacheEntry(UObject* Self, int Num, std::string& Guid, std::string& Filename, BitfieldBool& ReturnValue);
	static void GetMapName(UObject* Self, const std::string& NameEnding, const std::string& MapName, int Dir, std::string& ReturnValue);
	static void GetMeshTexture(UObject* Self, int* texnum, UObject*& ReturnValue);
	static void GetNextInt(UObject* Self, const std::string& ClassName, int Num, std::string& ReturnValue);
	static void GetNextIntDesc(UObject* Self, const std::string& ClassName, int Num, std::string& Entry, std::string& Description);
	static void GetNextSkin(UObject* Self, const std::string& Prefix, const std::string& CurrentSkin, int Dir, std::string& SkinName, std::string& SkinDesc);
	static void GetNextSkin_219(UObject* Self, const std::string& Prefix, const std::string& CurrentSkin, int Dir, std::string& ReturnValue);
	static void GetSoundDuration(UObject* Self, UObject* Sound, float& ReturnValue);
	static void GetURLMap(UObject* Self, std::string& ReturnValue);
	static void HasAnim(UObject* Self, const NameString& Sequence, BitfieldBool& ReturnValue);
	static void IsAnimating(UObject* Self, BitfieldBool& ReturnValue);
	static void IsOverlapping(UObject* Self, UObject* checkActor, BitfieldBool& ReturnValue);
	static void LastRendered(UObject* Self, float& ReturnValue);
	static void LinkSkelAnim(UObject* Self, UObject* Anim);
	static void LoopAnim(UObject* Self, const NameString& Sequence, float* Rate, float* TweenTime, float* MinRate);
	static void MakeNoise(UObject* Self, float Loudness);
	static void Move(UObject* Self, const vec3& Delta, BitfieldBool& ReturnValue);
	static void MoveCacheEntry(UObject* Self, const std::string& Guid, std::string* NewFilename, BitfieldBool& ReturnValue);
	static void MoveSmooth(UObject* Self, const vec3& Delta, BitfieldBool& ReturnValue);
	static void Multiply_ColorFloat(const Color& A, float B, Color& ReturnValue);
	static void Multiply_FloatColor(float A, const Color& B, Color& ReturnValue);
	static void PlayAnim(UObject* Self, const NameString& Sequence, float* Rate, float* TweenTime);
	static void PlayBlendAnim(UObject* Self, const NameString& Sequence, float* Rate, float* TweenTime, int* BlendSlot);
	static void PlayOwnedSound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, BitfieldBool* bNoOverride, float* Radius, float* Pitch);
	static void PlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, BitfieldBool* bNoOverride, float* Radius, float* Pitch);
	static void PlayerCanSeeMe(UObject* Self, BitfieldBool& ReturnValue);
	static void RadiusActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float Radius, vec3* Loc);
	static void SetBase(UObject* Self, UObject* NewBase);
	static void SetCollision(UObject* Self, BitfieldBool* NewColActors, BitfieldBool* NewBlockActors, BitfieldBool* NewBlockPlayers);
	static void SetCollisionSize(UObject* Self, float NewRadius, float NewHeight, BitfieldBool& ReturnValue);
	static void SetLocation(UObject* Self, const vec3& NewLocation, BitfieldBool& ReturnValue);
	static void SetOwner(UObject* Self, UObject* NewOwner);
	static void SetPhysics(UObject* Self, uint8_t newPhysics);
	static void SetRotation(UObject* Self, const Rotator& NewRotation, BitfieldBool& ReturnValue);
	static void SetTimer(UObject* Self, float NewTimerRate, bool bLoop);
	static void Sleep(UObject* Self, float Seconds);
	static void Spawn(UObject* Self, UObject* SpawnClass, UObject** SpawnOwner, NameString* SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation, UObject*& ReturnValue);
	static void Subtract_ColorColor(const Color& A, const Color& B, Color& ReturnValue);
	static void TouchingActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void Trace(UObject* Self, vec3& HitLocation, vec3& HitNormal, const vec3& TraceEnd, vec3* TraceStart, BitfieldBool* bTraceActors, vec3* Extent, UObject*& ReturnValue);
	static void TraceActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, vec3* Start, vec3* Extent);
	static void TweenAnim(UObject* Self, const NameString& Sequence, float Time);
	static void VisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc);
	static void VisibleCollidingActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc, BitfieldBool* bIgnoreHidden);
	static void VisibleCollidingActors_219(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc);
	static void GetPlayerPawn(UObject* Self, UObject*& ReturnValue);
	static void AIClearEvent(UObject* Self, const NameString& eventName);
	static void AIClearEventCallback(UObject* Self, const NameString& eventName);
	static void AIEndEvent(UObject* Self, const NameString& eventName, uint8_t eventType);
	static void AIGetLightLevel(UObject* Self, const vec3& Location, float& ReturnValue);
	static void AISendEvent(UObject* Self, const NameString& eventName, uint8_t eventType, float* Value, float* Radius);
	static void AISetEventCallback(UObject* Self, const NameString& eventName, const NameString& callback, NameString* scoreCallback, BitfieldBool* bCheckVisibility, BitfieldBool* bCheckDir, BitfieldBool* bCheckCylinder, BitfieldBool* bCheckLOS);
	static void AIStartEvent(UObject* Self, const NameString& eventName, uint8_t eventType, float* Value, float* Radius);
	static void AIVisibility(UObject* Self, BitfieldBool* bIncludeVelocity, float& ReturnValue);
	static void TraceTexture(UObject* Self, UObject* BaseClass, UObject*& Actor, NameString& texName, NameString& texGroup, int& flags, vec3& HitLoc, vec3& HitNorm, const vec3& End, vec3* Start, vec3* Extent);
	static void TraceVisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, vec3* Start, vec3* Extent);
};
