
#include "Precomp.h"
#include "ULevel.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/NavigationPoint/UNavigationPoint.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Engine.h"
#include "VM/ScriptCall.h"

ULevel::ULevel(NameString name, UClass* base, ObjectFlags flags) : ULevelBase(name, base, flags)
{
	Collision.SetLevel(this);
	Light.SetLevel(this);
}

void ULevel::Load(ObjectStream* stream)
{
	ULevelBase::Load(stream);

	Model = stream->ReadObject<UModel>();

	if (Model)
		Model->LoadNow();

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LevelReachSpec spec;
		spec.distance = stream->ReadInt32();
		spec.startActor = stream->ReadObject<UNavigationPoint>();
		spec.endActor = stream->ReadObject<UNavigationPoint>();
		spec.collisionRadius = stream->ReadInt32();
		spec.collisionHeight = stream->ReadInt32();
		spec.reachFlags = stream->ReadInt32();
		spec.bPruned = stream->ReadInt8();
		ReachSpecs.push_back(spec);
	}
}

void ULevel::Save(PackageStreamWriter* stream)
{
	ULevelBase::Save(stream);
	stream->WriteObject(Model);
	stream->WriteIndex((int)ReachSpecs.size());
	for (const LevelReachSpec& spec : ReachSpecs)
	{
		stream->WriteInt32(spec.distance);
		stream->WriteObject(spec.startActor);
		stream->WriteObject(spec.endActor);
		stream->WriteInt32(spec.collisionRadius);
		stream->WriteInt32(spec.collisionHeight);
		stream->WriteInt32(spec.reachFlags);
		stream->WriteInt8(spec.bPruned);
	}
}

void ULevel::TickActor(float elapsed, UActor* actor)
{
	if (!actor)
		return;

	// If we have an owner, tick it first
	if (!actor->bDeleteMe() && actor->Owner())
	{
		TickActor(elapsed, actor->Owner());
	}

	// Do we have an actor? is it deleted? did it already tick?
	if (actor->bDeleteMe() || actor->bTicked() == ticked)
		return;

	// Mark actor as ticked
	actor->bTicked() = ticked;

	// Tick the actor for this turn
	actor->Tick(elapsed);

	// Destroy the actor if its time
	if (actor->Role() >= ROLE_SimulatedProxy && actor->LifeSpan() != 0.0f)
	{
		actor->LifeSpan() = std::max(actor->LifeSpan() - elapsed, 0.0f);
		if (actor->LifeSpan() == 0.0f)
		{
			CallEvent(actor, EventName::Expired);
			actor->Destroy();
		}
	}
}

void ULevel::Tick(float elapsed, bool gamePaused)
{
	if (gamePaused)
	{
		for (size_t i = 0; i < Actors.size(); i++)
		{
			if (auto playerPawn = UObject::TryCast<UPlayerPawn>(Actors[i]))
				playerPawn->PausedInput(elapsed);
			else if (Actors[i] && Actors[i]->bAlwaysTick()) // Should this happen?
				TickActor(elapsed, Actors[i]);
		}
	}
	else if (engine->LevelInfo->bPlayersOnly())
	{
		for (size_t i = 0; i < Actors.size(); i++)
		{
			if (UObject::TryCast<UPlayerPawn>(Actors[i]) || Actors[i]->bAlwaysTick())
				TickActor(elapsed, Actors[i]);
		}
	}
	else
	{
		for (size_t i = 0; i < Actors.size(); i++)
		{
			if (Actors[i])
				TickActor(elapsed, Actors[i]);
		}
	}

	Array<UActor*> newActorList;
	newActorList.reserve(Actors.size());
	for (UActor* actor : Actors)
	{
		if (actor)
		{
			actor->Index = (int)newActorList.size();
			newActorList.push_back(actor);
		}
	}
	Actors.swap(newActorList);

	ticked = !ticked;
}
