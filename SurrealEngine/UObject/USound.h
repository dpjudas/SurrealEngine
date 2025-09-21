#pragma once

#include "UObject.h"

class AudioLoopInfo
{
public:
	bool Looped = false;
	uint64_t LoopStart = 0;
	uint64_t LoopEnd = 0;
};

class AudioFrequency
{
public:
	AudioFrequency(int frequency) : frequency(frequency) {}
	int frequency = 0;
};

class USound : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void GetSound();
	float GetDuration();
	int GetChannels();

	NameString Format;
	Array<uint8_t> Data;

	Array<float> samples;
	float duration = 0.0f;
	int frequency = 0;
	int channels = 0;
	void* handle = nullptr;
	AudioLoopInfo loopInfo;
};
