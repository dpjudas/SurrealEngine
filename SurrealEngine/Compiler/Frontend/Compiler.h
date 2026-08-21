
#pragma once

#include <string>
#include <vector>

#include "Compiler/Codegen/Codegen.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "CompilerMessage.h"

class ClassDebugInfo;

class SourceFile
{
public:
	SourceFile(const std::string &code, const std::string &filename) : code(code), filename(filename) { }

	std::string code;
	std::string filename;
};

class Compiler
{
public:
	Compiler();
	~Compiler();

	void add_code(const std::string &code, const std::string &filename);
	bool compile();

	const std::vector<CompilerMessage> &get_messages() const { return messages; }
	std::unique_ptr<ClassDebugInfo> move_debug_info(int index);

private:
	void logInfo(const std::string& text);

	std::vector<SourceFile> sources;
	TypeSystem type_system;
	std::unique_ptr<CodeGen> codegen;
	std::vector<CompilerMessage> messages;

	friend class StatementPrinter;
};
