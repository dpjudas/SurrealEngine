#pragma once

#include "vec.h"
#include "mat.h"

enum class EulerOrder
{
	xyz,
	xzy,
	yzx,
	yxz,
	zxy,
	zyx
};

template<typename T>
struct quaternionT
{
	quaternionT() : x((T)0), y((T)0), z((T)0), w((T)1) { }
	quaternionT(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }
	quaternionT(const vec4T<T> &v) : x(v.x), y(v.y), z(v.z), w(v.w) { }
	quaternionT(T angle, const vec3T<T> &axis);

	static quaternionT euler(T x, T y, T z, EulerOrder order = EulerOrder::yxz);
	static quaternionT euler(const vec3T<T> &xyz, EulerOrder order = EulerOrder::yxz);
	static quaternionT rotation_matrix(const mat4 &matrix);

	operator vec4T<T>() const { return vec4T<T>(x, y, z, w); }

	T x, y, z, w;
};

/// Linear Quaternion Interpolation
template<typename T>
quaternionT<T> lerp(const quaternionT<T> &quaternion_initial, const quaternionT<T> &quaternion_final, T lerp_time)
{
	quaternionT<T> q(
		quaternion_initial.x * (((T) 1.0) - lerp_time) + quaternion_final.x * lerp_time,
		quaternion_initial.y * (((T) 1.0) - lerp_time) + quaternion_final.y * lerp_time,
		quaternion_initial.z * (((T) 1.0) - lerp_time) + quaternion_final.z * lerp_time,
		quaternion_initial.w * (((T) 1.0) - lerp_time) + quaternion_final.w * lerp_time
	);
	return normalize(q);
}

/// Spherical Quaternion Interpolation
template<typename T>
quaternionT<T> slerp(const quaternionT<T> &quaternion_initial, const quaternionT<T> &quaternion_final, T slerp_time)
{
	T q2[4];

	q2[0] = quaternion_final.x;
	q2[1] = quaternion_final.y;
	q2[2] = quaternion_final.z;
	q2[3] = quaternion_final.w;

	T cos_theta = quaternion_initial.x * quaternion_final.x
		+ quaternion_initial.y * quaternion_final.y
		+ quaternion_initial.z * quaternion_final.z
		+ quaternion_initial.w * quaternion_final.w;

	if (cos_theta < ((T) 0.0))
	{
		q2[0] = -q2[0];
		q2[1] = -q2[1];
		q2[2] = -q2[2];
		q2[3] = -q2[3];
		cos_theta = -cos_theta;
	}

	T beta = ((T) 1.0) - slerp_time;

	if (((T) 1.0) - cos_theta > ((T) 0.001))
	{
		cos_theta = acos(cos_theta);
		T sin_theta = 1.0f / sin(cos_theta);
		beta = sin(cos_theta * beta) * sin_theta;
		slerp_time = sin(cos_theta * slerp_time) * sin_theta;
	}

	quaternionT<T> quat;
	quat.x = beta * quaternion_initial.x + slerp_time * q2[0];
	quat.y = beta * quaternion_initial.y + slerp_time * q2[1];
	quat.z = beta * quaternion_initial.z + slerp_time * q2[2];
	quat.w = beta * quaternion_initial.w + slerp_time * q2[3];
	return quat;
}

template<typename T>
T magnitude(const quaternionT<T> &q)
{
	return std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

template<typename T>
quaternionT<T> normalize(const quaternionT<T> &q)
{
	T m = magnitude(q);
	if (m != T(0))
		return quaternionT<T>(q.x / m, q.y / m, q.z / m, q.w / m);
	else
		return quaternionT<T>(T(0), T(0), T(0), T(0));
}

/// Inverse this quaternion
/// This is the same as the conjugate of a quaternion
template<typename T>
quaternionT<T> inverse(const quaternionT<T> &q)
{
	return { -q.x, -q.y, -q.z, q.w };
}

/// Shortest arc quaternion between two vectors
template<typename T>
quaternionT<T> rotation_between(vec3T<T> v0, vec3T<T> v1)
{
	v0 = normalize(v0);
	v1 = normalize(v1);

	T d = dot(v0, v1);
	if (d >= 1.0f) // v0 and v1 is the same vector
	{
		return quaternionT<T>();
	}
	else if (d < T(1e-6 - 1.0)) // v0 and v1 are parallel but pointing in opposite directions
	{
		// We can rotate around any axis - find one using the cross product:
		vec3T<T> axis = cross(vec3T<T>(T(1), T(0), T(0)), v0);
		if (dot(axis, axis) < T(1e-6)) // colinear
			axis = cross(vec3T<T>(T(0), T(1), T(0)), v0);
		axis = normalize(axis);
		return quaternionT<T>((T)3.14159265359, axis);
	}
	else
	{
		T s = std::sqrt((1 + d) * 2);
		T rcp_s = T(1) / s;
		vec3T<T> c = cross(v0, v1);
		quaternionT<T> q(vec4T<T>(c * rcp_s, s * T(0.5)));
		return normalize(q);
	}
}

template<typename T>
quaternionT<T> quaternionT<T>::euler(T x, T y, T z, EulerOrder order)
{
	quaternionT<T> q_x(x, vec3T<T>((T)1, (T)0, (T)0));
	quaternionT<T> q_y(y, vec3T<T>((T)0, (T)1, (T)0));
	quaternionT<T> q_z(z, vec3T<T>((T)0, (T)0, (T)1));

	switch (order)
	{
	default:
	case EulerOrder::xyz:
		return q_x * q_y * q_z;
	case EulerOrder::xzy:
		return q_x * q_z * q_y;
	case EulerOrder::yzx:
		return q_y * q_z * q_x;
	case EulerOrder::yxz:
		return q_y * q_x * q_z;
	case EulerOrder::zxy:
		return q_z * q_x * q_y;
	case EulerOrder::zyx:
		return q_z * q_y * q_x;
	}
}

template<typename T>
quaternionT<T> quaternionT<T>::euler(const vec3T<T> &angles, EulerOrder order)
{
	return euler(angles.x, angles.y, angles.z, order);
}

template<typename T>
quaternionT<T>::quaternionT(T angle, const vec3T<T> &axis)
{
	T len = length(axis);
	T half_angle_radians = angle / T(2);
	quaternionT<T> q;
	q.w = std::cos(half_angle_radians);
	q.x = axis.x * std::sin(half_angle_radians) / len;
	q.y = axis.y * std::sin(half_angle_radians) / len;
	q.z = axis.z * std::sin(half_angle_radians) / len;
	*this = normalize(q);
}

template<typename T>
quaternionT<T> quaternionT<T>::rotation_matrix(const mat4 &m)
{
	float x, y, z, w;
	float size;

	// We assume that this is a pure rotation matrix without any scale or translation
	float trace = m.matrix[0 * 4 + 0] + m.matrix[1 * 4 + 1] + m.matrix[2 * 4 + 2] + (float)1;
	if (trace > (float) 16.0*FLT_EPSILON)
	{
		size = sqrt(trace) * (float)2;
		x = (m.matrix[1 * 4 + 2] - m.matrix[2 * 4 + 1]) / size;
		y = (m.matrix[2 * 4 + 0] - m.matrix[0 * 4 + 2]) / size;
		z = (m.matrix[0 * 4 + 1] - m.matrix[1 * 4 + 0]) / size;
		w = (float) 0.25 * size;
	}
	else
	{
		// If the trace of the matrix is equal to zero then identify which major diagonal element has the greatest value.

		if (m.matrix[0 * 4 + 0] > m.matrix[1 * 4 + 1] && m.matrix[0 * 4 + 0] > m.matrix[2 * 4 + 2])
		{
			// Column 0: 
			size = sqrt((float) 1.0 + m.matrix[0 * 4 + 0] - m.matrix[1 * 4 + 1] - m.matrix[2 * 4 + 2]) * (float)2;
			x = (float) 0.25 * size;
			y = (m.matrix[0 * 4 + 1] + m.matrix[1 * 4 + 0]) / size;
			z = (m.matrix[2 * 4 + 0] + m.matrix[0 * 4 + 2]) / size;
			w = (m.matrix[1 * 4 + 2] - m.matrix[2 * 4 + 1]) / size;
		}
		else if (m.matrix[1 * 4 + 1] > m.matrix[2 * 4 + 2])
		{
			// Column 1: 
			size = sqrt((float) 1.0 + m.matrix[1 * 4 + 1] - m.matrix[0 * 4 + 0] - m.matrix[2 * 4 + 2]) * (float)2;
			x = (m.matrix[0 * 4 + 1] + m.matrix[1 * 4 + 0]) / size;
			y = (float) 0.25 * size;
			z = (m.matrix[1 * 4 + 2] + m.matrix[2 * 4 + 1]) / size;
			w = (m.matrix[2 * 4 + 0] - m.matrix[0 * 4 + 2]) / size;
		}
		else
		{
			// Column 2:
			size = sqrt((float) 1.0 + m.matrix[2 * 4 + 2] - m.matrix[0 * 4 + 0] - m.matrix[1 * 4 + 1]) * (float)2;
			x = (m.matrix[2 * 4 + 0] + m.matrix[0 * 4 + 2]) / size;
			y = (m.matrix[1 * 4 + 2] + m.matrix[2 * 4 + 1]) / size;
			z = (float) 0.25 * size;
			w = (m.matrix[0 * 4 + 1] - m.matrix[1 * 4 + 0]) / size;
		}
	}
	return { x, y, z, w };
}

template<typename T>
quaternionT<T> operator*(const quaternionT<T> &quaternion_1, const quaternionT<T> &quaternion_2)
{
	quaternionT<T> quaternion_dest;
	quaternion_dest.x = quaternion_1.w*quaternion_2.x + quaternion_1.x*quaternion_2.w + quaternion_1.y*quaternion_2.z - quaternion_1.z*quaternion_2.y;
	quaternion_dest.y = quaternion_1.w*quaternion_2.y + quaternion_1.y*quaternion_2.w + quaternion_1.z*quaternion_2.x - quaternion_1.x*quaternion_2.z;
	quaternion_dest.z = quaternion_1.w*quaternion_2.z + quaternion_1.z*quaternion_2.w + quaternion_1.x*quaternion_2.y - quaternion_1.y*quaternion_2.x;
	quaternion_dest.w = quaternion_1.w*quaternion_2.w - quaternion_1.x*quaternion_2.x - quaternion_1.y*quaternion_2.y - quaternion_1.z*quaternion_2.z;
	return quaternion_dest;
}

template<typename T>
vec3T<T> operator*(const quaternionT<T> &q, const vec3T<T> &v)
{
	vec3T<T> q3(q.x, q.y, q.z);
	return v + cross(q3, cross(q3, v) + v * q.w) * ((T) 2.0);
}

template<typename T>
quaternionT<T> operator*(const quaternionT<T> &q, const mat4 &m)
{
	quaternionT<T> result;
	result.x = m.matrix[(4 * 0) + 0] * q.x + m.matrix[(4 * 0) + 1] * q.y + m.matrix[(4 * 0) + 2] * q.z + m.matrix[(4 * 0) + 3] * q.w;
	result.y = m.matrix[(4 * 1) + 0] * q.x + m.matrix[(4 * 1) + 1] * q.y + m.matrix[(4 * 1) + 2] * q.z + m.matrix[(4 * 1) + 3] * q.w;
	result.z = m.matrix[(4 * 2) + 0] * q.x + m.matrix[(4 * 2) + 1] * q.y + m.matrix[(4 * 2) + 2] * q.z + m.matrix[(4 * 2) + 3] * q.w;
	result.w = m.matrix[(4 * 3) + 0] * q.x + m.matrix[(4 * 3) + 1] * q.y + m.matrix[(4 * 3) + 2] * q.z + m.matrix[(4 * 3) + 3] * q.w;
	return result;
}

typedef quaternionT<float> quaternion;
typedef quaternionT<double> dquaternion;
