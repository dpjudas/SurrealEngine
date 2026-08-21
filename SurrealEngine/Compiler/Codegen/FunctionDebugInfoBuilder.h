#pragma once

#include "Compiler/Ast/Ast.h"
#include "Packages/Core/UTextBuffer.h"

class FunctionDebugInfoBuilder : public AstStatementVisitor
{
public:
	FunctionDebugInfoBuilder(FunctionDebugInfo* funcDebugInfo);

	void statement(AstLabeledStatement* node) override;
	void statement(AstConstantDeclarationStatement* node) override;
	void statement(AstVariableDeclarationStatement* node) override;
	void statement(AstBlockStatement* node) override;
	void statement(AstEmptyStatement* node) override;
	void statement(AstExpressionStatement* node) override;
	void statement(AstIfStatement* node) override;
	void statement(AstSwitchStatement* node) override;
	void statement(AstWhileStatement* node) override;
	void statement(AstDoStatement* node) override;
	void statement(AstForStatement* node) override;
	void statement(AstForeachStatement* node) override;
	void statement(AstBreakStatement* node) override;
	void statement(AstContinueStatement* node) override;
	void statement(AstGotoStatement* node) override;
	void statement(AstReturnStatement* node) override;

private:
	FunctionDebugInfo* funcDebugInfo = nullptr;
};
