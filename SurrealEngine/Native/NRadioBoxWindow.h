#pragma once

#include "UObject/UObject.h"

class NRadioBoxWindow
{
public:
	static void RegisterFunctions();

	static void GetEnabledToggle(UObject* Self, UObject*& ReturnValue);
};
