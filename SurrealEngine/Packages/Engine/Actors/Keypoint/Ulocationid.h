#pragma once

#include "UKeypoint.h"

class Ulocationid : public UKeypoint
{
public:
	using UKeypoint::UKeypoint;

	std::string& LocationName() { return Value<std::string>(PropOffsets_locationid.LocationName); }
	Ulocationid*& NextLocation() { return Value<Ulocationid*>(PropOffsets_locationid.NextLocation); }
	float& Radius() { return Value<float>(PropOffsets_locationid.Radius); }
};
