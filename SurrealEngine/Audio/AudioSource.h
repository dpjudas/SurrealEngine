
#pragma once

#include <memory>
#include <vector>

class AudioSource
{
public:
	static std::unique_ptr<AudioSource> CreateMp3(std::vector<uint8_t> filedata);
	static std::unique_ptr<AudioSource> CreateFlac(std::vector<uint8_t> filedata);
	static std::unique_ptr<AudioSource> CreateWav(std::vector<uint8_t> filedata);
	static std::unique_ptr<AudioSource> CreateOgg(std::vector<uint8_t> filedata);
	static std::unique_ptr<AudioSource> CreateMod(std::vector<uint8_t> filedata, bool loop = true, int restrict_ = 0, int subsong = 0);
	static std::unique_ptr<AudioSource> CreateResampler(int targetFrequency, std::unique_ptr<AudioSource> source);

	AudioSource() = default;
	virtual ~AudioSource() = default;
	virtual int GetFrequency() = 0;
	virtual int GetChannels() = 0;
	virtual int GetSamples() = 0;
	virtual void SeekToSample(uint64_t position) = 0;
	virtual size_t ReadSamples(float* output, size_t samples) = 0;

	bool bIsLooped = false;
	uint32_t loopStart = 0;
	uint32_t loopEnd = 0;
};
