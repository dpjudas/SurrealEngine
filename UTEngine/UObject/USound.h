#pragma once

#include "UObject.h"

class AudioSound;

class USound : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	AudioSound* GetSound();

	std::string Format;
	std::vector<uint8_t> Data;
	AudioSound* Sound = nullptr;
};
