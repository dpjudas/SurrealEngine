
#include "Precomp.h"
#include "MethodSema.h"
#include "SemanticAnalysis.h"
#include "StatementSema.h"
#include "Compiler/Typesystem/ReferenceType.h"
#include "Compiler/Typesystem/ValueType.h"
#include "Compiler/Typesystem/TypeSystem.h"

MethodSema::MethodSema(SemanticAnalysis& sema) : sema(sema), type_scope(sema.type_system())
{
}

void MethodSema::analyze(AstCompilationUnit* ast)
{
	ast->class_decl->visit(this);
}

void MethodSema::name(AstClassDeclaration* node)
{
	type_scope.push_scope(node);

	Type* current_this_type = this_type;
	this_type = node->type;
	node->visit_children(this);
	this_type = current_this_type;

	type_scope.pop_scope();
}

void MethodSema::name(AstStructDeclaration* node)
{
	type_scope.push_scope(node);

	Type* current_this_type = this_type;
	this_type = node->type;
	node->visit_children(this);
	this_type = current_this_type;

	type_scope.pop_scope();
}

void MethodSema::name(AstEnumDeclaration* node)
{
}

void MethodSema::name(AstEnumValueDeclaration* node)
{
}

void MethodSema::name(AstConstantDeclaration* node)
{
}

void MethodSema::name(AstFieldDeclaration* node)
{
}

void MethodSema::name(AstMethodDeclaration* node)
{
	if (node->block)
	{
		add_parameters(node->name_scope, node->parameters, node->is_static);

		StatementSema statements(sema, node->name_scope, type_scope);
		statements.analyze(node->block);
	}
}

void MethodSema::name(AstOperatorDeclaration* node)
{
	throw SemaException("Operator overloading not supported");
}

void MethodSema::name(AstStateDeclaration* node)
{
}

void MethodSema::add_parameters(NameScope& name_scope, const std::vector<AstMethodParameter*>& parameters, bool is_static, Type* value_type)
{
	if (!is_static)
		name_scope.variables["this"] = ExpressionResult(this_type, dynamic_cast<StructType*>(this_type) ? ExpressionClass::variable : ExpressionClass::value);

	if (value_type)
		name_scope.variables["value"] = ExpressionResult(value_type, ExpressionClass::variable);

	for (size_t i = 0; i < parameters.size(); i++)
	{
		if (AstMethodFixedParameter* fixed_param = dynamic_cast<AstMethodFixedParameter*>(parameters[i]))
		{
			Type* type = type_scope.lookup_type(fixed_param->type);
			name_scope.variables[fixed_param->identifier] = ExpressionResult(type, ExpressionClass::variable);
		}
	}
}
