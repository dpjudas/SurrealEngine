#pragma once

#include "Packages/Core/UObject.h"

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

	Array<uint8_t> lipsyncLetters;

	uint8_t GetLipsyncLetterAt(float seconds);

private:
	static uint8_t LetterForHz(float hz);
	static constexpr int LIPSYNC_BLOCK_SIZE = 2048;
	const Array<uint8_t>& GetLipsyncLetters();
};
