#pragma once

#include <cstdint>
#include <cstring>

enum class VideoDecoderResult
{
	Decoded,
	DecodedFrame,
	Error
};

class IVideoDecoder
{
public:
	virtual VideoDecoderResult Decode(const void* data, size_t size) = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual const uint32_t* GetPixels() = 0;
	virtual void Release() = 0;
};

#ifdef _MSC_VER
#ifdef BUILDING_SURREALVIDEO
#define SURREALVIDEO_API __declspec(dllexport)
#else
#define SURREALVIDEO_API __declspec(dllimport)
#endif
#else
#define SURREALVIDEO_API
#endif

SURREALVIDEO_API IVideoDecoder* CreateVideoDecoder();
