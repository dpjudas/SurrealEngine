
#pragma once

#include <string>
#include <vector>

// #include "Compiler/Codegen/Codegen.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "CompilerMessage.h"

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

private:
	std::vector<SourceFile> sources;
	TypeSystem type_system;
	//std::unique_ptr<CodeGen> codegen;
	std::vector<CompilerMessage> messages;
};
