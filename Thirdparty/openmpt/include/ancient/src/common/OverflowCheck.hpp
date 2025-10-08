/* Copyright (C) Teemu Suutari */

#ifndef OVERFLOW_CHECK_HPP
#define OVERFLOW_CHECK_HPP

#include "Buffer.hpp"

namespace ancient::internal
{

class OverflowCheck
{
public:
	template<typename T,typename U>
	static T sum(T a,U b)
	{
		// TODO: Add type traits to handle signed integers
		T ret=a+b;
		if (ret<a) throw Buffer::OutOfBoundsError();
		return ret;
	}

	template<typename T,typename U,typename ...Args>
	static T sum(T a,U b,Args... args)
	{
		return sum(sum(a,b),args...);
	}
};

}

#endif
