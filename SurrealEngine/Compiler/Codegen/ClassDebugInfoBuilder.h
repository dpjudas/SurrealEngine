#pragma once

#include "Compiler/Ast/Ast.h"
#include "Packages/Core/UTextBuffer.h"

class ClassDebugInfoBuilder : public AstNameVisitor
{
public:
	static std::unique_ptr<ClassDebugInfo> generate(std::shared_ptr<AstCompilationUnit> unit);

	ClassDebugInfoBuilder(ClassDebugInfo* debug_info);

	void name(AstClassDeclaration* node) override;
	void name(AstStructDeclaration* node) override;
	void name(AstEnumDeclaration* node) override;
	void name(AstEnumValueDeclaration* node) override;

	void name(AstConstantDeclaration* node) override;
	void name(AstFieldDeclaration* node) override;
	void name(AstMethodDeclaration* node) override;
	void name(AstOperatorDeclaration* node) override;
	void name(AstStateDeclaration* node) override;

private:
	ClassDebugInfo* debug_info = nullptr;
};
