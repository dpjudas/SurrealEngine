#pragma once

#include "UObject/UObject.h"

class NExtString
{
public:
	static void RegisterFunctions();

	static void AppendText(UObject* Self, const std::string& NewText);
	static void GetFirstTextPart(UObject* Self, std::string& OutText, int& ReturnValue);
	static void GetNextTextPart(UObject* Self, std::string& OutText, int& ReturnValue);
	static void GetText(UObject* Self, std::string& ReturnValue);
	static void GetTextLength(UObject* Self, int& ReturnValue);
	static void GetTextPart(UObject* Self, int startPos, int Count, std::string& OutText, int& ReturnValue);
	static void SetText(UObject* Self, const std::string& NewText);
};
