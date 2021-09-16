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
	static void DemoPlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch);
	static void Destroy(UObject* Self, bool& ReturnValue);
	static void Error(UObject* Self, const std::string& S);
	static void FastTrace(UObject* Self, const vec3& TraceEnd, vec3* TraceStart, bool& ReturnValue);
	static void FinishAnim(UObject* Self);
	static void FinishInterpolation(UObject* Self);
	static void GetAnimGroup(UObject* Self, const NameString& Sequence, NameString& ReturnValue);
	static void GetCacheEntry(UObject* Self, int Num, std::string& Guid, std::string& Filename, bool& ReturnValue);
	static void GetMapName(UObject* Self, const std::string& NameEnding, const std::string& MapName, int Dir, std::string& ReturnValue);
	static void GetNextInt(UObject* Self, const std::string& ClassName, int Num, std::string& ReturnValue);
	static void GetNextIntDesc(UObject* Self, const std::string& ClassName, int Num, std::string& Entry, std::string& Description);
	static void GetNextSkin(UObject* Self, const std::string& Prefix, const std::string& CurrentSkin, int Dir, std::string& SkinName, std::string& SkinDesc);
	static void GetSoundDuration(UObject* Self, UObject* Sound, float& ReturnValue);
	static void GetURLMap(UObject* Self, std::string& ReturnValue);
	static void HasAnim(UObject* Self, const NameString& Sequence, bool& ReturnValue);
	static void IsAnimating(UObject* Self, bool& ReturnValue);
	static void LinkSkelAnim(UObject* Self, UObject* Anim);
	static void LoopAnim(UObject* Self, const NameString& Sequence, float* Rate, float* TweenTime, float* MinRate);
	static void MakeNoise(UObject* Self, float Loudness);
	static void Move(UObject* Self, const vec3& Delta, bool& ReturnValue);
	static void MoveCacheEntry(UObject* Self, const std::string& Guid, std::string* NewFilename, bool& ReturnValue);
	static void MoveSmooth(UObject* Self, const vec3& Delta, bool& ReturnValue);
	static void Multiply_ColorFloat(const Color& A, float B, Color& ReturnValue);
	static void Multiply_FloatColor(float A, const Color& B, Color& ReturnValue);
	static void PlayAnim(UObject* Self, const NameString& Sequence, float* Rate, float* TweenTime);
	static void PlayOwnedSound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch);
	static void PlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch);
	static void PlayerCanSeeMe(UObject* Self, bool& ReturnValue);
	static void RadiusActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float Radius, vec3* Loc);
	static void SetBase(UObject* Self, UObject* NewBase);
	static void SetCollision(UObject* Self, bool* NewColActors, bool* NewBlockActors, bool* NewBlockPlayers);
	static void SetCollisionSize(UObject* Self, float NewRadius, float NewHeight, bool& ReturnValue);
	static void SetLocation(UObject* Self, const vec3& NewLocation, bool& ReturnValue);
	static void SetOwner(UObject* Self, UObject* NewOwner);
	static void SetPhysics(UObject* Self, uint8_t newPhysics);
	static void SetRotation(UObject* Self, const Rotator& NewRotation, bool& ReturnValue);
	static void SetTimer(UObject* Self, float NewTimerRate, bool bLoop);
	static void Sleep(UObject* Self, float Seconds);
	static void Spawn(UObject* Self, UObject* SpawnClass, UObject** SpawnOwner, NameString* SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation, UObject*& ReturnValue);
	static void Subtract_ColorColor(const Color& A, const Color& B, Color& ReturnValue);
	static void TouchingActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void Trace(UObject* Self, vec3& HitLocation, vec3& HitNormal, const vec3& TraceEnd, vec3* TraceStart, bool* bTraceActors, vec3* Extent, UObject*& ReturnValue);
	static void TraceActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, vec3* Start, vec3* Extent);
	static void TweenAnim(UObject* Self, const NameString& Sequence, float Time);
	static void VisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc);
	static void VisibleCollidingActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc, bool* bIgnoreHidden);
};
