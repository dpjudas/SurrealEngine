#pragma once

#include "UObject/UObject.h"

class NActor
{
public:
	static void RegisterFunctions();

	static void Add_ColorColor(const Color& A, const Color& B, Color& ReturnValue);
	static void AllActors(UObject* Self, UObject* BaseClass, UObject*& Actor, std::optional<NameString> MatchTag);
	static void AllActors_U227(UObject* Self, UObject* BaseClass, UObject*& Actor, std::optional<NameString> MatchTag, std::optional<NameString> MatchEvent);
	static void AllActors_U227k(UObject* Self, UObject* BaseClass, UObject*& Actor, std::optional<NameString> MatchTag, std::optional<NameString> MatchEvent, bool bAllLevels);
	static void AutonomousPhysics(UObject* Self, float DeltaSeconds);
	static void BasedActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void ChildActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue);
	static void CycleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, int& index);
	static void DemoPlaySound(UObject* Self, UObject* Sound, std::optional<uint8_t> Slot, std::optional<float> Volume, std::optional<bool> bNoOverride, std::optional<float> Radius, std::optional<float> Pitch);
	static void Destroy(UObject* Self, BitfieldBool& ReturnValue);
	static void Error(UObject* Self, const std::string& S);
	static void FastTrace(UObject* Self, const vec3& TraceEnd, std::optional<vec3> TraceStart, BitfieldBool& ReturnValue);
	static void FinishAnim(UObject* Self);
	static void FinishInterpolation(UObject* Self);
	static void GetAnimGroup(UObject* Self, const NameString& Sequence, NameString& ReturnValue);
	static void GetBoundingBox(UObject* Self, vec3& MinVect, vec3& MaxVect, std::optional<bool> bExact, std::optional<vec3> testLocation, std::optional<Rotator> testRotation, BitfieldBool& ReturnValue);
	static void GetCacheEntry(UObject* Self, int Num, std::string& Guid, std::string& Filename, BitfieldBool& ReturnValue);
	static void GetMapName(UObject* Self, const std::string& NameEnding, const std::string& MapName, int Dir, std::string& ReturnValue);
	static void GetMeshTexture(UObject* Self, std::optional<int> texnum, UObject*& ReturnValue);
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
	static void LoopAnim(UObject* Self, const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<float> MinRate);
	static void MakeNoise(UObject* Self, float Loudness);
	static void Move(UObject* Self, const vec3& Delta, BitfieldBool& ReturnValue);
	static void MoveCacheEntry(UObject* Self, const std::string& Guid, std::optional<std::string> NewFilename, BitfieldBool& ReturnValue);
	static void MoveSmooth(UObject* Self, const vec3& Delta, BitfieldBool& ReturnValue);
	static void Multiply_ColorFloat(const Color& A, float B, Color& ReturnValue);
	static void Multiply_FloatColor(float A, const Color& B, Color& ReturnValue);
	static void PlayAnim(UObject* Self, const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime);
	static void PlayBlendAnim(UObject* Self, const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<uint8_t> BlendSlot);
	static void PlayOwnedSound(UObject* Self, UObject* Sound, std::optional<uint8_t> Slot, std::optional<float> Volume, std::optional<bool> bNoOverride, std::optional<float> Radius, std::optional<float> Pitch);
	static void PlaySound(UObject* Self, UObject* Sound, std::optional<uint8_t> Slot, std::optional<float> Volume, std::optional<bool> bNoOverride, std::optional<float> Radius, std::optional<float> Pitch);
	static void PlayerCanSeeMe(UObject* Self, BitfieldBool& ReturnValue);
	static void RadiusActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float Radius, std::optional<vec3> Loc);
	static void SetBase(UObject* Self, UObject* NewBase);
	static void SetCollision(UObject* Self, std::optional<bool> NewColActors, std::optional<bool> NewBlockActors, std::optional<bool> NewBlockPlayers);
	static void SetCollisionSize(UObject* Self, float NewRadius, float NewHeight, BitfieldBool& ReturnValue);
	static void SetLocation(UObject* Self, const vec3& NewLocation, BitfieldBool& ReturnValue);
	static void SetOwner(UObject* Self, UObject* NewOwner);
	static void SetPhysics(UObject* Self, uint8_t newPhysics);
	static void SetRotation(UObject* Self, const Rotator& NewRotation, BitfieldBool& ReturnValue);
	static void SetTimer(UObject* Self, float NewTimerRate, bool bLoop);
	static void Sleep(UObject* Self, float Seconds);
	static void Spawn(UObject* Self, UObject* SpawnClass, std::optional<UObject*> SpawnOwner, std::optional<NameString> SpawnTag, std::optional<vec3> SpawnLocation, std::optional<Rotator> SpawnRotation, UObject*& ReturnValue);
	static void Subtract_ColorColor(const Color& A, const Color& B, Color& ReturnValue);
	static void TouchingActors(UObject* Self, UObject* BaseClass, UObject*& Actor);
	static void Trace(UObject* Self, vec3& HitLocation, vec3& HitNormal, const vec3& TraceEnd, std::optional<vec3> TraceStart, std::optional<bool> bTraceActors, std::optional<vec3> Extent, UObject*& ReturnValue);
	static void TraceActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, std::optional<vec3> Start, std::optional<vec3> Extent);
	static void TweenAnim(UObject* Self, const NameString& Sequence, float Time);
	static void VisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, std::optional<float> Radius, std::optional<vec3> Loc);
	static void VisibleCollidingActors(UObject* Self, UObject* BaseClass, UObject*& Actor, std::optional<float> Radius, std::optional<vec3> Loc, std::optional<bool> bIgnoreHidden);
	static void VisibleCollidingActors_219(UObject* Self, UObject* BaseClass, UObject*& Actor, std::optional<float> Radius, std::optional<vec3> Loc);
	static void GetPlayerPawn(UObject* Self, UObject*& ReturnValue);
	static void AIClearEvent(UObject* Self, const NameString& eventName);
	static void AIClearEventCallback(UObject* Self, const NameString& eventName);
	static void AIEndEvent(UObject* Self, const NameString& eventName, uint8_t eventType);
	static void AIGetLightLevel(UObject* Self, const vec3& Location, float& ReturnValue);
	static void AISendEvent(UObject* Self, const NameString& eventName, uint8_t eventType, std::optional<float> Value, std::optional<float> Radius);
	static void AISetEventCallback(UObject* Self, const NameString& eventName, const NameString& callback, std::optional<NameString> scoreCallback, std::optional<bool> bCheckVisibility, std::optional<bool> bCheckDir, std::optional<bool> bCheckCylinder, std::optional<bool> bCheckLOS);
	static void AIStartEvent(UObject* Self, const NameString& eventName, uint8_t eventType, std::optional<float> Value, std::optional<float> Radius);
	static void AIVisibility(UObject* Self, std::optional<bool> bIncludeVelocity, float& ReturnValue);
	static void TraceTexture(UObject* Self, UObject* BaseClass, UObject*& Actor, NameString& texName, NameString& texGroup, int& flags, vec3& HitLoc, vec3& HitNorm, const vec3& End, std::optional<vec3> Start, std::optional<vec3> Extent);
	static void TraceVisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, std::optional<vec3> Start, std::optional<vec3> Extent);

	static void InStasis(UObject* Self, BitfieldBool& ReturnValue);
	static void ParabolicTrace(UObject* Self, vec3& finalLocation, std::optional<vec3> startVelocity, std::optional<vec3> startLocation, std::optional<bool> bCheckActors, std::optional<vec3> Cylinder, std::optional<float> maxTime, std::optional<float> elasticity, std::optional<bool> bBounce, std::optional<float> landingSpeed, std::optional<float> granularity, float& ReturnValue);
	static void PlaySound_Deus(UObject* Self, UObject* Sound, std::optional<uint8_t> Slot, std::optional<float> Volume, std::optional<bool> bNoOverride, std::optional<float> Radius, std::optional<float> Pitch, int& ReturnValue);
	static void RandomBiasedRotation(UObject* Self, int centralYaw, float yawDistribution, int centralPitch, float pitchDistribution, Rotator& ReturnValue);
	static void SetInstantMusicVolume(UObject* Self, uint8_t newMusicVolume);
	static void SetInstantSoundVolume(UObject* Self, uint8_t newSoundVolume);
	static void SetInstantSpeechVolume(UObject* Self, uint8_t newSpeechVolume);
	static void SetPhysics_Deus(UObject* Self, uint8_t newPhysics, std::optional<UObject*> newFloor);
	static void StopSound(UObject* Self, int Id);
	static void TweenBlendAnim(UObject* Self, const NameString& Sequence, float Time, std::optional<int> BlendSlot);
};
