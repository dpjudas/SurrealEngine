
#include "Precomp.h"
#include "FunctionDebugInfoBuilder.h"

FunctionDebugInfoBuilder::FunctionDebugInfoBuilder(FunctionDebugInfo* funcDebugInfo) : funcDebugInfo(funcDebugInfo)
{
}

void FunctionDebugInfoBuilder::statement(AstLabeledStatement* node)
{
}

void FunctionDebugInfoBuilder::statement(AstConstantDeclarationStatement* node)
{
}

void FunctionDebugInfoBuilder::statement(AstVariableDeclarationStatement* node)
{
	if (node->variable_declaration)
	{
		for (AstLocalVariableDeclarator* decl : node->variable_declaration->variables)
		{
			if (decl->expression)
				funcDebugInfo->Statements.push_back({ ExprToken::Skip, decl->expression->line });
		}
	}
}

void FunctionDebugInfoBuilder::statement(AstBlockStatement* node)
{
	for (AstStatement* statement : node->statements)
	{
		statement->visit(this);
	}
}

void FunctionDebugInfoBuilder::statement(AstEmptyStatement* node)
{
}

void FunctionDebugInfoBuilder::statement(AstExpressionStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Skip, node->line });
}

void FunctionDebugInfoBuilder::statement(AstIfStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->line });
	if (node->then_statement)
	{
		node->then_statement->visit(this);
		if (node->else_statement)
		{
			funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
		}
	}
	if (node->else_statement)
	{
		node->else_statement->visit(this);
	}
}

void FunctionDebugInfoBuilder::statement(AstSwitchStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Switch, node->line });
	for (AstSwitchSection* section : node->sections)
	{
		for (AstSwitchLabel* label : section->labels)
		{
			funcDebugInfo->Statements.push_back({ ExprToken::Case, label->line });
		}
		for (AstStatement* statement : section->statements)
		{
			statement->visit(this);
		}
	}
}

void FunctionDebugInfoBuilder::statement(AstWhileStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->line });
	if (node->statement)
		node->statement->visit(this);
	funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
}

void FunctionDebugInfoBuilder::statement(AstDoStatement* node)
{
	if (node->statement)
		node->statement->visit(this);
	funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->line });
}

void FunctionDebugInfoBuilder::statement(AstForStatement* node)
{
	if (node->init_variable_declaration)
	{
		for (AstLocalVariableDeclarator* decl : node->init_variable_declaration->variables)
		{
			if (decl->expression)
				funcDebugInfo->Statements.push_back({ ExprToken::Skip, decl->expression->line });
		}
	}
	for (AstExpression* expr : node->init_expressions)
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Skip, node->line });
	}

	if (node->condition)
		funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->condition->line });

	if (node->statement)
		node->statement->visit(this);

	for (AstExpression* expr : node->iterator_expressions)
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Skip, expr->line });
	}
	funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
}

void FunctionDebugInfoBuilder::statement(AstForeachStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Iterator, node->line });
	if (node->statement)
	{
		node->statement->visit(this);
	}
	funcDebugInfo->Statements.push_back({ ExprToken::IteratorNext, node->line });
	funcDebugInfo->Statements.push_back({ ExprToken::IteratorPop, node->line });
}

void FunctionDebugInfoBuilder::statement(AstBreakStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
}

void FunctionDebugInfoBuilder::statement(AstContinueStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
}

void FunctionDebugInfoBuilder::statement(AstGotoStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
}

void FunctionDebugInfoBuilder::statement(AstReturnStatement* node)
{
	funcDebugInfo->Statements.push_back({ ExprToken::Return, node->line });
}
