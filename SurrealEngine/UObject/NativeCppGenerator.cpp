
#include "Precomp.h"
#include "NativeCppGenerator.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "File.h"
#include <filesystem>
#include <set>

std::vector<NativeCppGenerator::NativeClass> NativeCppGenerator::classes;

static KnownUE1Games GetKnownGame(const std::string& game, const int version, const int subversion)
{
	switch (version)
	{
		case 200:
		{
			if (game == "TnnHunt")
				return KnownUE1Games::TNN_200;
			else
				return KnownUE1Games::UNREAL_200;
			break;
		}
		case 219:
		{
			if (game == "Klingons")
				return KnownUE1Games::KLINGON_219;
			else
				return KnownUE1Games::UNREAL_219;
			break;
		}
		case 226:
		{
			if (subversion == 2)
				return KnownUE1Games::UNREALGOLD_226b;
			else
				return KnownUE1Games::UNREAL_226f;
			break;
		}
		case 436:
		{
			return KnownUE1Games::UT99_436;
			break;
		}
		case 451:
		{
			return KnownUE1Games::UT99_451;
			break;
		}
		case 469:
		{
			if (subversion == 1)
				return KnownUE1Games::UT99_469a;
			else if (subversion == 2)
				return KnownUE1Games::UT99_469b;
			else if (subversion == 3)
				return KnownUE1Games::UT99_469c;
			else
				return KnownUE1Games::UT99_469d;
			break;
		}
		case 1112:
		{
			return KnownUE1Games::DEUS_EX_1112fm;
			break;
		}
	}

	throw std::runtime_error("Unknown game - Game: " + game + ", Version: " + std::to_string(version) + " , Subversion: " + std::to_string(subversion));
}

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

		int subversion = 0;
		if (isalpha(version.back()))
			subversion = version.back() - '`';

		JsonValue json = JsonValue::parse(File::read_all_text(file));

		if (parseNatives)
			ParseGameNatives(json, game, std::stoi(version), subversion);
		else
			ParseGameProperties(json, game, std::stoi(version), subversion);
	}

	
}

void NativeCppGenerator::ParseGameNatives(const JsonValue& json, const std::string& game, const int version, const int subversion)
{
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const JsonValue& package = (*prop).second;
		for (auto cls : package.properties())
		{
			ParseClassNatives(cls.first, cls.second, game, version, subversion);
		}
	}
}

void NativeCppGenerator::ParseClassNatives(const std::string& className, const JsonValue& json, const std::string& game, const int version, const int subversion)
{
	NativeClass& cls = AddUniqueNativeClass(className);
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const std::string& funcName = (*prop).first;
		const JsonValue& funcJson = (*prop).second;
		cls.ParseClassFunction(funcName, funcJson, game, version, subversion);
	}
}

void NativeCppGenerator::ParseGameProperties(const JsonValue& json, const std::string& game, const int version, const int subversion)
{
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const JsonValue& package = (*prop).second;
		for (auto cls : package.properties())
		{
			ParseClassProperties(cls.first, cls.second, game, version, subversion);
		}
	}
}

void NativeCppGenerator::ParseClassProperties(const std::string& className, const JsonValue& json, const std::string& game, const int version, const int subversion)
{
	NativeClass& cls = AddUniqueNativeClass(className);
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		NativeProperty& nativeProp = cls.AddUniqueNativeProperty((*prop).first);
		nativeProp.type = (*prop).second.to_string();
		nativeProp.games.push_back(GetKnownGame(game, version, subversion));
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

void NativeCppGenerator::NativeClass::ParseClassFunction(const std::string& funcName, const JsonValue& json, const std::string& game, const int version, const int subversion)
{
	std::string funcArgs = "UObject* Self";
	KnownUE1Games knownGame = GetKnownGame(game, version, subversion);

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
	func.versionIndex.push_back(std::make_pair(knownGame, json["NativeFuncIndex"].to_int()));

	if (func.decls.size() > 0)
	{
		// Need to check all existing function declarations.
		// Games can have different versions of the same function.
		for (auto decl : func.decls)
		{
			if (funcArgs == decl.args)
			{
				decl.games.push_back(knownGame);
				return;
			}
		}
	}

	// Add a new declaration
	NativeFunctionDecl decl;
	decl.args = funcArgs;
	decl.games.push_back(knownGame);
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

