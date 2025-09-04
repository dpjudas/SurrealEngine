#pragma once

#include <memory>

class VideoPlayer
{
public:
	static std::unique_ptr<VideoPlayer> Create(const std::string& filename);

	virtual ~VideoPlayer() = default;
	virtual bool Decode() = 0;
};
