#pragma once

#include "UObject/UObject.h"

struct PointRegion;

class NLevelInfo
{
public:
	static void RegisterFunctions();

	static void GetAddressURL(UObject* Self, std::string& ReturnValue);
	static void GetLocalURL(UObject* Self, std::string& ReturnValue);
	static void GetLocZone_U227(UObject* Self, const vec3& Pos, PointRegion& ReturnValue);
	static void InitEventManager(UObject* Self);
};
