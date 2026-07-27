#pragma once

#include "UFlag.h"

class UFlagBool : public UFlag
{
public:
	using UFlag::UFlag;

	BitfieldBool bValue() { return BoolValue(PropOffsets_FlagBool.bValue); }
};
