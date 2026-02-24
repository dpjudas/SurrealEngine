#pragma once

#include "Math/vec.h"
#include "Math/mat.h"
#include "Math/rotator.h"

class Coords
{
public:
	vec3 Origin;
	vec3 XAxis;
	vec3 YAxis;
	vec3 ZAxis;

	Coords Inverse() const;
	Coords Transpose() const;

	static Coords Identity();
	static Coords Location(const vec3& origin);

	static Coords YawRotation(float yaw);
	static Coords PitchRotation(float pitch);
	static Coords RollRotation(float roll);

	static Coords Rotation(const Rotator& rotator);
	static Coords InverseRotation(const Rotator& rotator);

	// Converts a global vector to be represented with Coord's axes
	vec3 LocalizeVector(const vec3& globalVector) const;
	vec3 GlobalizeVector(const vec3& localVector) const;

	static Coords ViewToRenderDev();
	static Coords ViewToAudioDev();

	void GetAxes(vec3& X, vec3& Y, vec3& Z) const;
	void GetUnAxes(vec3& X, vec3& Y, vec3& Z) const;

	mat4 ToMatrix() const;
	static Coords FromMatrix(const mat4& m);
};

inline vec3 operator*(const Coords& coords, const vec3& v)
{
	vec3 p = v - coords.Origin;
	return { dot(p, coords.XAxis), dot(p, coords.YAxis), dot(p, coords.ZAxis) };
}

inline Coords operator*(const Coords& a, const Coords& b)
{
	Coords result;
	result.Origin = a * b.Origin;
	result.XAxis = { dot(a.XAxis, b.XAxis), dot(a.XAxis, b.YAxis), dot(a.XAxis, b.ZAxis) };
	result.YAxis = { dot(a.YAxis, b.XAxis), dot(a.YAxis, b.YAxis), dot(a.YAxis, b.ZAxis) };
	result.ZAxis = { dot(a.ZAxis, b.XAxis), dot(a.ZAxis, b.YAxis), dot(a.ZAxis, b.ZAxis) };
	return result;
}

inline Coords Coords::Inverse() const
{
	float det =
		XAxis.x * (YAxis.y * ZAxis.z - YAxis.z * ZAxis.y) +
		XAxis.y * (YAxis.z * ZAxis.x - YAxis.x * ZAxis.z) +
		XAxis.z * (YAxis.x * ZAxis.y - YAxis.y * ZAxis.x);
	float invdet = 1.0f / det;

	Coords result;
	result.Origin =
	{
		-dot(Origin, XAxis),
		-dot(Origin, YAxis),
		-dot(Origin, ZAxis)
	};
	result.XAxis =
	{
		(YAxis.y * ZAxis.z - YAxis.z * ZAxis.y) * invdet,
		(ZAxis.y * XAxis.z - ZAxis.z * XAxis.y) * invdet,
		(XAxis.y * YAxis.z - XAxis.z * YAxis.y) * invdet
	};
	result.YAxis =
	{
		(YAxis.z * ZAxis.x - ZAxis.z * YAxis.x) * invdet,
		(ZAxis.z * XAxis.x - XAxis.z * ZAxis.x) * invdet,
		(XAxis.z * YAxis.x - XAxis.x * YAxis.z) * invdet
	};
	result.ZAxis =
	{
		(YAxis.x * ZAxis.y - YAxis.y * ZAxis.x) * invdet,
		(ZAxis.x * XAxis.y - ZAxis.y * XAxis.x) * invdet,
		(XAxis.x * YAxis.y - XAxis.y * YAxis.x) * invdet
	};
	return result;
}

inline Coords Coords::Transpose() const
{
	Coords result;
	result.Origin = { -dot(Origin, XAxis), -dot(Origin, YAxis), -dot(Origin, ZAxis) };
	result.XAxis = { XAxis.x, YAxis.x, ZAxis.x };
	result.YAxis = { XAxis.y, YAxis.y, ZAxis.y };
	result.ZAxis = { XAxis.z, YAxis.z, ZAxis.z };
	return result;
}

inline Coords Coords::Identity()
{
	Coords coords;
	coords.Origin = vec3(0.0f);
	coords.XAxis = vec3(1.0f, 0.0f, 0.0f);
	coords.YAxis = vec3(0.0f, 1.0f, 0.0f);
	coords.ZAxis = vec3(0.0f, 0.0f, 1.0f);
	return coords;
}

inline Coords Coords::Location(const vec3& origin)
{
	Coords coords;
	coords.Origin = origin;
	coords.XAxis = vec3(1.0f, 0.0f, 0.0f);
	coords.YAxis = vec3(0.0f, 1.0f, 0.0f);
	coords.ZAxis = vec3(0.0f, 0.0f, 1.0f);
	return coords;
}

inline Coords Coords::YawRotation(float yaw)
{
	Coords result;
	result.Origin = { 0.0f, 0.0f, 0.0f };
	result.XAxis = { std::cos(yaw), -std::sin(yaw), 0.0f };
	result.YAxis = { std::sin(yaw), std::cos(yaw), 0.0f };
	result.ZAxis = { 0.0f, 0.0f, 1.0f };
	return result;
}

inline Coords Coords::PitchRotation(float pitch)
{
	Coords result;
	result.Origin = { 0.0f, 0.0f, 0.0f };
	result.XAxis = { std::cos(pitch), 0.0f, -std::sin(pitch) };
	result.YAxis = { 0.0f, 1.0f, 0.0f };
	result.ZAxis = { std::sin(pitch), 0.0f, std::cos(pitch) };
	return result;
}

inline Coords Coords::RollRotation(float roll)
{
	Coords result;
	result.Origin = { 0.0f, 0.0f, 0.0f };
	result.XAxis = { 1.0f, 0.0f, 0.0f };
	result.YAxis = { 0.0f, std::cos(roll), -std::sin(roll) };
	result.ZAxis = { 0.0f, std::sin(roll), std::cos(roll) };
	return result;
}

inline Coords Coords::Rotation(const Rotator& rotator)
{
	return RollRotation(rotator.RollRadians()) * PitchRotation(rotator.PitchRadians()) * YawRotation(rotator.YawRadians());
}

inline Coords Coords::InverseRotation(const Rotator& rotator)
{
	return YawRotation(-rotator.YawRadians()) * PitchRotation(-rotator.PitchRadians()) * RollRotation(-rotator.RollRadians());
}

inline vec3 Coords::LocalizeVector(const vec3& globalVector) const
{
	auto xAxis = dot(globalVector, XAxis);
	auto yAxis = dot(globalVector, YAxis);
	auto zAxis = dot(globalVector, ZAxis);

	return {xAxis, yAxis, zAxis};
}

inline vec3 Coords::GlobalizeVector(const vec3& localVector) const
{
	auto xAxis = dot(localVector, {1, 0, 0});
	auto yAxis = dot(localVector, {0, 1, 0});
	auto zAxis = dot(localVector, {0, 0, 1});

	return {xAxis, yAxis, zAxis};
}

inline void Coords::GetAxes(vec3& X, vec3& Y, vec3& Z) const
{
	X = XAxis;
	Y = YAxis;
	Z = ZAxis;
}

inline void Coords::GetUnAxes(vec3& X, vec3& Y, vec3& Z) const
{
	Coords coords = Inverse();
	X = coords.XAxis;
	Y = coords.YAxis;
	Z = coords.ZAxis;
}

inline Coords Coords::ViewToRenderDev()
{
	Coords coords;
	coords.Origin = { 0.0f, 0.0f, 0.0f };
	coords.XAxis = vec3(0.0f, 0.0f, 1.0f);
	coords.YAxis = vec3(1.0f, 0.0f, 0.0f);
	coords.ZAxis = vec3(0.0f, -1.0f, 0.0f);
	return coords;
}

inline Coords Coords::ViewToAudioDev()
{
	Coords coords;
	coords.Origin = { 0.0f, 0.0f, 0.0f };
	coords.XAxis = vec3(0.0f, 0.0f, 1.0f);
	coords.YAxis = vec3(1.0f, 0.0f, 0.0f);
	coords.ZAxis = vec3(0.0f, -1.0f, 0.0f);
	return coords;
}

inline mat4 Coords::ToMatrix() const
{
	mat4 coordsmatrix;
	coordsmatrix[0] = XAxis.x;
	coordsmatrix[1] = XAxis.y;
	coordsmatrix[2] = XAxis.z;
	coordsmatrix[3] = 0.0f;
	coordsmatrix[4] = YAxis.x;
	coordsmatrix[5] = YAxis.y;
	coordsmatrix[6] = YAxis.z;
	coordsmatrix[7] = 0.0f;
	coordsmatrix[8] = ZAxis.x;
	coordsmatrix[9] = ZAxis.y;
	coordsmatrix[10] = ZAxis.z;
	coordsmatrix[11] = 0.0f;
	coordsmatrix[12] = -Origin.x;
	coordsmatrix[13] = -Origin.y;
	coordsmatrix[14] = -Origin.z;
	coordsmatrix[15] = 1.0f;
	return coordsmatrix;
}

inline Coords Coords::FromMatrix(const mat4& coordsmatrix)
{
	Coords coords;
	coords.Origin = vec3(coordsmatrix[12], coordsmatrix[13], coordsmatrix[14]);
	coords.XAxis = vec3(coordsmatrix[0], coordsmatrix[1], coordsmatrix[2]);
	coords.YAxis = vec3(coordsmatrix[4], coordsmatrix[5], coordsmatrix[6]);
	coords.ZAxis = vec3(coordsmatrix[8], coordsmatrix[9], coordsmatrix[10]);
	return coords;
}
