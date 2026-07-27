#pragma once

#include "Packages/Extension/UExtensionObject.h"

class UFlagBase;

class UFlag : public UExtensionObject
{
public:
	using UExtensionObject::UExtensionObject;

	UFlagBase*& FlagBase() { return Value<UFlagBase*>(PropOffsets_Flag.FlagBase); }
	NameString& FlagName() { return Value<NameString>(PropOffsets_Flag.FlagName); }
	int& expiration() { return Value<int>(PropOffsets_Flag.expiration); }
	int& flagHash() { return Value<int>(PropOffsets_Flag.flagHash); }
	uint8_t& flagType() { return Value<uint8_t>(PropOffsets_Flag.flagType); }
	UFlag*& nextFlag() { return Value<UFlag*>(PropOffsets_Flag.nextFlag); }
};
