#pragma once

#include <set>
#include "UE1GameDatabase.h"
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

	struct NativeFunctionDecl
	{
		std::string args;
		int argCount;
		std::vector<KnownUE1Games> games;
	};

	struct NativeFunction
	{
		std::string name;
		std::vector<NativeFunctionDecl> decls;
		std::vector<std::pair<KnownUE1Games, int>> versionIndex;
	};

	struct NativeProperty
	{
		std::string name;
		std::string type;
		std::vector<KnownUE1Games> games;
	};

	struct NativeClass
	{
		std::string name;
		std::string package;
		std::vector<NativeFunction> funcs;
		std::vector<NativeProperty> props;

		NativeFunction& AddUniqueNativeFunction(const std::string& funcName);
		NativeProperty& AddUniqueNativeProperty(const std::string& propName);

		void ParseClassFunction(const std::string& funcName, const JsonValue& json, KnownUE1Games knownGame);
	};

	static void ParseClassNatives(const std::string& className, const std::string& packageName, const JsonValue& json, KnownUE1Games knownGame);
	static void ParseClassProperties(const std::string& className, const std::string& packageName, const JsonValue& json, KnownUE1Games knownGame);
	static void ParseGameNatives(const JsonValue& json, KnownUE1Games knownGame);
	static void ParseGameProperties(const JsonValue& json, KnownUE1Games knownGame);

	static NativeClass& AddUniqueNativeClass(const std::string& className);

	static std::vector<NativeCppGenerator::NativeClass> classes;
};
