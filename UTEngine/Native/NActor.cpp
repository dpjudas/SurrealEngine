
#include "Precomp.h"
#include "NActor.h"
#include "VM/NativeFunc.h"

void NActor::RegisterFunctions()
{
	RegisterVMNativeFunc_3("Actor", "Add_ColorColor", &NActor::Add_ColorColor, 551);
	RegisterVMNativeFunc_3("Actor", "AllActors", &NActor::AllActors, 304);
	RegisterVMNativeFunc_1("Actor", "AutonomousPhysics", &NActor::AutonomousPhysics, 3971);
	RegisterVMNativeFunc_2("Actor", "BasedActors", &NActor::BasedActors, 306);
	RegisterVMNativeFunc_2("Actor", "ChildActors", &NActor::ChildActors, 305);
	RegisterVMNativeFunc_2("Actor", "ConsoleCommand", &NActor::ConsoleCommand, 0);
	RegisterVMNativeFunc_6("Actor", "DemoPlaySound", &NActor::DemoPlaySound, 0);
	RegisterVMNativeFunc_1("Actor", "Destroy", &NActor::Destroy, 279);
	RegisterVMNativeFunc_1("Actor", "Error", &NActor::Error, 233);
	RegisterVMNativeFunc_3("Actor", "FastTrace", &NActor::FastTrace, 548);
	RegisterVMNativeFunc_0("Actor", "FinishAnim", &NActor::FinishAnim, 261);
	RegisterVMNativeFunc_0("Actor", "FinishInterpolation", &NActor::FinishInterpolation, 301);
	RegisterVMNativeFunc_2("Actor", "GetAnimGroup", &NActor::GetAnimGroup, 293);
	RegisterVMNativeFunc_4("Actor", "GetCacheEntry", &NActor::GetCacheEntry, 0);
	RegisterVMNativeFunc_4("Actor", "GetMapName", &NActor::GetMapName, 539);
	RegisterVMNativeFunc_3("Actor", "GetNextInt", &NActor::GetNextInt, 0);
	RegisterVMNativeFunc_4("Actor", "GetNextIntDesc", &NActor::GetNextIntDesc, 0);
	RegisterVMNativeFunc_5("Actor", "GetNextSkin", &NActor::GetNextSkin, 545);
	RegisterVMNativeFunc_2("Actor", "GetSoundDuration", &NActor::GetSoundDuration, 0);
	RegisterVMNativeFunc_1("Actor", "GetURLMap", &NActor::GetURLMap, 547);
	RegisterVMNativeFunc_2("Actor", "HasAnim", &NActor::HasAnim, 263);
	RegisterVMNativeFunc_1("Actor", "IsAnimating", &NActor::IsAnimating, 282);
	RegisterVMNativeFunc_1("Actor", "LinkSkelAnim", &NActor::LinkSkelAnim, 0);
	RegisterVMNativeFunc_4("Actor", "LoopAnim", &NActor::LoopAnim, 260);
	RegisterVMNativeFunc_1("Actor", "MakeNoise", &NActor::MakeNoise, 512);
	RegisterVMNativeFunc_2("Actor", "Move", &NActor::Move, 266);
	RegisterVMNativeFunc_3("Actor", "MoveCacheEntry", &NActor::MoveCacheEntry, 0);
	RegisterVMNativeFunc_2("Actor", "MoveSmooth", &NActor::MoveSmooth, 3969);
	RegisterVMNativeFunc_3("Actor", "Multiply_ColorFloat", &NActor::Multiply_ColorFloat, 552);
	RegisterVMNativeFunc_3("Actor", "Multiply_FloatColor", &NActor::Multiply_FloatColor, 550);
	RegisterVMNativeFunc_3("Actor", "PlayAnim", &NActor::PlayAnim, 259);
	RegisterVMNativeFunc_6("Actor", "PlayOwnedSound", &NActor::PlayOwnedSound, 0);
	RegisterVMNativeFunc_6("Actor", "PlaySound", &NActor::PlaySound, 264);
	RegisterVMNativeFunc_1("Actor", "PlayerCanSeeMe", &NActor::PlayerCanSeeMe, 532);
	RegisterVMNativeFunc_4("Actor", "RadiusActors", &NActor::RadiusActors, 310);
	RegisterVMNativeFunc_1("Actor", "SetBase", &NActor::SetBase, 298);
	RegisterVMNativeFunc_3("Actor", "SetCollision", &NActor::SetCollision, 262);
	RegisterVMNativeFunc_3("Actor", "SetCollisionSize", &NActor::SetCollisionSize, 283);
	RegisterVMNativeFunc_2("Actor", "SetLocation", &NActor::SetLocation, 267);
	RegisterVMNativeFunc_1("Actor", "SetOwner", &NActor::SetOwner, 272);
	RegisterVMNativeFunc_1("Actor", "SetPhysics", &NActor::SetPhysics, 3970);
	RegisterVMNativeFunc_2("Actor", "SetRotation", &NActor::SetRotation, 299);
	RegisterVMNativeFunc_2("Actor", "SetTimer", &NActor::SetTimer, 280);
	RegisterVMNativeFunc_1("Actor", "Sleep", &NActor::Sleep, 256);
	RegisterVMNativeFunc_6("Actor", "Spawn", &NActor::Spawn, 278);
	RegisterVMNativeFunc_3("Actor", "Subtract_ColorColor", &NActor::Subtract_ColorColor, 549);
	RegisterVMNativeFunc_2("Actor", "TouchingActors", &NActor::TouchingActors, 307);
	RegisterVMNativeFunc_7("Actor", "Trace", &NActor::Trace, 277);
	RegisterVMNativeFunc_7("Actor", "TraceActors", &NActor::TraceActors, 309);
	RegisterVMNativeFunc_2("Actor", "TweenAnim", &NActor::TweenAnim, 294);
	RegisterVMNativeFunc_4("Actor", "VisibleActors", &NActor::VisibleActors, 311);
	RegisterVMNativeFunc_5("Actor", "VisibleCollidingActors", &NActor::VisibleCollidingActors, 312);
}

void NActor::Add_ColorColor(const Color& A, const Color& B, Color& ReturnValue)
{
	throw std::runtime_error("Actor.Add_ColorColor not implemented");
}

void NActor::AllActors(UObject* Self, UObject* BaseClass, UObject*& Actor, std::string* MatchTag)
{
	throw std::runtime_error("Actor.AllActors not implemented");
}

void NActor::AutonomousPhysics(UObject* Self, float DeltaSeconds)
{
	throw std::runtime_error("Actor.AutonomousPhysics not implemented");
}

void NActor::BasedActors(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	throw std::runtime_error("Actor.BasedActors not implemented");
}

void NActor::ChildActors(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	throw std::runtime_error("Actor.ChildActors not implemented");
}

void NActor::ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue)
{
	throw std::runtime_error("Actor.ConsoleCommand not implemented");
}

void NActor::DemoPlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch)
{
	throw std::runtime_error("Actor.DemoPlaySound not implemented");
}

void NActor::Destroy(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("Actor.Destroy not implemented");
}

void NActor::Error(UObject* Self, const std::string& S)
{
	throw std::runtime_error("Actor.Error not implemented");
}

void NActor::FastTrace(UObject* Self, const vec3& TraceEnd, vec3* TraceStart, bool& ReturnValue)
{
	throw std::runtime_error("Actor.FastTrace not implemented");
}

void NActor::FinishAnim(UObject* Self)
{
	throw std::runtime_error("Actor.FinishAnim not implemented");
}

void NActor::FinishInterpolation(UObject* Self)
{
	throw std::runtime_error("Actor.FinishInterpolation not implemented");
}

void NActor::GetAnimGroup(UObject* Self, const std::string& Sequence, std::string& ReturnValue)
{
	throw std::runtime_error("Actor.GetAnimGroup not implemented");
}

void NActor::GetCacheEntry(UObject* Self, int Num, std::string& Guid, std::string& Filename, bool& ReturnValue)
{
	throw std::runtime_error("Actor.GetCacheEntry not implemented");
}

void NActor::GetMapName(UObject* Self, const std::string& NameEnding, const std::string& MapName, int Dir, std::string& ReturnValue)
{
	throw std::runtime_error("Actor.GetMapName not implemented");
}

void NActor::GetNextInt(UObject* Self, const std::string& ClassName, int Num, std::string& ReturnValue)
{
	throw std::runtime_error("Actor.GetNextInt not implemented");
}

void NActor::GetNextIntDesc(UObject* Self, const std::string& ClassName, int Num, std::string& Entry, std::string& Description)
{
	throw std::runtime_error("Actor.GetNextIntDesc not implemented");
}

void NActor::GetNextSkin(UObject* Self, const std::string& Prefix, const std::string& CurrentSkin, int Dir, std::string& SkinName, std::string& SkinDesc)
{
	throw std::runtime_error("Actor.GetNextSkin not implemented");
}

void NActor::GetSoundDuration(UObject* Self, UObject* Sound, float& ReturnValue)
{
	throw std::runtime_error("Actor.GetSoundDuration not implemented");
}

void NActor::GetURLMap(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("Actor.GetURLMap not implemented");
}

void NActor::HasAnim(UObject* Self, const std::string& Sequence, bool& ReturnValue)
{
	throw std::runtime_error("Actor.HasAnim not implemented");
}

void NActor::IsAnimating(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("Actor.IsAnimating not implemented");
}

void NActor::LinkSkelAnim(UObject* Self, UObject* Anim)
{
	throw std::runtime_error("Actor.LinkSkelAnim not implemented");
}

void NActor::LoopAnim(UObject* Self, const std::string& Sequence, float* Rate, float* TweenTime, float* MinRate)
{
	throw std::runtime_error("Actor.LoopAnim not implemented");
}

void NActor::MakeNoise(UObject* Self, float Loudness)
{
	throw std::runtime_error("Actor.MakeNoise not implemented");
}

void NActor::Move(UObject* Self, const vec3& Delta, bool& ReturnValue)
{
	throw std::runtime_error("Actor.Move not implemented");
}

void NActor::MoveCacheEntry(UObject* Self, const std::string& Guid, std::string* NewFilename, bool& ReturnValue)
{
	throw std::runtime_error("Actor.MoveCacheEntry not implemented");
}

void NActor::MoveSmooth(UObject* Self, const vec3& Delta, bool& ReturnValue)
{
	throw std::runtime_error("Actor.MoveSmooth not implemented");
}

void NActor::Multiply_ColorFloat(const Color& A, float B, Color& ReturnValue)
{
	throw std::runtime_error("Actor.Multiply_ColorFloat not implemented");
}

void NActor::Multiply_FloatColor(float A, const Color& B, Color& ReturnValue)
{
	throw std::runtime_error("Actor.Multiply_FloatColor not implemented");
}

void NActor::PlayAnim(UObject* Self, const std::string& Sequence, float* Rate, float* TweenTime)
{
	throw std::runtime_error("Actor.PlayAnim not implemented");
}

void NActor::PlayOwnedSound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch)
{
	throw std::runtime_error("Actor.PlayOwnedSound not implemented");
}

void NActor::PlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch)
{
	throw std::runtime_error("Actor.PlaySound not implemented");
}

void NActor::PlayerCanSeeMe(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("Actor.PlayerCanSeeMe not implemented");
}

void NActor::RadiusActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float Radius, vec3* Loc)
{
	throw std::runtime_error("Actor.RadiusActors not implemented");
}

void NActor::SetBase(UObject* Self, UObject* NewBase)
{
	throw std::runtime_error("Actor.SetBase not implemented");
}

void NActor::SetCollision(UObject* Self, bool* NewColActors, bool* NewBlockActors, bool* NewBlockPlayers)
{
	throw std::runtime_error("Actor.SetCollision not implemented");
}

void NActor::SetCollisionSize(UObject* Self, float NewRadius, float NewHeight, bool& ReturnValue)
{
	throw std::runtime_error("Actor.SetCollisionSize not implemented");
}

void NActor::SetLocation(UObject* Self, const vec3& NewLocation, bool& ReturnValue)
{
	throw std::runtime_error("Actor.SetLocation not implemented");
}

void NActor::SetOwner(UObject* Self, UObject* NewOwner)
{
	throw std::runtime_error("Actor.SetOwner not implemented");
}

void NActor::SetPhysics(UObject* Self, uint8_t newPhysics)
{
	throw std::runtime_error("Actor.SetPhysics not implemented");
}

void NActor::SetRotation(UObject* Self, const Rotator& NewRotation, bool& ReturnValue)
{
	throw std::runtime_error("Actor.SetRotation not implemented");
}

void NActor::SetTimer(UObject* Self, float NewTimerRate, bool bLoop)
{
	throw std::runtime_error("Actor.SetTimer not implemented");
}

void NActor::Sleep(UObject* Self, float Seconds)
{
	throw std::runtime_error("Actor.Sleep not implemented");
}

void NActor::Spawn(UObject* Self, UObject* SpawnClass, UObject** SpawnOwner, std::string* SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation, UObject*& ReturnValue)
{
	throw std::runtime_error("Actor.Spawn not implemented");
}

void NActor::Subtract_ColorColor(const Color& A, const Color& B, Color& ReturnValue)
{
	throw std::runtime_error("Actor.Subtract_ColorColor not implemented");
}

void NActor::TouchingActors(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	throw std::runtime_error("Actor.TouchingActors not implemented");
}

void NActor::Trace(UObject* Self, vec3& HitLocation, vec3& HitNormal, const vec3& TraceEnd, vec3* TraceStart, bool* bTraceActors, vec3* Extent, UObject*& ReturnValue)
{
	throw std::runtime_error("Actor.Trace not implemented");
}

void NActor::TraceActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, vec3* Start, vec3* Extent)
{
	throw std::runtime_error("Actor.TraceActors not implemented");
}

void NActor::TweenAnim(UObject* Self, const std::string& Sequence, float Time)
{
	throw std::runtime_error("Actor.TweenAnim not implemented");
}

void NActor::VisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc)
{
	throw std::runtime_error("Actor.VisibleActors not implemented");
}

void NActor::VisibleCollidingActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc, bool* bIgnoreHidden)
{
	throw std::runtime_error("Actor.VisibleCollidingActors not implemented");
}
