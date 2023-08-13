#pragma once

#include <cstdint>

class Colorf
{
public:
	Colorf() = default;
	Colorf(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) { }

	static Colorf transparent() { return { 0.0f, 0.0f, 0.0f, 0.0f }; }

	static Colorf fromRgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
	{
		float s = 1.0f / 255.0f;
		return { r * s, g * s, b * s, a * s };
	}

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
};
