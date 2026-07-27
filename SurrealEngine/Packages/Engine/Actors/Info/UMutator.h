#pragma once

#include "UInfo.h"

class UMutator : public UInfo
{
public:
	using UInfo::UInfo;

	UClass*& DefaultWeapon() { return Value<UClass*>(PropOffsets_Mutator.DefaultWeapon); }
	UMutator*& NextDamageMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextDamageMutator); }
	UMutator*& NextHUDMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextHUDMutator); }
	UMutator*& NextMessageMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextMessageMutator); }
	UMutator*& NextMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextMutator); }
	BitfieldBool bHUDMutator() { return BoolValue(PropOffsets_Mutator.bHUDMutator); }
};
