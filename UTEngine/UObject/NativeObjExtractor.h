#pragma once

class PackageManager;
class Package;
class UClass;
class UProperty;

class NativeObjExtractor
{
public:
	static std::string Run(PackageManager* packages);

private:
	static std::string WriteStructHeader(UClass* cls);
	static std::string WriteClassHeader(UClass* cls);
	static std::string WriteStructBody(Package* package, UClass* cls);
	static std::string WritePropertyGetter(const std::string& clsname, UProperty* prop);
};
