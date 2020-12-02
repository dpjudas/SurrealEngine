#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>

class Pointf
{
public:
	Pointf() = default;
	Pointf(float x, float y) : x(x), y(y) { }

	float x = 0.0f;
	float y = 0.0f;
};

class Sizef
{
public:
	Sizef() = default;
	Sizef(float w, float h) : width(w), height(h) { }

	float width = 0.0f;
	float height = 0.0f;

	inline Sizef &operator+=(Sizef b) { width += b.width; height += b.height; return *this; }
	inline Sizef &operator-=(Sizef b) { width -= b.width; height -= b.height; return *this; }
};

inline Sizef operator+(Sizef a, Sizef b) { return { a.width + b.width, a.height + b.height }; }
inline Sizef operator-(Sizef a, Sizef b) { return { a.width - b.width, a.height - b.height }; }

class Rectf
{
public:
	Rectf() = default;
	Rectf(float left, float top, float right, float bottom) : left(left), top(top), right(right), bottom(bottom) { }
	Rectf(Pointf pos, Sizef size) : left(pos.x), top(pos.y), right(pos.x + size.width), bottom(pos.y + size.height) { }

	Pointf position() const { return { left, top }; }
	Sizef size() const { return { right - left, bottom - top }; }

	Rectf &boundingRect(const Rectf &rect)
	{
		Rectf result;
		result.left = std::min(left, rect.left);
		result.right = std::max(right, rect.right);
		result.top = std::min(top, rect.top);
		result.bottom = std::max(bottom, rect.bottom);
		*this = result;
		return *this;
	}

	bool contains(const Pointf &p) const
	{
		return (p.x >= left && p.x < right) && (p.y >= top && p.y < bottom);
	}

	float left = 0.0f;
	float top = 0.0f;
	float right = 0.0f;
	float bottom = 0.0f;
};

template<typename T>
struct vec2T
{
	union
	{
		struct { T x, y; };
		struct { T r, g; };
		struct { T s, t; };
		struct { T v[2]; };
	};

	vec2T() = default;
	vec2T(T v) : x(v), y(v) { }
	vec2T(T x, T y) : x(x), y(y) { }

	template<typename U>
	explicit vec2T(const vec2T<U> &v2) : x(static_cast<T>(v2.x)), y(static_cast<T>(v2.y)) { }

	vec2T<T> yx() const { return vec2T<T>(y, x); }
	vec2T<T> gr() const { return vec2T<T>(g, r); }
	vec2T<T> ts() const { return vec2T<T>(t, s); }

	vec2T<T> swizzle(int a, int b) const { return vec2T<T>(v[a], v[b]); }

	inline vec2T &operator+=(vec2T b) { x += b.x; y += b.y; return *this; }
	inline vec2T &operator-=(vec2T b) { x -= b.x; y -= b.y; return *this; }
	inline vec2T &operator*=(vec2T b) { x *= b.x; y *= b.y; return *this; }
	inline vec2T &operator/=(vec2T b) { x /= b.x; y /= b.y; return *this; }

	inline vec2T &operator+=(T b) { x += b; y += b; return *this; }
	inline vec2T &operator-=(T b) { x -= b; y -= b; return *this; }
	inline vec2T &operator*=(T b) { x *= b; y *= b; return *this; }
	inline vec2T &operator/=(T b) { x /= b; y /= b; return *this; }
};

template<typename T>
struct vec3T
{
	union
	{
		struct { T x, y, z; };
		struct { T r, g, b; };
		struct { T s, t, p; };
		struct { T v[3]; };
	};

	vec3T() = default;
	vec3T(T v) : x(v), y(v), z(v) { }
	vec3T(const vec2T<T> &v2, T z) : x(v2.x), y(v2.y), z(z) { }
	vec3T(T x, T y, T z) : x(x), y(y), z(z) { }

	template<typename U>
	explicit vec3T(const vec3T<U> &v3) : x(static_cast<T>(v3.x)), y(static_cast<T>(v3.y)), z(static_cast<T>(v3.z)) { }

	vec2T<T> xy() const { return vec2T<T>(x, y); }
	vec2T<T> rg() const { return vec2T<T>(r, g); }
	vec2T<T> st() const { return vec2T<T>(s, t); }

	vec2T<T> yx() const { return vec2T<T>(y, x); }
	vec2T<T> gr() const { return vec2T<T>(g, r); }
	vec2T<T> ts() const { return vec2T<T>(t, s); }

	vec3T<T> zyx() const { return vec3T<T>(z, y, x); }
	vec3T<T> bgr() const { return vec3T<T>(b, g, r); }
	vec3T<T> pts() const { return vec3T<T>(p, t, s); }

	vec2T<T> swizzle(int a, int b) const { return vec2T<T>(v[a], v[b]); }
	vec3T<T> swizzle(int a, int b, int c) const { return vec3T<T>(v[a], v[b], v[c]); }

	T &operator[](int i) { return v[i]; }
	const T &operator[](int i) const { return v[i]; }

	inline vec3T &operator+=(vec3T b) { x += b.x; y += b.y; z += b.z; return *this; }
	inline vec3T &operator-=(vec3T b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
	inline vec3T &operator*=(vec3T b) { x *= b.x; y *= b.y; z *= b.z; return *this; }
	inline vec3T &operator/=(vec3T b) { x /= b.x; y /= b.y; z /= b.z; return *this; }

	inline vec3T &operator+=(T b) { x += b; y += b; z += b; return *this; }
	inline vec3T &operator-=(T b) { x -= b; y -= b; z -= b; return *this; }
	inline vec3T &operator*=(T b) { x *= b; y *= b; z *= b; return *this; }
	inline vec3T &operator/=(T b) { x /= b; y /= b; z /= b; return *this; }
};

template<typename T>
struct vec4T
{
	union
	{
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };
		struct { T s, t, p, q; };
		struct { T v[4]; };
	};

	vec4T() = default;
	vec4T(T v) : x(v), y(v), z(v), w(v) { }
	vec4T(const vec2T<T> &a, const vec2T<T> &b) : x(a.x), y(a.y), z(b.x), w(b.y) { }
	vec4T(const vec3T<T> &v3, T w) : x(v3.x), y(v3.y), z(v3.z), w(w) { }
	vec4T(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }

	template<typename U>
	explicit vec4T(const vec4T<U> &v4) : x(static_cast<T>(v4.x)), y(static_cast<T>(v4.y)), z(static_cast<T>(v4.z)), w(static_cast<T>(v4.w)) { }

	vec2T<T> xy() const { return vec2T<T>(x, y); }
	vec2T<T> rg() const { return vec2T<T>(r, g); }
	vec2T<T> st() const { return vec2T<T>(s, t); }

	vec2T<T> yx() const { return vec2T<T>(y, x); }
	vec2T<T> gr() const { return vec2T<T>(g, r); }
	vec2T<T> ts() const { return vec2T<T>(t, s); }

	vec3T<T> xyz() const { return vec3T<T>(x, y, z); }
	vec3T<T> rgb() const { return vec3T<T>(r, g, b); }
	vec3T<T> stp() const { return vec3T<T>(s, t, p); }

	vec3T<T> zyx() const { return vec3T<T>(z, y, x); }
	vec3T<T> bgr() const { return vec3T<T>(b, g, r); }
	vec3T<T> pts() const { return vec3T<T>(p, t, s); }

	vec4T<T> wzyx() const { return vec4T<T>(w, z, y, x); }
	vec4T<T> abgr() const { return vec4T<T>(a, b, g, r); }
	vec4T<T> qpts() const { return vec4T<T>(q, p, t, s); }

	vec4T<T> bgra() const { return vec4T<T>(b, g, r, a); }

	vec2T<T> swizzle(int a, int b) const { return vec2T<T>(v[a], v[b]); }
	vec3T<T> swizzle(int a, int b, int c) const { return vec3T<T>(v[a], v[b], v[c]); }
	vec4T<T> swizzle(int a, int b, int c, int d) const { return vec4T<T>(v[a], v[b], v[c], v[d]); }

	inline vec4T &operator+=(vec4T b) { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
	inline vec4T &operator-=(vec4T b) { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
	inline vec4T &operator*=(vec4T b) { x *= b.x; y *= b.y; z *= b.z; w *= b.w; return *this; }
	inline vec4T &operator/=(vec4T b) { x /= b.x; y /= b.y; z /= b.z; w /= b.w; return *this; }

	inline vec4T &operator+=(T b) { x += b; y += b; z += b;  w += b; return *this; }
	inline vec4T &operator-=(T b) { x -= b; y -= b; z -= b;  w -= b; return *this; }
	inline vec4T &operator*=(T b) { x *= b; y *= b; z *= b;  w *= b; return *this; }
	inline vec4T &operator/=(T b) { x /= b; y /= b; z /= b;  w /= b; return *this; }
};

template<typename T> vec2T<T> operator+(vec2T<T> a, vec2T<T> b) { return vec2T<T>(a.x + b.x, a.y + b.y); }
template<typename T> vec2T<T> operator-(vec2T<T> a, vec2T<T> b) { return vec2T<T>(a.x - b.x, a.y - b.y); }
template<typename T> vec2T<T> operator*(vec2T<T> a, vec2T<T> b) { return vec2T<T>(a.x * b.x, a.y * b.y); }
template<typename T> vec2T<T> operator/(vec2T<T> a, vec2T<T> b) { return vec2T<T>(a.x / b.x, a.y / b.y); }

template<typename T> vec2T<T> operator+(T a, vec2T<T> b) { return vec2T<T>(a + b.x, a + b.y); }
template<typename T> vec2T<T> operator-(T a, vec2T<T> b) { return vec2T<T>(a - b.x, a - b.y); }
template<typename T> vec2T<T> operator*(T a, vec2T<T> b) { return vec2T<T>(a * b.x, a * b.y); }
template<typename T> vec2T<T> operator/(T a, vec2T<T> b) { return vec2T<T>(a / b.x, a / b.y); }

template<typename T> vec2T<T> operator+(vec2T<T> a, T b) { return vec2T<T>(a.x + b, a.y + b); }
template<typename T> vec2T<T> operator-(vec2T<T> a, T b) { return vec2T<T>(a.x - b, a.y - b); }
template<typename T> vec2T<T> operator*(vec2T<T> a, T b) { return vec2T<T>(a.x * b, a.y * b); }
template<typename T> vec2T<T> operator/(vec2T<T> a, T b) { return vec2T<T>(a.x / b, a.y / b); }

template<typename T> vec3T<T> operator+(vec3T<T> a, vec3T<T> b) { return vec3T<T>(a.x + b.x, a.y + b.y, a.z + b.z); }
template<typename T> vec3T<T> operator-(vec3T<T> a, vec3T<T> b) { return vec3T<T>(a.x - b.x, a.y - b.y, a.z - b.z); }
template<typename T> vec3T<T> operator*(vec3T<T> a, vec3T<T> b) { return vec3T<T>(a.x * b.x, a.y * b.y, a.z * b.z); }
template<typename T> vec3T<T> operator/(vec3T<T> a, vec3T<T> b) { return vec3T<T>(a.x / b.x, a.y / b.y, a.z / b.z); }

template<typename T> vec3T<T> operator+(T a, vec3T<T> b) { return vec3T<T>(a + b.x, a + b.y, a + b.z); }
template<typename T> vec3T<T> operator-(T a, vec3T<T> b) { return vec3T<T>(a - b.x, a - b.y, a - b.z); }
template<typename T> vec3T<T> operator*(T a, vec3T<T> b) { return vec3T<T>(a * b.x, a * b.y, a * b.z); }
template<typename T> vec3T<T> operator/(T a, vec3T<T> b) { return vec3T<T>(a / b.x, a / b.y, a / b.z); }

template<typename T> vec3T<T> operator+(vec3T<T> a, T b) { return vec3T<T>(a.x + b, a.y + b, a.z + b); }
template<typename T> vec3T<T> operator-(vec3T<T> a, T b) { return vec3T<T>(a.x - b, a.y - b, a.z - b); }
template<typename T> vec3T<T> operator*(vec3T<T> a, T b) { return vec3T<T>(a.x * b, a.y * b, a.z * b); }
template<typename T> vec3T<T> operator/(vec3T<T> a, T b) { return vec3T<T>(a.x / b, a.y / b, a.z / b); }

template<typename T> vec4T<T> operator+(vec4T<T> a, vec4T<T> b) { return vec4T<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
template<typename T> vec4T<T> operator-(vec4T<T> a, vec4T<T> b) { return vec4T<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
template<typename T> vec4T<T> operator*(vec4T<T> a, vec4T<T> b) { return vec4T<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
template<typename T> vec4T<T> operator/(vec4T<T> a, vec4T<T> b) { return vec4T<T>(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }

template<typename T> vec4T<T> operator+(T a, vec4T<T> b) { return vec4T<T>(a + b.x, a + b.y, a + b.z, a + b.w); }
template<typename T> vec4T<T> operator-(T a, vec4T<T> b) { return vec4T<T>(a - b.x, a - b.y, a - b.z, a - b.w); }
template<typename T> vec4T<T> operator*(T a, vec4T<T> b) { return vec4T<T>(a * b.x, a * b.y, a * b.z, a * b.w); }
template<typename T> vec4T<T> operator/(T a, vec4T<T> b) { return vec4T<T>(a / b.x, a / b.y, a / b.z, a / b.w); }

template<typename T> vec4T<T> operator+(vec4T<T> a, T b) { return vec4T<T>(a.x + b, a.y + b, a.z + b, a.w + b); }
template<typename T> vec4T<T> operator-(vec4T<T> a, T b) { return vec4T<T>(a.x - b, a.y - b, a.z - b, a.w - b); }
template<typename T> vec4T<T> operator*(vec4T<T> a, T b) { return vec4T<T>(a.x * b, a.y * b, a.z * b, a.w * b); }
template<typename T> vec4T<T> operator/(vec4T<T> a, T b) { return vec4T<T>(a.x / b, a.y / b, a.z / b, a.w / b); }

template<typename T> bool operator==(const vec2T<T> &a, const vec2T<T> &b) { return a.x == b.x && a.y == b.y; }
template<typename T> bool operator==(const vec3T<T> &a, const vec3T<T> &b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
template<typename T> bool operator==(const vec4T<T> &a, const vec4T<T> &b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

template<typename T> bool operator!=(const vec2T<T> &a, const vec2T<T> &b) { return a.x != b.x || a.y != b.y; }
template<typename T> bool operator!=(const vec3T<T> &a, const vec3T<T> &b) { return a.x != b.x || a.y != b.y || a.z != b.z; }
template<typename T> bool operator!=(const vec4T<T> &a, const vec4T<T> &b) { return a.x != b.x || a.y != b.y || a.z != b.z || a.w == b.w; }

typedef vec2T<float> vec2;
typedef vec3T<float> vec3;
typedef vec4T<float> vec4;
typedef vec2T<double> dvec2;
typedef vec3T<double> dvec3;
typedef vec4T<double> dvec4;
typedef vec2T<int32_t> ivec2;
typedef vec3T<int32_t> ivec3;
typedef vec4T<int32_t> ivec4;
typedef vec2T<uint32_t> uvec2;
typedef vec3T<uint32_t> uvec3;
typedef vec4T<uint32_t> uvec4;
typedef vec2T<bool> bvec2;
typedef vec3T<bool> bvec3;
typedef vec4T<bool> bvec4;
typedef vec2T<int16_t> ivec2s;
typedef vec3T<int16_t> ivec3s;
typedef vec4T<int16_t> ivec4s;
typedef vec2T<uint16_t> uvec2s;
typedef vec3T<uint16_t> uvec3s;
typedef vec4T<uint16_t> uvec4s;
typedef vec2T<int8_t> ivec2b;
typedef vec3T<int8_t> ivec3b;
typedef vec4T<int8_t> ivec4b;
typedef vec2T<uint8_t> uvec2b;
typedef vec3T<uint8_t> uvec3b;
typedef vec4T<uint8_t> uvec4b;

inline float dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }
inline float dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float dot(vec4 a, vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

inline double dot(dvec2 a, dvec2 b) { return a.x * b.x + a.y * b.y; }
inline double dot(dvec3 a, dvec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline double dot(dvec4 a, dvec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

inline float radians(float deg) { return deg * (3.14159265359f / 180.0f); }
inline float degrees(float rad) { return rad * (180.0f / 3.14159265359f); }

inline double radians(double deg) { return deg * (3.14159265359 / 180.0); }
inline double degrees(double rad) { return rad * (180.0 / 3.14159265359); }

inline float length(vec2 v) { return std::sqrt(dot(v, v)); }
inline float length(vec3 v) { return std::sqrt(dot(v, v)); }
inline float length(vec4 v) { return std::sqrt(dot(v, v)); }

inline double length(dvec2 v) { return std::sqrt(dot(v, v)); }
inline double length(dvec3 v) { return std::sqrt(dot(v, v)); }
inline double length(dvec4 v) { return std::sqrt(dot(v, v)); }

inline vec2 normalize(vec2 v) { return v / length(v); }
inline vec3 normalize(vec3 v) { return v / length(v); }
inline vec4 normalize(vec4 v) { return v / length(v); }

inline dvec2 normalize(dvec2 v) { return v / length(v); }
inline dvec3 normalize(dvec3 v) { return v / length(v); }
inline dvec4 normalize(dvec4 v) { return v / length(v); }

inline vec3 cross(vec3 a, vec3 b) { return { a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y }; }
inline dvec3 cross(dvec3 a, dvec3 b) { return { a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y }; }

template<typename T>
T mix(T a, T b, float t) { return a * (1.0f - t) + b * t; }

template<typename T>
T mix(T a, T b, double t) { return a * (1.0 - t) + b * t; }

template<typename T>
T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

template<class T>
T smoothstep(const T edge0, const T edge1, const T x)
{
	auto t = clamp<T>((x - edge0) / (edge1 - edge0), T(0.0), T(1.0));
	return t * t * (T(3.0) - T(2.0) * t);
}
