#pragma once

#include "UFlag.h"

class UFlagVector : public UFlag
{
public:
	using UFlag::UFlag;

	vec3& vectorValue() { return Value<vec3>(PropOffsets_FlagVector.vectorValue); }
};
