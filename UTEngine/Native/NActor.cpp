
#include "Precomp.h"
#include "NActor.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "UObject/USound.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Package/PackageManager.h"
#include "Engine.h"
#include "Audio/AudioMixer.h"
#include "Collision.h"

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
	Color c;
	c.R = std::min(A.R + (int)B.R, 255);
	c.G = std::min(A.G + (int)B.G, 255);
	c.B = std::min(A.B + (int)B.B, 255);
	c.A = std::min(A.A + (int)B.A, 255);
	ReturnValue = c;
}

void NActor::AllActors(UObject* Self, UObject* BaseClass, UObject*& Actor, std::string* MatchTag)
{
	Frame::CreatedIterator = std::make_unique<AllObjectsIterator>(BaseClass, &Actor, MatchTag ? *MatchTag : std::string());
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
	// "Execute a console command in the context of the current level and game engine"

	bool found;
	ReturnValue = engine->ConsoleCommand(Self, Command, found);
}

void NActor::DemoPlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch)
{
	throw std::runtime_error("Actor.DemoPlaySound not implemented");
}

void NActor::Destroy(UObject* Self, bool& ReturnValue)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	ULevel* level = SelfActor->XLevel();
	auto it = std::find(level->Actors.begin(), level->Actors.end(), SelfActor);
	if (it != level->Actors.end())
		level->Actors.erase(it);
}

void NActor::Error(UObject* Self, const std::string& S)
{
	throw std::runtime_error("Actor.Error not implemented");
}

void NActor::FastTrace(UObject* Self, const vec3& TraceEnd, vec3* TraceStart, bool& ReturnValue)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	vec3 start = TraceStart ? *TraceStart : SelfActor->Location();
	ReturnValue = SelfActor->FastTrace(start, TraceEnd);
}

void NActor::FinishAnim(UObject* Self)
{
	engine->LogUnimplemented("FinishAnim");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::FinishAnim;
}

void NActor::FinishInterpolation(UObject* Self)
{
	engine->LogUnimplemented("FinishInterpolation");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::FinishInterpolation;
}

void NActor::GetAnimGroup(UObject* Self, const std::string& Sequence, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->GetAnimGroup(Sequence);
}

void NActor::GetCacheEntry(UObject* Self, int Num, std::string& Guid, std::string& Filename, bool& ReturnValue)
{
	throw std::runtime_error("Actor.GetCacheEntry not implemented");
}

void NActor::GetNextInt(UObject* Self, const std::string& ClassName, int Num, std::string& ReturnValue)
{
	std::vector<IntObject>& objects = engine->packages->GetIntObjects(ClassName);
	if (Num >= 0 && (size_t)Num < objects.size())
		ReturnValue = objects[Num].Name;
	else
		ReturnValue = {};
}

void NActor::GetNextIntDesc(UObject* Self, const std::string& ClassName, int Num, std::string& Entry, std::string& Description)
{
	std::vector<IntObject>& objects = engine->packages->GetIntObjects(ClassName);
	if (Num >= 0 && (size_t)Num < objects.size())
	{
		Entry = objects[Num].Name;
		Description = objects[Num].Description;
	}
	else
	{
		Entry = {};
		Description = {};
	}
}

void NActor::GetMapName(UObject* Self, const std::string& NameEnding, const std::string& MapName, int Dir, std::string& ReturnValue)
{
	std::vector<std::string> maps;

	// Filter list to only those with the matching map type
	for (const std::string& name : engine->packages->GetMaps())
	{
		if (name.size() >= NameEnding.size() && name.substr(0, NameEnding.size()) == NameEnding)
			maps.push_back(name);
	}

	// Find the previous or next map relative to the MapName specified
	if (!MapName.empty() && MapName != "None")
	{
		auto it = std::find(maps.begin(), maps.end(), MapName);
		if (it != maps.end())
		{
			int index = (int)std::distance(maps.begin(), it) + Dir;
			if (index < 0)
				index = (int)maps.size() - 1;
			else if (index >= (int)maps.size())
				index = 0;
			ReturnValue = maps[index];
			return;
		}
	}

	// Grab first map if map wasn't found or none was specified
	ReturnValue = !maps.empty() ? maps.front() : std::string();
}

void NActor::GetNextSkin(UObject* Self, const std::string& Prefix, const std::string& CurrentSkin, int Dir, std::string& SkinName, std::string& SkinDesc)
{
	// To do: is this even right? Why does the skin name have Mesh/ in front of it? Are we doing a workaround for a bug somewhere else?
	std::string prefix;
	size_t slashpos = Prefix.find_last_of('/');
	if (slashpos != std::string::npos)
		prefix = Prefix.substr(slashpos + 1);
	else
		prefix = Prefix;

	std::vector<const IntObject*> skins;

	// Filter list to only those with the matching skin prefix
	for (const IntObject& skin : engine->packages->GetIntObjects("Texture"))
	{
		if (skin.Name.size() >= prefix.size() && skin.Name.substr(0, prefix.size()) == prefix)
			skins.push_back(&skin);
	}

	// Find the previous or next skin relative to the CurrentSkin specified
	if (!CurrentSkin.empty() && CurrentSkin != "None")
	{
		auto it = std::find_if(skins.begin(), skins.end(), [&](auto skin) { return skin->Name == CurrentSkin; });
		if (it != skins.end())
		{
			int index = (int)std::distance(skins.begin(), it) + Dir;
			if (index < 0)
				index = (int)skins.size() - 1;
			else if (index >= (int)skins.size())
				index = 0;

			SkinName = skins[index]->Name;
			SkinDesc = skins[index]->Description;
			return;
		}
	}

	// Grab first skin if skin wasn't found or none was specified
	if (!skins.empty())
	{
		SkinName = skins.front()->Name;
		SkinDesc = skins.front()->Description;
	}
	else
	{
		SkinName = {};
		SkinDesc = {};
	}
}

void NActor::GetSoundDuration(UObject* Self, UObject* Sound, float& ReturnValue)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	USound* s = UObject::Cast<USound>(Sound);
	ReturnValue = engine->audio->GetSoundDuration(s->GetSound());
}

void NActor::GetURLMap(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->Level()->URL.Map;
}

void NActor::HasAnim(UObject* Self, const std::string& Sequence, bool& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->HasAnim(Sequence);
}

void NActor::IsAnimating(UObject* Self, bool& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->IsAnimating();
}

void NActor::LinkSkelAnim(UObject* Self, UObject* Anim)
{
	throw std::runtime_error("Actor.LinkSkelAnim not implemented");
}

void NActor::LoopAnim(UObject* Self, const std::string& Sequence, float* Rate, float* TweenTime, float* MinRate)
{
	UObject::Cast<UActor>(Self)->LoopAnim(Sequence, Rate, TweenTime, MinRate);
}

void NActor::MakeNoise(UObject* Self, float Loudness)
{
	engine->LogUnimplemented("Actor.MakeNoise(" + std::to_string(Loudness) + ")");
}

void NActor::Move(UObject* Self, const vec3& Delta, bool& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->Move(Delta);
}

void NActor::MoveCacheEntry(UObject* Self, const std::string& Guid, std::string* NewFilename, bool& ReturnValue)
{
	throw std::runtime_error("Actor.MoveCacheEntry not implemented");
}

void NActor::MoveSmooth(UObject* Self, const vec3& Delta, bool& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->MoveSmooth(Delta);
}

void NActor::Multiply_ColorFloat(const Color& A, float B, Color& ReturnValue)
{
	Color c;
	c.R = (int)std::round(clamp(A.R * B, 0.0f, 255.0f));
	c.G = (int)std::round(clamp(A.G * B, 0.0f, 255.0f));
	c.B = (int)std::round(clamp(A.B * B, 0.0f, 255.0f));
	c.A = A.A;
	ReturnValue = c;
}

void NActor::Multiply_FloatColor(float A, const Color& B, Color& ReturnValue)
{
	Color c;
	c.R = (int)std::round(clamp(A * B.R, 0.0f, 255.0f));
	c.G = (int)std::round(clamp(A * B.G, 0.0f, 255.0f));
	c.B = (int)std::round(clamp(A * B.B, 0.0f, 255.0f));
	c.A = B.A;
	ReturnValue = c;
}

void NActor::PlayAnim(UObject* Self, const std::string& Sequence, float* Rate, float* TweenTime)
{
	UObject::Cast<UActor>(Self)->PlayAnim(Sequence, Rate, TweenTime);
}

void NActor::PlayOwnedSound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch)
{
	engine->LogUnimplemented("Actor.PlayOwnedSound(" + Sound->Name + ")");
}

void NActor::PlaySound(UObject* Self, UObject* Sound, uint8_t* Slot, float* Volume, bool* bNoOverride, float* Radius, float* Pitch)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	USound* s = UObject::Cast<USound>(Sound);
	engine->audio->PlaySound(Self, Slot ? *Slot : SLOT_Misc, s->GetSound(), SelfActor->Location(), Volume ? *Volume : 1.0f, Radius ? (*Radius) * 25.0f : 0.0f, Pitch ? *Pitch : 1.0f);
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
	UObject::Cast<UActor>(Self)->SetBase(UObject::Cast<UActor>(NewBase), true);
}

void NActor::SetCollision(UObject* Self, bool* NewColActors, bool* NewBlockActors, bool* NewBlockPlayers)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	SelfActor->SetCollision(
		NewColActors ? *NewColActors : SelfActor->bCollideActors(),
		NewBlockActors ? *NewBlockActors : SelfActor->bBlockActors(),
		NewBlockPlayers ? *NewBlockPlayers : SelfActor->bBlockPlayers());
}

void NActor::SetCollisionSize(UObject* Self, float NewRadius, float NewHeight, bool& ReturnValue)
{
	throw std::runtime_error("Actor.SetCollisionSize not implemented");
}

void NActor::SetLocation(UObject* Self, const vec3& NewLocation, bool& ReturnValue)
{
	// To do: do collision test
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	SelfActor->Location() = NewLocation;
	ReturnValue = true;
}

void NActor::SetOwner(UObject* Self, UObject* NewOwner)
{
	UObject::Cast<UActor>(Self)->SetOwner(UObject::Cast<UActor>(NewOwner));
}

void NActor::SetPhysics(UObject* Self, uint8_t newPhysics)
{
	UObject::Cast<UActor>(Self)->SetPhysics(newPhysics);
}

void NActor::SetRotation(UObject* Self, const Rotator& NewRotation, bool& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->SetRotation(NewRotation);
}

void NActor::SetTimer(UObject* Self, float NewTimerRate, bool bLoop)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	SelfActor->TimerCounter() = 0.0f;
	SelfActor->TimerRate() = NewTimerRate > 0.0f ? NewTimerRate : 0.0f;
	SelfActor->bTimerLoop() = bLoop;
}

void NActor::Sleep(UObject* Self, float Seconds)
{
	engine->LogUnimplemented("Sleep(" + std::to_string(Seconds) + ")");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::Sleep;
}

void NActor::Spawn(UObject* Self, UObject* SpawnClass, UObject** SpawnOwner, std::string* SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation, UObject*& ReturnValue)
{
	ReturnValue = UObject::Cast<UActor>(Self)->Spawn(UObject::Cast<UClass>(SpawnClass), SpawnOwner ? UObject::Cast<UActor>(*SpawnOwner) : nullptr, SpawnTag ? *SpawnTag : std::string(), SpawnLocation, SpawnRotation);
}

void NActor::Subtract_ColorColor(const Color& A, const Color& B, Color& ReturnValue)
{
	Color c;
	c.R = std::max(A.R - (int)B.R, 0);
	c.G = std::max(A.G - (int)B.G, 0);
	c.B = std::max(A.B - (int)B.B, 0);
	c.A = std::max(A.A - (int)B.A, 0);
	ReturnValue = c;
}

void NActor::TouchingActors(UObject* Self, UObject* BaseClass, UObject*& Actor)
{
	throw std::runtime_error("Actor.TouchingActors not implemented");
}

void NActor::Trace(UObject* Self, vec3& HitLocation, vec3& HitNormal, const vec3& TraceEnd, vec3* TraceStart, bool* bTraceActors, vec3* Extent, UObject*& ReturnValue)
{
	UActor* SelfActor = UObject::Cast<UActor>(Self);
	ReturnValue = SelfActor->Trace(
		HitLocation, HitNormal, TraceEnd,
		TraceStart ? *TraceStart : SelfActor->Location(),
		bTraceActors ? *bTraceActors : false,
		Extent ? *Extent : vec3(SelfActor->CollisionRadius(), SelfActor->CollisionRadius(), SelfActor->CollisionHeight()));
}

void NActor::TraceActors(UObject* Self, UObject* BaseClass, UObject*& Actor, vec3& HitLoc, vec3& HitNorm, const vec3& End, vec3* Start, vec3* Extent)
{
	// To do: trace against actors, filter by class and extent, return actor + hit location + normal
	Actor = nullptr;
}

void NActor::TweenAnim(UObject* Self, const std::string& Sequence, float Time)
{
	UObject::Cast<UActor>(Self)->TweenAnim(Sequence, Time);
}

void NActor::VisibleActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc)
{
	throw std::runtime_error("Actor.VisibleActors not implemented");
}

void NActor::VisibleCollidingActors(UObject* Self, UObject* BaseClass, UObject*& Actor, float* Radius, vec3* Loc, bool* bIgnoreHidden)
{
	throw std::runtime_error("Actor.VisibleCollidingActors not implemented");
}
