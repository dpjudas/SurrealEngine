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

	float PitchRadians() const { return Pitch * (3.14159265359f / 32768.0f); }
	float YawRadians() const { return Yaw * (3.14159265359f / 32768.0f); }
	float RollRadians() const { return Roll * (3.14159265359f / 32768.0f); }

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

	void GetAxes(vec3& X, vec3& Y, vec3& Z) const
	{
		mat4 m = ToMatrix();
		X.x = m[0]; X.y = m[1]; X.z = m[2];
		Y.x = m[4]; Y.y = m[5]; Y.z = m[6];
		Z.x = m[8]; Z.y = m[9]; Z.z = m[10];
	}

	void GetUnAxes(vec3& X, vec3& Y, vec3& Z) const
	{
		mat3 m = mat3::inverse(mat3(ToMatrix()));
		X.x = m[0]; X.y = m[1]; X.z = m[2];
		Y.x = m[3]; Y.y = m[4]; Y.z = m[5];
		Z.x = m[6]; Z.y = m[7]; Z.z = m[8];
	}

	static int TurnToShortest(int from, int to, int speed)
	{
		from = from & 0xffff;
		to = to & 0xffff;

		if (from > to)
		{
			if (from - to < 0x8000)
				return (from - std::min((from - to), speed)) & 0xffff;
			else
				return (from + std::min((to + 0x10000 - from), speed)) & 0xffff;
		}
		else
		{
			if (to - from < 0x8000)
				return (from + std::min((to - from), speed)) & 0xffff;
			else
				return (from - std::min((from + 0x10000 - to), speed)) & 0xffff;
		}
	}

	static int TurnToFixed(int from, int to, int direction)
	{
		from = from & 0xffff;
		to = to & 0xffff;

		if (direction > 0)
		{
			if (from > to)
				return (from + std::min(direction, to - from + 0x10000)) & 0xffff;
			else
				return (from + std::min(direction, to - from)) & 0xffff;
		}
		else
		{
			if (from < to)
				return (from + std::max(direction, to - from - 0x10000)) & 0xffff;
			else
				return (from + std::max(direction, to - from)) & 0xffff;
		}
	}

	static Rotator FromVector(const vec3& v)
	{
		float scale = 0x10000 / (2.0f * 3.14159265359f);
		return Rotator((int)(std::atan2(v.z, std::sqrt(v.x * v.x + v.y * v.y)) * scale), (int)(std::atan2(v.y, v.x) * scale), 0);
	}
};

inline Rotator operator+(const Rotator& a, const Rotator& b) { return Rotator(a.Pitch + b.Pitch, a.Yaw + b.Yaw, a.Roll + b.Roll); }
inline Rotator operator-(const Rotator& a, const Rotator& b) { return Rotator(a.Pitch - b.Pitch, a.Yaw - b.Yaw, a.Roll - b.Roll); }
inline Rotator operator*(const Rotator& a, float scale) { return Rotator((int)(a.Pitch * scale), (int)(a.Yaw * scale), (int)(a.Roll * scale)); }
inline Rotator operator*(float scale, const Rotator& b) { return Rotator((int)(b.Pitch * scale), (int)(b.Yaw * scale), (int)(b.Roll * scale)); }
inline Rotator operator/(const Rotator& a, float scale) { return Rotator((int)(a.Pitch / scale), (int)(a.Yaw / scale), (int)(a.Roll / scale)); }
inline Rotator operator/(float scale, const Rotator& b) { return Rotator((int)(b.Pitch / scale), (int)(b.Yaw / scale), (int)(b.Roll / scale)); }
inline bool operator==(const Rotator& a, const Rotator& b) { return (uint16_t)a.Pitch == (uint16_t)b.Pitch && (uint16_t)a.Yaw == (uint16_t)b.Yaw && (uint16_t)a.Roll == (uint16_t)b.Roll; }
inline bool operator!=(const Rotator& a, const Rotator& b) { return (uint16_t)a.Pitch != (uint16_t)b.Pitch || (uint16_t)a.Yaw != (uint16_t)b.Yaw || (uint16_t)a.Roll != (uint16_t)b.Roll; }

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
