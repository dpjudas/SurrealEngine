#pragma once

#include "UObject.h"

class AudioLoopInfo
{
public:
	bool Looped = false;
	uint64_t LoopStart = 0;
	uint64_t LoopEnd = 0;
};

class USound : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	void GetSound();
	float GetDuration();
	int GetChannels();

	NameString Format;
	std::vector<uint8_t> Data;

	std::vector<float> samples;
	float duration = 0.0f;
	int frequency = 0;
	int channels = 0;
	void* handle = nullptr;
};
