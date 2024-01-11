#pragma once

#include "JsonValue.h"

class PackageManager;
class Package;
class UClass;
class UProperty;
class NameString;

class NativeObjExtractor
{
public:
	static std::string Run(PackageManager* packages);

private:
	static JsonValue CreatePackageJson(Package* package);
	static JsonValue CreateClassJson(UClass* cls);
};
