#pragma once

#include <set>
#include "UE1GameDatabase.h"
#include "Utils/JsonValue.h"

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

	struct NativeFunctionDecl
	{
		std::string args;
		int argCount;
		std::vector<std::string> games;
	};

	struct NativeFunction
	{
		std::string name;
		std::vector<NativeFunctionDecl> decls;
		std::vector<std::pair<std::string, int>> versionIndex;
	};

	struct NativeProperty
	{
		std::string name;
		std::string type;
		std::vector<std::string> games;
	};

	struct NativeClass
	{
		std::string name;
		std::string package;
		std::vector<NativeFunction> funcs;
		std::vector<NativeProperty> props;

		NativeFunction& AddUniqueNativeFunction(const std::string& funcName);
		NativeProperty& AddUniqueNativeProperty(const std::string& propName);

		void ParseClassFunction(const std::string& funcName, const JsonValue& json, const std::string& version);
	};

	static void ParseClassNatives(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	static void ParseClassProperties(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	static void ParseGameNatives(const JsonValue& json, const std::string& version);
	static void ParseGameProperties(const JsonValue& json, const std::string& version);

	static NativeClass& AddUniqueNativeClass(const std::string& className);

	static std::vector<NativeCppGenerator::NativeClass> classes;
};
