#pragma once

#include "Packages/Core/UTextBuffer.h"

class TypeSystem;
class AstCompilationUnit;

class CodeGen
{
public:
	CodeGen(TypeSystem& type_system);
	~CodeGen();

	void codegen(std::vector<std::shared_ptr<AstCompilationUnit>> asts);

	std::vector<std::unique_ptr<ClassDebugInfo>> sources_debug_info;

private:
	std::unique_ptr<ClassDebugInfo> generate_debug_info(std::shared_ptr<AstCompilationUnit> unit);

	TypeSystem& type_system;
};
