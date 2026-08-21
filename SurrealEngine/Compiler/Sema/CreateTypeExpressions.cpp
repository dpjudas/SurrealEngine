
#include "Precomp.h"
#include "CreateTypeExpressions.h"
#include "SemanticAnalysis.h"
#include "ExpressionSema.h"
#include "CreateTypeConstExpression.h"
#include "Compiler/Typesystem/ReferenceType.h"
#include "Compiler/Typesystem/ValueType.h"
#include "Compiler/Typesystem/TypeSystem.h"

CreateTypeExpressions::CreateTypeExpressions(SemanticAnalysis &sema) : sema(sema), type_scope(sema.type_system())
{
}

void CreateTypeExpressions::exec(AstCompilationUnit *ast)
{
	ast->class_decl->visit(this);
}

void CreateTypeExpressions::name(AstClassDeclaration *node)
{
	type_scope.push_scope(node);

	Type *current_this_type = this_type;
	this_type = node->type;
	node->visit_children(this);
	this_type = current_this_type;

	type_scope.pop_scope();
}

void CreateTypeExpressions::name(AstStructDeclaration *node)
{
	type_scope.push_scope(node);

	Type *current_this_type = this_type;
	this_type = node->type;
	node->visit_children(this);
	this_type = current_this_type;

	type_scope.pop_scope();
}

void CreateTypeExpressions::name(AstEnumDeclaration *node)
{
}

void CreateTypeExpressions::name(AstEnumValueDeclaration *node)
{
	node->sema_type->expression = create_expression(node->expression);
}

void CreateTypeExpressions::name(AstConstantDeclaration *node)
{
	for (const auto &decl : node->declarators)
	{
		ConstantTypeMember *type = decl->sema_type;
		type->expression = create_expression(decl->expression);
	}
}

void CreateTypeExpressions::name(AstFieldDeclaration *node)
{
}

void CreateTypeExpressions::name(AstMethodDeclaration *node)
{
}

void CreateTypeExpressions::name(AstOperatorDeclaration *node)
{
}

void CreateTypeExpressions::name(AstStateDeclaration* node)
{
}

TypeConstantExpression *CreateTypeExpressions::create_expression(AstExpression *expression)
{
	NameScope name_scope;
	ExpressionSema resolve_expression(sema, name_scope, type_scope);
	resolve_expression.analyze(expression);

	CreateTypeConstExpression create_expression(sema);
	return create_expression.exec(expression);
}
