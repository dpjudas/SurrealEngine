#pragma once

#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"

class UCanvas;
class UFlagBase;
class URootWindow;

struct ActorRef
{
	UActor* Actor = nullptr;
	int RefCount = 0;
};

class UPlayerPawnExt : public UPlayerPawn
{
public:
	using UPlayerPawn::UPlayerPawn;

	void InitRootWindow();
	void PreRenderWindows(UCanvas* canvas);
	void PostRenderWindows(UCanvas* canvas);

	UFlagBase*& FlagBase() { return Value<UFlagBase*>(PropOffsets_PlayerPawnExt.FlagBase); }
	URootWindow*& RootWindow() { return Value<URootWindow*>(PropOffsets_PlayerPawnExt.RootWindow); }
	int& actorCount() { return Value<int>(PropOffsets_PlayerPawnExt.actorCount); }
	FixedArrayView<ActorRef, 32> actorList() { return FixedArray<ActorRef, 32>(PropOffsets_PlayerPawnExt.actorList); }
};
