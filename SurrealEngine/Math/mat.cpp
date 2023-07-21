
#include "Precomp.h"
#include "mat.h"
#include <cmath>
#ifndef NO_SSE
#include <emmintrin.h>
#endif
#include <string.h>

mat4 mat4::null()
{
	mat4 m;
	memset(m.matrix, 0, sizeof(m.matrix));
	return m;
}

mat4 mat4::identity()
{
	mat4 m = null();
	m.matrix[0] = 1.0f;
	m.matrix[5] = 1.0f;
	m.matrix[10] = 1.0f;
	m.matrix[15] = 1.0f;
	return m;
}

mat4 mat4::from_values(float *matrix)
{
	mat4 m;
	memcpy(m.matrix, matrix, sizeof(m.matrix));
	return m;
}

mat4 mat4::transpose(const mat4 &matrix)
{
	mat4 m;
	for (int y = 0; y < 4; y++)
		for (int x = 0; x < 4; x++)
			m.matrix[x + y * 4] = matrix.matrix[y + x * 4];
	return m;
}

mat4 mat4::translate(float x, float y, float z)
{
	mat4 m = identity();
	m.matrix[0 + 3 * 4] = x;
	m.matrix[1 + 3 * 4] = y;
	m.matrix[2 + 3 * 4] = z;
	return m;
}

mat4 mat4::scale(float x, float y, float z)
{
	mat4 m = null();
	m.matrix[0 + 0 * 4] = x;
	m.matrix[1 + 1 * 4] = y;
	m.matrix[2 + 2 * 4] = z;
	m.matrix[3 + 3 * 4] = 1;
	return m;
}

mat4 mat4::rotate(float angle, float x, float y, float z)
{
	float c = cosf(angle);
	float s = sinf(angle);
	mat4 m = null();
	m.matrix[0 + 0 * 4] = (x*x*(1.0f - c) + c);
	m.matrix[0 + 1 * 4] = (x*y*(1.0f - c) - z*s);
	m.matrix[0 + 2 * 4] = (x*z*(1.0f - c) + y*s);
	m.matrix[1 + 0 * 4] = (y*x*(1.0f - c) + z*s);
	m.matrix[1 + 1 * 4] = (y*y*(1.0f - c) + c);
	m.matrix[1 + 2 * 4] = (y*z*(1.0f - c) - x*s);
	m.matrix[2 + 0 * 4] = (x*z*(1.0f - c) - y*s);
	m.matrix[2 + 1 * 4] = (y*z*(1.0f - c) + x*s);
	m.matrix[2 + 2 * 4] = (z*z*(1.0f - c) + c);
	m.matrix[3 + 3 * 4] = 1.0f;
	return m;
}

mat4 mat4::quaternion(float x, float y, float z, float w)
{
	mat4 m = mat4::null();
	m.matrix[0 * 4 + 0] = 1.0f - 2.0f * y*y - 2.0f * z*z;
	m.matrix[1 * 4 + 0] = 2.0f * x*y - 2.0f * w*z;
	m.matrix[2 * 4 + 0] = 2.0f * x*z + 2.0f * w*y;
	m.matrix[0 * 4 + 1] = 2.0f * x*y + 2.0f * w*z;
	m.matrix[1 * 4 + 1] = 1.0f - 2.0f * x*x - 2.0f * z*z;
	m.matrix[2 * 4 + 1] = 2.0f * y*z - 2.0f * w*x;
	m.matrix[0 * 4 + 2] = 2.0f * x*z - 2.0f * w*y;
	m.matrix[1 * 4 + 2] = 2.0f * y*z + 2.0f * w*x;
	m.matrix[2 * 4 + 2] = 1.0f - 2.0f * x*x - 2.0f * y*y;
	m.matrix[3 * 4 + 3] = 1.0f;
	return m;
}

mat4 mat4::swap_yz()
{
	mat4 m = null();
	m.matrix[0 + 0 * 4] = 1.0f;
	m.matrix[1 + 2 * 4] = 1.0f;
	m.matrix[2 + 1 * 4] = -1.0f;
	m.matrix[3 + 3 * 4] = 1.0f;
	return m;
}

mat4 mat4::perspective(float fovy, float aspect, float z_near, float z_far, handedness handedness, clipzrange clipZ)
{
	float f = (float)(1.0 / tan(fovy * 3.14159265359 / 360.0));
	mat4 m = null();
	m.matrix[0 + 0 * 4] = f / aspect;
	m.matrix[1 + 1 * 4] = f;
	m.matrix[2 + 2 * 4] = (z_far + z_near) / (z_near - z_far);
	m.matrix[2 + 3 * 4] = (2.0f * z_far * z_near) / (z_near - z_far);
	m.matrix[3 + 2 * 4] = -1.0f;

	if (handedness == handedness::left)
	{
		m = m * mat4::scale(1.0f, 1.0f, -1.0f);
	}

	if (clipZ == clipzrange::zero_positive_w)
	{
		mat4 scale_translate = identity();
		scale_translate.matrix[2 + 2 * 4] = 0.5f;
		scale_translate.matrix[2 + 3 * 4] = 0.5f;
		m = scale_translate * m;
	}

	return m;
}

mat4 mat4::frustum(float left, float right, float bottom, float top, float z_near, float z_far, handedness handedness, clipzrange clipZ)
{
	float a = (right + left) / (right - left);
	float b = (top + bottom) / (top - bottom);
	float c = -(z_far + z_near) / (z_far - z_near);
	float d = -(2.0f * z_far) / (z_far - z_near);
	mat4 m = null();
	m.matrix[0 + 0 * 4] = 2.0f * z_near / (right - left);
	m.matrix[1 + 1 * 4] = 2.0f * z_near / (top - bottom);
	m.matrix[0 + 2 * 4] = a;
	m.matrix[1 + 2 * 4] = b;
	m.matrix[2 + 2 * 4] = c;
	m.matrix[2 + 3 * 4] = d;
	m.matrix[3 + 2 * 4] = -1;

	if (handedness == handedness::left)
	{
		m = m * mat4::scale(1.0f, 1.0f, -1.0f);
	}

	if (clipZ == clipzrange::zero_positive_w)
	{
		mat4 scale_translate = identity();
		scale_translate.matrix[2 + 2 * 4] = 0.5f;
		scale_translate.matrix[2 + 3 * 4] = 0.5f;
		m = scale_translate * m;
	}

	return m;
}

mat4 mat4::look_at(vec3 eye, vec3 center, vec3 up)
{
	vec3 f = normalize(center - eye);
	vec3 s = cross(f, normalize(up));
	vec3 u = cross(s, f);

	mat4 m = null();
	m.matrix[0 + 0 * 4] = s.x;
	m.matrix[0 + 1 * 4] = s.y;
	m.matrix[0 + 2 * 4] = s.z;
	m.matrix[1 + 0 * 4] = u.x;
	m.matrix[1 + 1 * 4] = u.y;
	m.matrix[1 + 2 * 4] = u.z;
	m.matrix[2 + 0 * 4] = -f.x;
	m.matrix[2 + 1 * 4] = -f.y;
	m.matrix[2 + 2 * 4] = -f.z;
	m.matrix[3 + 3 * 4] = 1.0f;
	return m * translate(-eye.x, -eye.y, -eye.z);
}

mat4 mat4::operator*(const mat4 &mult) const
{
	mat4 result;
	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			result.matrix[x + y * 4] =
				matrix[0 * 4 + x] * mult.matrix[y * 4 + 0] +
				matrix[1 * 4 + x] * mult.matrix[y * 4 + 1] +
				matrix[2 * 4 + x] * mult.matrix[y * 4 + 2] +
				matrix[3 * 4 + x] * mult.matrix[y * 4 + 3];
		}
	}
	return result;
}

vec4 mat4::operator*(const vec4 &v) const
{
#ifdef NO_SSE
	vec4 result;
	result.x = matrix[0 * 4 + 0] * v.x + matrix[1 * 4 + 0] * v.y + matrix[2 * 4 + 0] * v.z + matrix[3 * 4 + 0] * v.w;
	result.y = matrix[0 * 4 + 1] * v.x + matrix[1 * 4 + 1] * v.y + matrix[2 * 4 + 1] * v.z + matrix[3 * 4 + 1] * v.w;
	result.z = matrix[0 * 4 + 2] * v.x + matrix[1 * 4 + 2] * v.y + matrix[2 * 4 + 2] * v.z + matrix[3 * 4 + 2] * v.w;
	result.w = matrix[0 * 4 + 3] * v.x + matrix[1 * 4 + 3] * v.y + matrix[2 * 4 + 3] * v.z + matrix[3 * 4 + 3] * v.w;
	return result;
#else
	__m128 m0 = _mm_loadu_ps(matrix);
	__m128 m1 = _mm_loadu_ps(matrix + 4);
	__m128 m2 = _mm_loadu_ps(matrix + 8);
	__m128 m3 = _mm_loadu_ps(matrix + 12);
	__m128 mv = _mm_loadu_ps(&v.x);
	m0 = _mm_mul_ps(m0, _mm_shuffle_ps(mv, mv, _MM_SHUFFLE(0, 0, 0, 0)));
	m1 = _mm_mul_ps(m1, _mm_shuffle_ps(mv, mv, _MM_SHUFFLE(1, 1, 1, 1)));
	m2 = _mm_mul_ps(m2, _mm_shuffle_ps(mv, mv, _MM_SHUFFLE(2, 2, 2, 2)));
	m3 = _mm_mul_ps(m3, _mm_shuffle_ps(mv, mv, _MM_SHUFFLE(3, 3, 3, 3)));
	mv = _mm_add_ps(_mm_add_ps(_mm_add_ps(m0, m1), m2), m3);
	vec4 result;
	_mm_storeu_ps(&result.x, mv);
	return result;
#endif
}

/////////////////////////////////////////////////////////////////////////////

mat3::mat3(const mat4 &initmatrix)
{
	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
			matrix[x + y * 3] = initmatrix.matrix[x + y * 4];
}

mat3 mat3::null()
{
	mat3 m;
	memset(m.matrix, 0, sizeof(m.matrix));
	return m;
}

mat3 mat3::identity()
{
	mat3 m = null();
	m.matrix[0] = 1.0f;
	m.matrix[4] = 1.0f;
	m.matrix[8] = 1.0f;
	return m;
}

mat3 mat3::from_values(float *matrix)
{
	mat3 m;
	memcpy(m.matrix, matrix, sizeof(m.matrix));
	return m;
}

mat3 mat3::transpose(const mat3 &matrix)
{
	mat3 m;
	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
			m.matrix[x + y * 3] = matrix.matrix[y + x * 3];
	return m;
}

double mat3::determinant(const mat3 &m)
{
	double value;
	value = m.matrix[0 * 3 + 0] * ((m.matrix[1 * 3 + 1] * m.matrix[2 * 3 + 2]) - (m.matrix[2 * 3 + 1] * m.matrix[1 * 3 + 2]));
	value -= m.matrix[0 * 3 + 1] * ((m.matrix[1 * 3 + 0] * m.matrix[2 * 3 + 2]) - (m.matrix[2 * 3 + 0] * m.matrix[1 * 3 + 2]));
	value += m.matrix[0 * 3 + 2] * ((m.matrix[1 * 3 + 0] * m.matrix[2 * 3 + 1]) - (m.matrix[2 * 3 + 0] * m.matrix[1 * 3 + 1]));
	return value;
}

mat3 mat3::adjoint(const mat3 &m)
{
	mat3 result;
	result.matrix[0 * 3 + 0] = ((m.matrix[1 * 3 + 1] * m.matrix[2 * 3 + 2]) - (m.matrix[1 * 3 + 2] * m.matrix[2 * 3 + 1]));
	result.matrix[1 * 3 + 0] = -((m.matrix[1 * 3 + 0] * m.matrix[2 * 3 + 2]) - (m.matrix[1 * 3 + 2] * m.matrix[2 * 3 + 0]));
	result.matrix[2 * 3 + 0] = ((m.matrix[1 * 3 + 0] * m.matrix[2 * 3 + 1]) - (m.matrix[1 * 3 + 1] * m.matrix[2 * 3 + 0]));
	result.matrix[0 * 3 + 1] = -((m.matrix[0 * 3 + 1] * m.matrix[2 * 3 + 2]) - (m.matrix[0 * 3 + 2] * m.matrix[2 * 3 + 1]));
	result.matrix[1 * 3 + 1] = ((m.matrix[0 * 3 + 0] * m.matrix[2 * 3 + 2]) - (m.matrix[0 * 3 + 2] * m.matrix[2 * 3 + 0]));
	result.matrix[2 * 3 + 1] = -((m.matrix[0 * 3 + 0] * m.matrix[2 * 3 + 1]) - (m.matrix[0 * 3 + 1] * m.matrix[2 * 3 + 0]));
	result.matrix[0 * 3 + 2] = ((m.matrix[0 * 3 + 1] * m.matrix[1 * 3 + 2]) - (m.matrix[0 * 3 + 2] * m.matrix[1 * 3 + 1]));
	result.matrix[1 * 3 + 2] = -((m.matrix[0 * 3 + 0] * m.matrix[1 * 3 + 2]) - (m.matrix[0 * 3 + 2] * m.matrix[1 * 3 + 0]));
	result.matrix[2 * 3 + 2] = ((m.matrix[0 * 3 + 0] * m.matrix[1 * 3 + 1]) - (m.matrix[0 * 3 + 1] * m.matrix[1 * 3 + 0]));
	return result;
}

mat3 mat3::inverse(const mat3 &matrix)
{
	double d = mat3::determinant(matrix);

	// Inverse unknown when determinant is close to zero
	if (fabs(d) < 1e-15)
	{
		return null();
	}
	else
	{
		mat3 result = mat3::adjoint(matrix);

		d = 1.0 / d; // Inverse the determinant
		for (int i = 0; i < 9; i++)
		{
			result.matrix[i] = (float)(result.matrix[i] * d);
		}

		return result;
	}
}

mat3 mat3::operator*(const mat3 &mult) const
{
	mat3 result;
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			result.matrix[x + y * 3] =
				matrix[0 * 3 + x] * mult.matrix[y * 3 + 0] +
				matrix[1 * 3 + x] * mult.matrix[y * 3 + 1] +
				matrix[2 * 3 + x] * mult.matrix[y * 3 + 2];
		}
	}
	return result;
}

vec3 mat3::operator*(const vec3 &v) const
{
	vec3 result;
	result.x = matrix[0 * 3 + 0] * v.x + matrix[1 * 3 + 0] * v.y + matrix[2 * 3 + 0] * v.z;
	result.y = matrix[0 * 3 + 1] * v.x + matrix[1 * 3 + 1] * v.y + matrix[2 * 3 + 1] * v.z;
	result.z = matrix[0 * 3 + 2] * v.x + matrix[1 * 3 + 2] * v.y + matrix[2 * 3 + 2] * v.z;
	return result;
}
