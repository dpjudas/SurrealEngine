#pragma once

#include "UObject.h"

class USound : public UObject
{
public:
	USound(ObjectStream* stream);

	std::string Format;
	std::vector<uint8_t> Data;
};
