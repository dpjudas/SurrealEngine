
#include "Precomp.h"
#include "Codegen.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Ast/Ast.h"
#include "ClassDebugInfoBuilder.h"

CodeGen::CodeGen(TypeSystem& type_system) : type_system(type_system)
{
}

CodeGen::~CodeGen()
{
}

void CodeGen::codegen(std::vector<std::shared_ptr<AstCompilationUnit>> asts)
{
	for (auto& unit : asts)
	{
		sources_debug_info.push_back(generate_debug_info(unit));
	}
}

std::unique_ptr<ClassDebugInfo> CodeGen::generate_debug_info(std::shared_ptr<AstCompilationUnit> unit)
{
	return ClassDebugInfoBuilder::generate(unit);
}
