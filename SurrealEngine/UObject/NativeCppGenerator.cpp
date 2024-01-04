
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
		version = file.substr(dashPos + 1, jsonTypePos - dashPos);

		int subversion = 0;
		if (isalpha(*version.end()))
			subversion = *version.end() - '`';

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
			ParseClassNatives(cls.first, cls.second, version, subversion);
		}
	}
}

void NativeCppGenerator::ParseClassNatives(const std::string& className, const JsonValue& json, const int version, const int subversion)
{
	NativeClass& cls = AddUniqueNativeClass(className);
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const std::string& funcName = (*prop).first;
		const JsonValue& funcJson = (*prop).second;
		cls.ParseClassFunction(funcName, funcJson, version, subversion);
	}
}

void NativeCppGenerator::ParseGameProperties(const JsonValue& json, const std::string& game, const int version, const int subversion)
{
	const std::map<std::string, JsonValue>& props = json.properties();
	for (auto prop = props.begin(); prop != props.end(); prop++)
	{
		const std::string& packageName = (*prop).first;
		const JsonValue& packageJson = (*prop).second;

		
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


void NativeCppGenerator::NativeClass::ParseClassFunction(const std::string& funcName, const JsonValue& json, const int version, const int subversion)
{
	NativeFunction& func = AddUniqueNativeFunction(funcName);
	func.AddVersionIndex(version, subversion, json["NativeFuncIndex"].to_int());

	for (auto prop : json.properties())
	{
		if (prop.first.compare("NativeFuncIndex") == 0)
			continue;

		func.args += prop.second.to_string() + " " + prop.first + ", ";
	}

	func.args.resize(func.args.size() - 2);
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

void NativeCppGenerator::NativeFunction::AddVersionIndex(const int version, const int subversion, const int index)
{
	std::pair<KnownUE1Games, int> vi;
	
	switch (version)
	{
		case 226:
		{
			if (subversion == 2)
				vi.first = KnownUE1Games::UNREALGOLD_226b;
			break;
		}
		case 436:
		{
			vi.first = KnownUE1Games::UT99_436;
			break;
		}
	}

	vi.second = index;
}
