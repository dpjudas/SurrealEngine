#pragma once

#include "UObject.h"

class UTextBuffer : public UObject
{
public:
	UTextBuffer(ObjectStream* stream);

	uint32_t Pos = 0;
	uint32_t Top = 0;
	std::string Text;
};
