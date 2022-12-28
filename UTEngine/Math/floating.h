#pragma once
#include <cmath>
#include <cfloat>

class Float
{
public:
	static inline bool Equals(float a, float b, float epsilon = FLT_EPSILON)
	{
		return (std::fabs(a - b) < epsilon);
	}
};

class Double
{
public:
	static inline bool Equals(double a, double b, double epsilon = DBL_EPSILON)
	{
		return (std::abs(a - b) < epsilon);
	}
};
