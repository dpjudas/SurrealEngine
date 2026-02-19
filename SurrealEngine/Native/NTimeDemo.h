#pragma once

#include "UObject/UObject.h"

class NTimeDemo
{
public:
	static void RegisterFunctions();

	static void CloseFile(UObject* Self);
	static void OpenFile(UObject* Self);
	static void WriteToFile(UObject* Self, const std::string& Text);
};
