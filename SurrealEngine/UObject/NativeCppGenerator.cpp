
#include "Precomp.h"
#include "NativeCppGenerator.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "File.h"
#include <filesystem>
#include <set>

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

	// Write native functions
	for (NativeClass cls : classes)
	{
		
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
			ParseClassNatives(cls.first, cls.second);
		}
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
