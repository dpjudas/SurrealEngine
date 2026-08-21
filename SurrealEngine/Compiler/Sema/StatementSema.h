
#pragma once

#include "Compiler/Ast/Ast.h"

class SemanticAnalysis;
class TypeScope;

class StatementSema : AstStatementVisitor
{
public:
	StatementSema(SemanticAnalysis& sema, NameScope& name_scope, TypeScope& type_scope);
	void analyze(AstBlockStatement* node);

private:
	void statement(AstLabeledStatement* node);
	void statement(AstConstantDeclarationStatement* node);
	void statement(AstVariableDeclarationStatement* node);
	void statement(AstBlockStatement* node);
	void statement(AstEmptyStatement* node);
	void statement(AstExpressionStatement* node);
	void statement(AstIfStatement* node);
	void statement(AstSwitchStatement* node);
	void statement(AstWhileStatement* node);
	void statement(AstDoStatement* node);
	void statement(AstForStatement* node);
	void statement(AstForeachStatement* node);
	void statement(AstBreakStatement* node);
	void statement(AstContinueStatement* node);
	void statement(AstGotoStatement* node);
	void statement(AstReturnStatement* node);

	void local_variable_declaration(AstLocalVariableDeclaration* variable_declaration);

	SemanticAnalysis& sema;
	NameScope& name_scope;
	TypeScope& type_scope;

	int loop_level = 0;
};
