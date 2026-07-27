#pragma once

#include "UConObject.h"

class UConFlagRef : public UConObject
{
public:
	using UConObject::UConObject;

	NameString& FlagName() { return UObject::Value<NameString>(PropOffsets_ConFlagRef.FlagName); }
	BitfieldBool Value() { return UObject::BoolValue(PropOffsets_ConFlagRef.Value); }
	int& expiration() { return UObject::Value<int>(PropOffsets_ConFlagRef.expiration); }
	UConFlagRef*& nextFlagRef() { return UObject::Value<UConFlagRef*>(PropOffsets_ConFlagRef.nextFlagRef); }
};
