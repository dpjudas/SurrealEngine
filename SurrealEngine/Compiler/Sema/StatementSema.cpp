
#include "Precomp.h"
#include "StatementSema.h"
#include "ExpressionSema.h"
#include "ResolveExpressionConstants.h"
#include "SemanticAnalysis.h"
#include "Compiler/Typesystem/Type.h"
#include "Compiler/Typesystem/TypeSystem.h"

StatementSema::StatementSema(SemanticAnalysis& sema, NameScope& name_scope, TypeScope& type_scope) : sema(sema), name_scope(name_scope), type_scope(type_scope)
{
}

void StatementSema::analyze(AstBlockStatement* node)
{
	node->visit_children(this);
}

void StatementSema::statement(AstLabeledStatement* node)
{
	//node->statement->visit(this);
}

void StatementSema::statement(AstConstantDeclarationStatement* node)
{
	throw SemaException("constant declaration statements not supported", node);
}

void StatementSema::statement(AstVariableDeclarationStatement* node)
{
	local_variable_declaration(node->variable_declaration);
}

void StatementSema::statement(AstBlockStatement* node)
{
	node->visit_children(this);
}

void StatementSema::statement(AstEmptyStatement* node)
{
}

void StatementSema::statement(AstExpressionStatement* node)
{
	ExpressionSema expression(sema, name_scope, type_scope);
	expression.analyze(node->expression);

	ResolveExpressionConstants resolve_constants(sema);
	resolve_constants.analyze(node->expression);
}

void StatementSema::statement(AstIfStatement* node)
{
	ExpressionSema expression(sema, name_scope, type_scope);
	expression.analyze(node->boolean_expression);

	ResolveExpressionConstants resolve_constants(sema);
	resolve_constants.analyze(node->boolean_expression);

	auto& ts = sema.type_system();
	if (!ts.implicit_convert_allowed(node->boolean_expression->result.type, ts.boolean_type, false))
		throw SemaException("expression cannot be implicitly cast to bool", node);

	node->then_statement->visit(this);
	if (node->else_statement)
		node->else_statement->visit(this);
}

void StatementSema::statement(AstSwitchStatement* node)
{
	throw SemaException("switch statements not implemented", node);
}

void StatementSema::statement(AstWhileStatement* node)
{
	ExpressionSema expression(sema, name_scope, type_scope);
	expression.analyze(node->boolean_expression);

	ResolveExpressionConstants resolve_constants(sema);
	resolve_constants.analyze(node->boolean_expression);

	auto& ts = sema.type_system();
	if (!ts.implicit_convert_allowed(node->boolean_expression->result.type, ts.boolean_type, false))
		throw SemaException("expression cannot be implicitly cast to bool", node);

	loop_level++;
	node->statement->visit(this);
	loop_level--;
}

void StatementSema::statement(AstDoStatement* node)
{
	loop_level++;
	node->statement->visit(this);
	loop_level--;

	ExpressionSema expression(sema, name_scope, type_scope);
	expression.analyze(node->boolean_expression);

	ResolveExpressionConstants resolve_constants(sema);
	resolve_constants.analyze(node->boolean_expression);

	auto& ts = sema.type_system();
	if (!ts.implicit_convert_allowed(node->boolean_expression->result.type, ts.boolean_type, false))
		throw SemaException("expression cannot be implicitly cast to bool", node);
}

void StatementSema::statement(AstForStatement* node)
{
	ExpressionSema expression(sema, name_scope, type_scope);
	ResolveExpressionConstants resolve_constants(sema);

	if (node->init_variable_declaration)
	{
		local_variable_declaration(node->init_variable_declaration);
	}

	for (size_t i = 0; i < node->init_expressions.size(); i++)
	{
		expression.analyze(node->init_expressions[i]);
		resolve_constants.analyze(node->init_expressions[i]);
	}

	expression.analyze(node->condition);
	resolve_constants.analyze(node->condition);

	auto& ts = sema.type_system();
	if (!ts.implicit_convert_allowed(node->condition->result.type, ts.boolean_type, false))
		throw SemaException("expression cannot be implicitly cast to bool", node);

	loop_level++;
	node->statement->visit(this);
	loop_level--;

	for (size_t i = 0; i < node->iterator_expressions.size(); i++)
	{
		expression.analyze(node->iterator_expressions[i]);
	}
}

void StatementSema::statement(AstForeachStatement* node)
{
	throw SemaException("foreach statements not implemented", node);
}

void StatementSema::statement(AstBreakStatement* node)
{
	if (loop_level == 0)
		throw SemaException("break not allowed outside loops", node);
}

void StatementSema::statement(AstContinueStatement* node)
{
	if (loop_level == 0)
		throw SemaException("continue not allowed outside loops", node);
}

void StatementSema::statement(AstGotoStatement* node)
{
	throw SemaException("goto statements not supported", node);
}

void StatementSema::statement(AstReturnStatement* node)
{
	if (node->expression)
	{
		ExpressionSema expression(sema, name_scope, type_scope);
		expression.analyze(node->expression);

		ResolveExpressionConstants resolve_constants(sema);
		resolve_constants.analyze(node->expression);
	}
}

void StatementSema::local_variable_declaration(AstLocalVariableDeclaration* variable_declaration)
{
	Type* type = type_scope.lookup_type(variable_declaration->type);

	for (size_t i = 0; i < variable_declaration->variables.size(); i++)
	{
		name_scope.variables[variable_declaration->variables[i]->identifier] = ExpressionResult(type, ExpressionClass::variable);

		auto initial_value = variable_declaration->variables[i]->expression;
		if (initial_value)
		{
			ExpressionSema expression(sema, name_scope, type_scope);
			expression.analyze(initial_value);

			ResolveExpressionConstants resolve_constants(sema);
			resolve_constants.analyze(initial_value);

			auto& ts = sema.type_system();
			if (!ts.implicit_convert_allowed(initial_value->result.type, type, false))
				throw SemaException("type conversion not allowed", initial_value);
		}
	}
}
