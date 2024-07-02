
#include "Precomp.h"
#include "NativeCppGenerator.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "Utils/File.h"
#include <filesystem>
#include <set>

std::vector<NativeCppGenerator::NativeClass> NativeCppGenerator::classes;

void NativeCppGenerator::Run()
{
	classes.clear();

	std::string game;
	std::string version;

	for (std::string file : Directory::files(FilePath::combine(std::filesystem::current_path().string(), "*.json")))
	{
		size_t jsonTypePos = file.find("-Natives");
		bool parseNatives = true;
		if (jsonTypePos == std::string::npos)
		{
			parseNatives = false;
			jsonTypePos = file.find("-Properties");
			if (jsonTypePos == std::string::npos)
				continue;
		}

		size_t dashPos = file.find("-");
		game = file.substr(0, dashPos);
		version = file.substr(dashPos + 1, jsonTypePos - dashPos - 1);

		JsonValue json = JsonValue::parse(File::read_all_text(file));

		if (parseNatives)
			ParseGameNatives(json, version);
		else
			ParseGameProperties(json, version);
	}

	Directory::make_directory("Cpp");
	Directory::make_directory("Cpp/Native");
	Directory::make_directory("Cpp/Package");
	Directory::make_directory("Cpp/UObject");

	std::string packageManagerRegisterFuncsText;
	std::string propertyOffsetsHText;
	std::string propertyOffsetsCppText;

	packageManagerRegisterFuncsText += "\t// Copy/paste this into the PackageManager constructor\r\n";

	propertyOffsetsHText += "#pragma once\r\n\r\n";
	propertyOffsetsHText += "class PackageManager;\r\n\r\n";
	propertyOffsetsHText += "void InitPropertyOffsets(PackageManager * packages); \r\n\r\n";
	propertyOffsetsHText += "struct PropertyDataOffset\r\n{\r\n\tsize_t DataOffset = ~(size_t)0;\r\n\tuint32_t BitfieldMask = 1;\r\n};\r\n\r\n";

	propertyOffsetsCppText += "#include \"Precomp.h\"\r\n";
	propertyOffsetsCppText += "#include \"PropertyOffsets.h\"\r\n";
	propertyOffsetsCppText += "#include \"Package/PackageManager.h\"\r\n";
	propertyOffsetsCppText += "#include \"UClass.h\"\r\n";
	propertyOffsetsCppText += "#include \"UProperty.h\"\r\n\r\n";

	for (auto cls : classes)
	{
		std::string nClassCppText;
		std::string nClassHText;
		std::string nClassCppRegisterFunctionsText;
		std::string nClassCppFunctionImplsText;

		// Write native function info
		if (cls.funcs.size() > 0)
		{
			nClassHText += "#pragma once\r\n\r\n";
			nClassHText += "#include \"UObject/UObject.h\"\r\n\r\n";

			nClassHText += "class N" + cls.name;
			nClassHText += "\r\n{\r\npublic:\r\n\tstatic void RegisterFunctions();\r\n\r\n";

			nClassCppText += "#include \"Precomp.h\"\r\n";
			nClassCppText += "#include \"N" + cls.name + "\"\r\n";
			nClassCppText += "#include \"VM/NativeFunc.h\"\r\n";
			nClassCppText += "#include \"Engine.h\"\r\n\r\n";

			nClassCppText += "void N" + cls.name + "::RegisterFunctions(const std::map<std::string, int>& funcIndexMap)\r\n{\r\n";

			for (auto func : cls.funcs)
			{
				for (int i = 0; i < func.decls.size(); i++)
				{
					NativeFunctionDecl& decl = func.decls[i];
					std::string funcName = func.name;
					if (i > 0)
						funcName += "_" + std::to_string(i);

					std::string cppFuncName = "N" + cls.name + "::" + funcName;
					std::string cppFuncDecl = "void " + cppFuncName + "(" + decl.args + ")";

					nClassCppRegisterFunctionsText += "\tRegisterVMNativeFunc_" + std::to_string(decl.argCount) + "(\"" + cls.name + "\", \"" + func.name + "\", &" + cppFuncName + ", funcIndexMap[\"" + cppFuncName + "\"]);\r\n";
					nClassCppFunctionImplsText += "\r\n" + cppFuncDecl + "\r\n{\r\n\tException::Throw(\"" + cppFuncName + " not implemented\");\r\n}\r\n";
					nClassHText += "static void " + funcName + "(" + decl.args + ")" + ";\r\n";
				}
			}

			nClassHText += "};\r\n";

			nClassCppText += nClassCppRegisterFunctionsText;
			nClassCppText += nClassCppFunctionImplsText;

			File::write_all_text("Cpp/Native/N" + cls.name + ".cpp", nClassCppText);
			File::write_all_text("Cpp/Native/N" + cls.name + ".h", nClassHText);
		}

		// Write native property info
		packageManagerRegisterFuncsText += "\tN" + cls.name + "::RegisterFunctions(funcIndexMap);\r\n";
		std::string propOffsetsVarName = "PropOffsets_" + cls.name;
		std::string propOffsetsStructName = "PropertyOffsets_" + cls.name;
		std::string propOffsetsVarDecl = propOffsetsStructName + " " + propOffsetsVarName + ";\r\n\r\n";

		propertyOffsetsCppText += propOffsetsVarDecl;
		propertyOffsetsCppText += "static void InitPropertyOffsets_" + cls.name + "(PackageManager* packages)\r\n{\r\n";
		propertyOffsetsCppText += "\tUClass* cls = UObject::TryCast<UClass>(packages->GetPackage(\"" + cls.package + "\")->GetUObject(\"Class\", \"" + cls.name + "\"));\r\n";
		propertyOffsetsCppText += "\tif (!cls)\r\n\t{\r\n";
		propertyOffsetsCppText += "\t\tmemset(&" + propOffsetsVarName + ", 0xff, sizeof(" + propOffsetsVarName + "));\r\n";
		propertyOffsetsCppText += "\t\treturn;\r\n\t}\r\n";

		propertyOffsetsHText += "struct " + propOffsetsStructName + "\r\n{\r\n";

		for (auto prop : cls.props)
		{
			propertyOffsetsCppText += "\t" + propOffsetsVarName + "." + prop.name + " = cls->GetPropertyDataOffset(\"" + prop.name + "\");\r\n";
			propertyOffsetsHText += "\tPropertyDataOffset " + prop.name + ";\r\n";
		}

		propertyOffsetsCppText += "}\r\n\r\n";
		propertyOffsetsHText += "}\r\n\r\nextern " + propOffsetsVarDecl;
	}

	File::write_all_text("Cpp/Package/PackageManager_RegisterFuncs.cpp", packageManagerRegisterFuncsText);
	File::write_all_text("Cpp/UObject/PropertyOffsets.cpp", propertyOffsetsCppText);
	File::write_all_text("Cpp/UObject/PropertyOffsets.h", propertyOffsetsHText);
}

void NativeCppGenerator::ParseGameNatives(const JsonValue& json, const std::string& version)
{
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const JsonValue& package = (*prop).second;
		for (auto cls : package.properties())
		{
			ParseClassNatives(cls.first, (*prop).first, cls.second, version);
		}
	}
}

void NativeCppGenerator::ParseClassNatives(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version)
{
	NativeClass& cls = AddUniqueNativeClass(className);

	// Hopefully we never run into this scenario :)
	// If we do, we'll have to figure out a way to address this
	if (cls.package.size() > 0 && cls.package != packageName)
		Exception::Throw("Class package mismatch between games, got " + cls.package + "first, then " + packageName);

	cls.package = packageName;

	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const std::string& funcName = (*prop).first;
		const JsonValue& funcJson = (*prop).second;
		cls.ParseClassFunction(funcName, funcJson, version);
	}
}

void NativeCppGenerator::ParseGameProperties(const JsonValue& json, const std::string& version)
{
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const JsonValue& package = (*prop).second;
		for (auto cls : package.properties())
		{
			ParseClassProperties(cls.first, (*prop).first, cls.second, version);
		}
	}
}

void NativeCppGenerator::ParseClassProperties(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version)
{
	NativeClass& cls = AddUniqueNativeClass(className);

	// Hopefully we never run into this scenario :)
	// If we do, we'll have to figure out a way to address this
	if (cls.package.size() > 0 && cls.package != packageName)
		Exception::Throw("Class package mismatch between games, got " + cls.package + "first, then " + packageName);

	cls.package = packageName;

	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		NativeProperty& nativeProp = cls.AddUniqueNativeProperty((*prop).first);
		nativeProp.type = (*prop).second.to_string();
		nativeProp.games.push_back(version);
	}
}

NativeCppGenerator::NativeClass& NativeCppGenerator::AddUniqueNativeClass(const std::string& className)
{
	for (int i = 0; i < classes.size(); i++)
	{
		NativeClass& cls = classes[i];
		if (className == cls.name)
			return cls;
	}

	NativeClass cls;
	cls.name = className;
	classes.push_back(std::move(cls));
	return classes.back();
}

void NativeCppGenerator::NativeClass::ParseClassFunction(const std::string& funcName, const JsonValue& json, const std::string& version)
{
	std::string funcArgs = "UObject* Self";
	const std::vector<JsonValue>& args = json["Arguments"].items();
	if (args.size() > 0)
	{
		// Assemble function arguments
		for (auto arg : args)
		{
			funcArgs += ", " + arg.to_string();
		}
	}

	NativeFunction& func = AddUniqueNativeFunction(funcName);
	func.versionIndex.push_back(std::make_pair(version, json["NativeFuncIndex"].to_int()));

	if (func.decls.size() > 0)
	{
		// Some games (Deus Ex for example) only change the capitalization of argument names
		// Lowercase the strings and check those so we don't add another decl unnecessarily
		std::string funcArgsLower;
		funcArgsLower.resize(funcArgs.size());
		std::transform(funcArgs.begin(), funcArgs.end(), funcArgsLower.begin(), [](unsigned char c) { return tolower(c); });

		// Need to check all existing function declarations.
		// Games can have different versions of the same function.
		for (auto decl : func.decls)
		{
			std::string declArgsLower;
			declArgsLower.resize(decl.args.size());
			std::transform(decl.args.begin(), decl.args.end(), declArgsLower.begin(), [](unsigned char c) {return tolower(c); });

			if (funcArgsLower == declArgsLower)
			{
				decl.games.push_back(version);
				return;
			}
		}
	}

	// Add a new declaration
	NativeFunctionDecl decl;
	decl.args = funcArgs;
	decl.games.push_back(version);
	decl.argCount = (int)args.size();
	func.decls.push_back(std::move(decl));
}

NativeCppGenerator::NativeFunction& NativeCppGenerator::NativeClass::AddUniqueNativeFunction(const std::string& funcName)
{
	for (int i = 0; i < funcs.size(); i++)
	{
		NativeFunction& func = funcs[i];
		if (funcName == func.name)
			return func;
	}

	NativeFunction func;
	func.name = funcName;
	funcs.push_back(std::move(func));
	return funcs.back();
}

NativeCppGenerator::NativeProperty& NativeCppGenerator::NativeClass::AddUniqueNativeProperty(const std::string& propName)
{
	for (int i = 0; i < props.size(); i++)
	{
		NativeProperty& prop = props[i];
		if (propName == prop.name)
			return prop;
	}

	NativeProperty prop;
	prop.name = propName;
	props.push_back(std::move(prop));
	return props.back();
}

