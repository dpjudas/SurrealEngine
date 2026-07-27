#pragma once

#include "Packages/Engine/Actors/Info/UInfo.h"

class UEmitterGarbageCollector : public UInfo
{
public:
	using UInfo::UInfo;

	BitfieldBool bCleanUp() { return BoolValue(PropOffsets_EmitterGarbageCollector.bCleanUp); }
	float& CleanUpTime() { return Value<float>(PropOffsets_EmitterGarbageCollector.CleanUpTime); }
	void*& GarbagePtr() { return Value<void*>(PropOffsets_EmitterGarbageCollector.GarbagePtr); }
};
