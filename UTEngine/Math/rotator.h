#pragma once

#include "mat.h"

class Rotator
{
public:
	Rotator() = default;
	Rotator(int pitch, int yaw, int roll) : Pitch(pitch), Yaw(yaw), Roll(roll) { }

	int Pitch;
	int Yaw;
	int Roll;

	float PitchDegrees() const { return Pitch * (360.0f / 65536.0f); }
	float YawDegrees() const { return Yaw * (360.0f / 65536.0f); }
	float RollDegrees() const { return Roll * (360.0f / 65536.0f); }

	Rotator& operator+=(const Rotator& b) { Pitch += b.Pitch; Yaw += b.Yaw; Roll += b.Roll; return *this; }
	Rotator& operator-=(const Rotator& b) { Pitch -= b.Pitch; Yaw -= b.Yaw; Roll -= b.Roll; return *this; }

	Rotator& operator*=(float scale) { Pitch = (int)(Pitch * scale); Yaw = (int)(Yaw * scale), Roll = (int)(Roll * scale); return *this; }
	Rotator& operator/=(float scale) { Pitch = (int)(Pitch / scale); Yaw = (int)(Yaw / scale), Roll = (int)(Roll / scale); return *this; }

	mat4 ToMatrix() const
	{
		return
			mat4::rotate(radians(YawDegrees()), 0.0f, 0.0f, 1.0f) *
			mat4::rotate(radians(PitchDegrees()), 0.0f, -1.0f, 0.0f) *
			mat4::rotate(radians(RollDegrees()), -1.0f, 0.0f, 0.0f);
	}
};

inline Rotator operator+(const Rotator& a, const Rotator& b) { return Rotator(a.Pitch + b.Pitch, a.Yaw + b.Yaw, a.Roll + b.Roll); }
inline Rotator operator-(const Rotator& a, const Rotator& b) { return Rotator(a.Pitch - b.Pitch, a.Yaw - b.Yaw, a.Roll - b.Roll); }
inline Rotator operator*(const Rotator& a, float scale) { return Rotator((int)(a.Pitch * scale), (int)(a.Yaw * scale), (int)(a.Roll * scale)); }
inline Rotator operator*(float scale, const Rotator& b) { return Rotator((int)(b.Pitch * scale), (int)(b.Yaw * scale), (int)(b.Roll * scale)); }
inline Rotator operator/(const Rotator& a, float scale) { return Rotator((int)(a.Pitch / scale), (int)(a.Yaw / scale), (int)(a.Roll / scale)); }
inline Rotator operator/(float scale, const Rotator& b) { return Rotator((int)(b.Pitch / scale), (int)(b.Yaw / scale), (int)(b.Roll / scale)); }
inline bool operator==(const Rotator& a, const Rotator& b) { return a.Pitch == b.Pitch && a.Yaw == b.Yaw && a.Roll == b.Roll; }
inline bool operator!=(const Rotator& a, const Rotator& b) { return a.Pitch != b.Pitch || a.Yaw != b.Yaw || a.Roll != b.Roll; }

inline Rotator normalize(Rotator rot)
{
	int pitch = rot.Pitch & 0xffff;
	int roll = rot.Roll & 0xffff;
	int yaw = rot.Yaw & 0xffff;
	if (pitch > 32767) pitch -= 0x10000;
	if (roll > 32767) roll -= 0x10000;
	if (yaw > 32767) yaw -= 0x10000;
	return { pitch, yaw, roll };
}
