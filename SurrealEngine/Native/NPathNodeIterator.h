#pragma once

#include "UObject/UObject.h"

class NPathNodeIterator
{
public:
	static void RegisterFunctions();

	static void BuildPath(UObject* Self, const vec3& Start, const vec3& End);
	static void GetCurrent(UObject* Self, UObject*& ReturnValue);
	static void GetFirst(UObject* Self, UObject*& ReturnValue);
	static void GetLast(UObject* Self, UObject*& ReturnValue);
	static void GetLastVisible(UObject* Self, UObject*& ReturnValue);
	static void GetNext(UObject* Self, UObject*& ReturnValue);
	static void GetPrevious(UObject* Self, UObject*& ReturnValue);
};
