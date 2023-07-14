#pragma once

class PackageManager;
class UClass;
class UFunction;

class NativeFuncExtractor
{
public:
	static std::string Run(PackageManager* packages);

private:
	static std::string WriteClassHeader(UClass* cls);
	static std::string WriteClassBody(UClass* cls);
	static std::string WriteFunctionSignature(UFunction* func);
};
