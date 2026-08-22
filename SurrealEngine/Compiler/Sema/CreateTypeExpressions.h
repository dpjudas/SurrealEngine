
#pragma once

#include "Compiler/Ast/Ast.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "SemanticAnalysis.h"

class SemanticAnalysis;
class NameScope;
class TypeConstantExpression;

class CreateTypeExpressions : AstNameVisitor
{
public:
	CreateTypeExpressions(SemanticAnalysis& sema);
	void exec(AstCompilationUnit* ast);

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

	TypeConstantExpression* create_expression(AstExpression* expression);

	SemanticAnalysis& sema;
	TypeScope type_scope;
	Type* this_type = nullptr;
};
