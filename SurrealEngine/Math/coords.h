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

	Coords Inverse() const
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

	Coords Transpose() const
	{
		Coords result;
		result.Origin = { -dot(Origin, XAxis), -dot(Origin, YAxis), -dot(Origin, ZAxis) };
		result.XAxis = { XAxis.x, YAxis.x, ZAxis.x };
		result.YAxis = { XAxis.y, YAxis.y, ZAxis.y };
		result.ZAxis = { XAxis.z, YAxis.z, ZAxis.z };
		return result;
	}

	Coords operator*(const Coords& coords)
	{
		Coords result;
		result.Origin = coords * Origin;
		result.XAxis = { dot(XAxis, coords.XAxis), dot(XAxis, coords.YAxis), dot(XAxis, coords.ZAxis) };
		result.YAxis = { dot(YAxis, coords.XAxis), dot(YAxis, coords.YAxis), dot(YAxis, coords.ZAxis) };
		result.ZAxis = { dot(ZAxis, coords.XAxis), dot(ZAxis, coords.YAxis), dot(ZAxis, coords.ZAxis) };
		return result;
	}

	vec3 operator*(const vec3& v) const
	{
		vec3 p = v - Origin;
		return { dot(p, XAxis), dot(p, YAxis), dot(p, ZAxis) };
	}

	static Coords Identity()
	{
		Coords coords;
		coords.Origin = vec3(0.0f);
		coords.XAxis = vec3(1.0f, 0.0f, 0.0f);
		coords.YAxis = vec3(0.0f, 1.0f, 0.0f);
		coords.ZAxis = vec3(0.0f, 0.0f, 1.0f);
		return coords;
	}

	static Coords YawRotation(float yaw)
	{
		Coords result;
		result.Origin = { 0.0f, 0.0f, 0.0f };
		result.XAxis = { std::cos(yaw), std::sin(yaw), 0.0f };
		result.YAxis = { -std::sin(yaw), std::cos(yaw), 0.0f };
		result.ZAxis = { 0.0f, 0.0f, 1.0f };
		return result;
	}

	static Coords PitchRotation(float pitch)
	{
		Coords result;
		result.Origin = { 0.0f, 0.0f, 0.0f };
		result.XAxis = { std::cos(pitch), 0.0f, std::sin(pitch) };
		result.YAxis = { 0.0f, 1.0f, 0.0f };
		result.ZAxis = { -std::sin(pitch), 0.0f, std::cos(pitch) };
		return result;
	}

	static Coords RollRotation(float roll)
	{
		Coords result;
		result.Origin = { 0.0f, 0.0f, 0.0f };
		result.XAxis = { 1.0f, 0.0f, 0.0f };
		result.YAxis = { 0.0f, std::cos(roll), -std::sin(roll) };
		result.ZAxis = { 0.0f, std::sin(roll), std::cos(roll) };
		return result;
	}

	static Coords Rotation(const Rotator& rotator)
	{
		return RollRotation(rotator.RollRadians()) * PitchRotation(rotator.PitchRadians()) * YawRotation(rotator.YawRadians());
	}

	static Coords InverseRotation(const Rotator& rotator)
	{
		return YawRotation(-rotator.YawRadians()) * PitchRotation(-rotator.PitchRadians()) * RollRotation(-rotator.RollRadians());
	}

	static Coords ViewToRenderDev()
	{
		Coords coords;
		coords.XAxis = vec3(-1.0f, 0.0f, 0.0f);
		coords.YAxis = vec3(0.0f, 0.0f, 1.0f);
		coords.ZAxis = vec3(0.0f, -1.0f, 0.0f);
		return coords;
	}

	static Coords ViewToAudioDev()
	{
		Coords coords;
		coords.XAxis = vec3(-1.0f, 0.0f, 0.0f);
		coords.YAxis = vec3(0.0f, 0.0f, 1.0f);
		coords.ZAxis = vec3(0.0f, -1.0f, 0.0f);
		return coords;
	}

	mat4 ToMatrix() const
	{
		mat4 coordsmatrix = mat4::null();
		coordsmatrix[0] = XAxis[0];
		coordsmatrix[1] = XAxis[1];
		coordsmatrix[2] = XAxis[2];
		coordsmatrix[4] = YAxis[0];
		coordsmatrix[5] = YAxis[1];
		coordsmatrix[6] = YAxis[2];
		coordsmatrix[8] = ZAxis[0];
		coordsmatrix[9] = ZAxis[1];
		coordsmatrix[10] = ZAxis[2];
		coordsmatrix[15] = 1.0f;
		return coordsmatrix;
	}
};
