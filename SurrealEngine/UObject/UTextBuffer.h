#pragma once

#include "UObject.h"

class UTextBuffer : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	uint32_t Pos = 0;
	uint32_t Top = 0;
	std::string Text;
};
