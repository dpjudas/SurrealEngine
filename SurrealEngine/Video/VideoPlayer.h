#pragma once

#include <memory>
#include "Utils/Array.h"

class UnrealMipmap;
class AudioSource;

class VideoPlayer
{
public:
	static std::unique_ptr<VideoPlayer> Create(const std::string& filename);

	virtual ~VideoPlayer() = default;

	virtual int GetFrameIndexForTime(float timestamp) = 0;

	virtual bool Decode() = 0;
	virtual UnrealMipmap* NextVideoFrame() = 0;

	virtual std::unique_ptr<AudioSource> GetAudio() = 0;
};
