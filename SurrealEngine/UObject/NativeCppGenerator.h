#pragma once

#include "JsonValue.h"

class PackageManager;
class Package;
class UClass;
class UProperty;
class NameString;

class NativeCppGenerator
{
public:
	static void Run();

private:

	struct NativeFunction
	{
		std::string name;
		std::string args;
		int index;
		std::vector<int> versions;
	};

	struct NativeProperty
	{
		std::string name;
		std::string type;
	};

	struct NativeClass
	{
		std::string name;
		std::vector<NativeFunction> funcs;
		std::vector<NativeProperty> props;
	};

	static void ParseGameNatives(const JsonValue& json, const std::string& game, const int version, const int subversion);
	static std::string ParseClassNatives(const std::string& className, const JsonValue& json);

	static void ParseGameProperties(const JsonValue& json, const std::string& game, const int version, const int subversion);
	static std::string GenerateClassProperties(const std::string& className, const JsonValue& json);

	static std::vector<NativeClass> classes;
};
