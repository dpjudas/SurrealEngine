#pragma once

#include "UObject.h"

class USound : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::string Format;
	std::vector<uint8_t> Data;
};
