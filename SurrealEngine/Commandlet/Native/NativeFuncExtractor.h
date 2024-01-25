#pragma once

#include "JsonValue.h"

class Package;
class PackageManager;
class UClass;
class UFunction;

class NativeFuncExtractor
{
public:
	static std::string Run(PackageManager* packages);

private:
	static JsonValue CreatePackageJson(Package* package);
	static JsonValue CreateClassJson(UClass* cls);
	static JsonValue CreateFunctionJson(UFunction* func);
};
