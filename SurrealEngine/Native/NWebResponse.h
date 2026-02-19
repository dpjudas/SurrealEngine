#pragma once

#include "UObject/UObject.h"

class NWebResponse
{
public:
	static void RegisterFunctions();

	static void ClearSubst(UObject* Self);
	static void IncludeBinaryFile(UObject* Self, const std::string& Filename);
	static void IncludeUHTM(UObject* Self, const std::string& Filename);
	static void Subst(UObject* Self, const std::string& Variable, const std::string& Value, BitfieldBool* bClear);
};
