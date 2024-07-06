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
		Array<std::string> games;
	};

	struct NativeFunction
	{
		std::string name;
		Array<NativeFunctionDecl> decls;
		Array<std::pair<std::string, int>> versionIndex;
	};

	struct NativeProperty
	{
		std::string name;
		std::string type;
		Array<std::string> games;
	};

	struct NativeClass
	{
		std::string name;
		std::string package;
		Array<NativeFunction> funcs;
		Array<NativeProperty> props;

		NativeFunction& AddUniqueNativeFunction(const std::string& funcName);
		NativeProperty& AddUniqueNativeProperty(const std::string& propName);

		void ParseClassFunction(const std::string& funcName, const JsonValue& json, const std::string& version);
	};

	static void ParseClassNatives(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	static void ParseClassProperties(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	static void ParseGameNatives(const JsonValue& json, const std::string& version);
	static void ParseGameProperties(const JsonValue& json, const std::string& version);

	static NativeClass& AddUniqueNativeClass(const std::string& className);

	static Array<NativeCppGenerator::NativeClass> classes;
};
