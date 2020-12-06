#pragma once

#include "UObject.h"

class UMusic : public UObject
{
public:
	UMusic(ObjectStream* stream);

	std::string Format;
	std::vector<uint8_t> Data;
};
