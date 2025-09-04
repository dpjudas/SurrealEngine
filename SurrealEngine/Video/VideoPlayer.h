#pragma once

#include <memory>
#include "Utils/Array.h"

class UnrealMipmap;

class VideoPlayer
{
public:
	static std::unique_ptr<VideoPlayer> Create(const std::string& filename);

	virtual ~VideoPlayer() = default;
	virtual bool Decode() = 0;
	virtual UnrealMipmap* NextVideoFrame() = 0;
};
