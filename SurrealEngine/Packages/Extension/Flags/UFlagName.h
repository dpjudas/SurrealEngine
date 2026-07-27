#pragma once

#include "UFlag.h"

class UFlagName : public UFlag
{
public:
	using UFlag::UFlag;

	NameString& nameValue() { return Value<NameString>(PropOffsets_FlagName.nameValue); }
};
