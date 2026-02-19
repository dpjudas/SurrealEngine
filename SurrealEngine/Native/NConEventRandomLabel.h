#pragma once

#include "UObject/UObject.h"

class NConEventRandomLabel
{
public:
	static void RegisterFunctions();

	static void GetLabel(UObject* Self, int labelIndex, std::string& ReturnValue);
	static void GetLabelCount(UObject* Self, int& ReturnValue);
	static void GetRandomLabel(UObject* Self, std::string& ReturnValue);
};
