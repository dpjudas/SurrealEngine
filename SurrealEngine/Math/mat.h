#pragma once

#include "vec.h"

enum class handedness
{
	left,
	right
};

enum class clipzrange
{
	negative_positive_w, // OpenGL, -wclip <= zclip <= wclip
	zero_positive_w      // Direct3D, 0 <= zclip <= wclip
};

struct mat4
{
	mat4() = default;

	static mat4 null();
	static mat4 identity();
	static mat4 from_values(float *matrix);
	static mat4 transpose(const mat4 &matrix);
	static mat4 translate(float x, float y, float z);
	static mat4 translate(const vec3 &v) { return translate(v.x, v.y, v.z); }
	static mat4 scale(float x, float y, float z);
	static mat4 scale(const vec3 &v) { return scale(v.x, v.y, v.z); }
	static mat4 rotate(float angle, float x, float y, float z);
	static mat4 rotate(float angle, const vec3 &v) { return rotate(angle, v.x, v.y, v.z); }
	static mat4 quaternion(float x, float y, float z, float w); // This function assumes that the quarternion is normalized.
	static mat4 quaternion(const vec4 &q) { return quaternion(q.x, q.y, q.z, q.w); }
	static mat4 swap_yz();
	static mat4 perspective(float fovy, float aspect, float z_near, float z_far, handedness handedness, clipzrange clipz);
	static mat4 frustum(float left, float right, float bottom, float top, float z_near, float z_far, handedness handedness, clipzrange clipz);
	static mat4 look_at(vec3 eye, vec3 center, vec3 up);
	static mat4 mirror(vec3 normal);

	vec4 operator*(const vec4 &v) const;
	mat4 operator*(const mat4 &m) const;

	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }

	float matrix[4 * 4];
};

struct mat3
{
	mat3() = default;
	mat3(const mat4 &m);

	static mat3 null();
	static mat3 identity();
	static mat3 from_values(float *matrix);
	static mat3 transpose(const mat3 &matrix);
	static mat3 inverse(const mat3 &matrix);
	static mat3 adjoint(const mat3 &matrix);

	static double determinant(const mat3 &m);

	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }

	vec3 operator*(const vec3 &v) const;
	mat3 operator*(const mat3 &m) const;

	float matrix[3 * 3];
};

struct mat2
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[2 * 2];
};

struct mat4x3
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[4 * 3];
};

struct mat4x2
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[4 * 2];
};

struct mat3x4
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[3 * 4];
};

struct mat3x2
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[3 * 2];
};

struct mat2x4
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[2 * 4];
};

struct mat2x3
{
	float operator[](size_t i) const { return matrix[i]; }
	float &operator[](size_t i) { return matrix[i]; }
	float matrix[2 * 3];
};
