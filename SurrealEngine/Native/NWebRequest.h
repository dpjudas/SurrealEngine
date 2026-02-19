#pragma once

#include "UObject/UObject.h"

class NWebRequest
{
public:
	static void RegisterFunctions();

	static void AddVariable(UObject* Self, const std::string& VariableName, const std::string& Value);
	static void DecodeBase64(UObject* Self, const std::string& Encoded, std::string& ReturnValue);
	static void GetVariable(UObject* Self, const std::string& VariableName, std::string* DefaultValue, std::string& ReturnValue);
	static void GetVariableCount(UObject* Self, const std::string& VariableName, int& ReturnValue);
	static void GetVariableNumber(UObject* Self, const std::string& VariableName, int Number, std::string* DefaultValue, std::string& ReturnValue);
};
