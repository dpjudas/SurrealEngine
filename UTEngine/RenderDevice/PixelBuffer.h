#pragma once

#include <cstdint>

enum class PixelFormat
{
	rgba8,
	rgba16,
	rgba16f,
	r32f,
	rg16f,
	rgba16_snorm
};

inline int getBytesPerPixels(PixelFormat format)
{
	static const int formatSizes[] = { 4, 8, 8, 4, 4, 8 };
	return formatSizes[(int)format];
}

class PixelBuffer
{
public:
	PixelBuffer(int width, int height, int size, PixelFormat format, int levels) : width(width), height(height), size(size), format(format), data(new uint8_t[size]), bytes_per_pixel(getBytesPerPixels(format)), levels(levels) { }
	PixelBuffer(int width, int height, PixelFormat format) : width(width), height(height), size(width * height * getBytesPerPixels(format)), format(format), bytes_per_pixel(getBytesPerPixels(format)) { data = new uint8_t[size]; }
	~PixelBuffer() { delete[](uint8_t*)data; }

	int width = 0;
	int height = 0;
	int size = 0;
	PixelFormat format = PixelFormat::rgba8;
	void *data = nullptr;

	float pixel_ratio = 1.0f;
	int bytes_per_pixel = 4;
	int levels = 1;

private:
	PixelBuffer(const PixelBuffer &) = delete;
	PixelBuffer &operator=(const PixelBuffer &) = delete;
};
