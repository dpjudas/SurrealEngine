
#pragma once

#include "Compiler/Ast/Ast.h"
#include "Compiler/Typesystem/TypeSystem.h"

class SemanticAnalysis;
class NameScope;

class MethodSema : AstNameVisitor
{
public:
	MethodSema(SemanticAnalysis& sema);
	void analyze(AstCompilationUnit* ast);

private:
	void name(AstClassDeclaration* node) override;
	void name(AstStructDeclaration* node) override;
	void name(AstEnumDeclaration* node) override;
	void name(AstEnumValueDeclaration* node) override;

	void name(AstConstantDeclaration* node) override;
	void name(AstFieldDeclaration* node) override;
	void name(AstMethodDeclaration* node) override;
	void name(AstOperatorDeclaration* node) override;
	void name(AstStateDeclaration* node) override;

	void add_parameters(NameScope& name_scope, const std::vector<AstMethodParameter*>& parameters = {}, bool is_static = false, Type* value_type = nullptr);

	SemanticAnalysis& sema;
	TypeScope type_scope;
	Type* this_type = nullptr;
};
