#pragma once

#include "UActor.h"

class USpawnNotify : public UActor
{
public:
	using UActor::UActor;

	UClass*& ActorClass() { return Value<UClass*>(PropOffsets_SpawnNotify.ActorClass); }
	USpawnNotify*& Next() { return Value<USpawnNotify*>(PropOffsets_SpawnNotify.Next); }
};
