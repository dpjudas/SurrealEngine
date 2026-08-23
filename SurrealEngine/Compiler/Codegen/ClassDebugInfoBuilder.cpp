
#include "Precomp.h"
#include "ClassDebugInfoBuilder.h"
#include "FunctionDebugInfoBuilder.h"

std::unique_ptr<ClassDebugInfo> ClassDebugInfoBuilder::generate(std::shared_ptr<AstCompilationUnit> unit)
{
	auto debug_info = std::make_unique<ClassDebugInfo>();
	debug_info->Name = unit->class_decl->identifier;
	ClassDebugInfoBuilder builder(debug_info.get());
	unit->class_decl->visit(&builder);
	return debug_info;
}

ClassDebugInfoBuilder::ClassDebugInfoBuilder(ClassDebugInfo* debug_info) : debug_info(debug_info)
{
}

void ClassDebugInfoBuilder::name(AstMethodDeclaration* node)
{
	FunctionDebugInfo funcDebugInfo;
	FunctionDebugInfoBuilder builder(&funcDebugInfo);

	if (node->block)
	{
		for (AstStatement* statement : node->block->statements)
		{
			statement->visit(&builder);
		}
	}

	if (funcDebugInfo.Statements.empty() || funcDebugInfo.Statements.back().Token != ExprToken::Return)
		funcDebugInfo.Statements.push_back({ ExprToken::Return, node->line });

	debug_info->Functions[node->identifier] = std::move(funcDebugInfo);
}

void ClassDebugInfoBuilder::name(AstClassDeclaration* node)
{
}

void ClassDebugInfoBuilder::name(AstStructDeclaration* node)
{
}

void ClassDebugInfoBuilder::name(AstEnumDeclaration* node)
{
}

void ClassDebugInfoBuilder::name(AstEnumValueDeclaration* node)
{
}

void ClassDebugInfoBuilder::name(AstConstantDeclaration* node)
{
}

void ClassDebugInfoBuilder::name(AstFieldDeclaration* node)
{
}

void ClassDebugInfoBuilder::name(AstStateDeclaration* node)
{
}
