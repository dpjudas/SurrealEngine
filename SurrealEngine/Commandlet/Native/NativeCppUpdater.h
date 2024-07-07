#pragma once

#include <filesystem>
#include <unordered_set>

class DebuggerApp;
class JsonValue;

class NativeCppUpdater
{
public:
	NativeCppUpdater(DebuggerApp* console) : Console(console) { }

	void Run();

private:
	struct NativeFunctionDecl
	{
		std::string args;
		Array<std::string> games;
		int argCount = 0;
	};

	struct NativeFunction
	{
		std::string name;
		Array<NativeFunctionDecl> decls;
		Array<std::pair<std::string, int>> versionIndex;
		bool staticFlag = false;
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

	bool FindSourceCode();
	void ParseJsonFiles();
	void ParseClassNatives(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	void ParseClassProperties(const std::string& className, const std::string& packageName, const JsonValue& json, const std::string& version);
	void ParseGameNatives(const JsonValue& json, const std::string& version);
	void ParseGameProperties(const JsonValue& json, const std::string& version);

	NativeClass& AddUniqueNativeClass(const std::string& className);

	std::string UpdateBlock(const std::string& args, const std::string& block, const std::string& whitespaceprefix);

	static std::unordered_set<std::string> ExtractBlockLines(const std::string& block);
	static std::string TrimWhitespace(const std::string& text);

	DebuggerApp* Console = nullptr;
	std::filesystem::path SourceBasePath;
	Array<NativeClass> Classes;
};
