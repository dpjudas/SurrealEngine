
#include "Precomp.h"
#include "CreateTypeMembers.h"

CreateTypeMembers::CreateTypeMembers(TypeSystem& type_system) : type_system(type_system), type_scope(type_system)
{
}

void CreateTypeMembers::exec(AstCompilationUnit* ast)
{
	ast->class_decl->visit(this);
}

void CreateTypeMembers::name(AstClassDeclaration* node)
{
	type_scope.push_scope(node);

	ClassType* last_class = current_class;
	StructType* last_struct = current_struct;

	current_class = static_cast<ClassType*>(node->type);
	current_struct = nullptr;

	node->visit_children(this);

	current_class = last_class;
	current_struct = last_struct;

	type_scope.pop_scope();
}

void CreateTypeMembers::name(AstStructDeclaration* node)
{
	type_scope.push_scope(node);

	ClassType* last_class = current_class;
	StructType* last_struct = current_struct;

	current_class = nullptr;
	current_struct = static_cast<StructType*>(node->type);

	node->visit_children(this);

	current_class = last_class;
	current_struct = last_struct;

	type_scope.pop_scope();
}

void CreateTypeMembers::name(AstEnumDeclaration* node)
{
}

void CreateTypeMembers::name(AstEnumValueDeclaration* node)
{
}

void CreateTypeMembers::name(AstConstantDeclaration* node)
{
	for (const auto& var_decl : node->declarators)
	{
		ConstantTypeMember* member = type_system.newType<ConstantTypeMember>(get_parent(), var_decl->identifier, nullptr);
		var_decl->sema_type = member;

		if (current_class)
			current_class->constants.push_back(member);
		else if (current_struct)
			current_struct->constants.push_back(member);
	}
}

void CreateTypeMembers::name(AstFieldDeclaration* node)
{
	Type* type;
	if (node->struct_decl)
	{
		node->struct_decl->visit(this);
		type = node->struct_decl->type;
	}
	else if (node->enum_decl)
	{
		node->enum_decl->visit(this);
		type = node->enum_decl->type;
	}
	else
	{
		type = type_scope.lookup_type(node->type);
	}

	for (const auto& var_decl : node->declarators)
	{
		FieldTypeMember* member = type_system.newType<FieldTypeMember>(get_parent(), var_decl->identifier, type);
		node->sema_type = member;

		if (current_class)
			current_class->fields.push_back(member);
		else if (current_struct)
			current_struct->fields.push_back(member);
	}
}

void CreateTypeMembers::name(AstMethodDeclaration* node)
{
	Type* return_type = type_scope.lookup_type(node->return_type);

	MethodTypeMember* member = type_system.newType<MethodTypeMember>(
		get_parent(),
		node->identifier,
		node->is_static, 
		node->is_event,
		node->is_native,
		node->is_exec,
		node->is_singular,
		node->is_simulated,
		node->is_final,
		node->is_latent,
		node->is_iterator,
		node->is_operator,
		node->is_preoperator,
		node->is_postoperator,
		return_type);

	node->sema_type = member;

	add_parameters(node->parameters, member->parameters);

	if (current_class)
		current_class->methods.push_back(member);
	else if (current_struct)
		current_struct->methods.push_back(member);
}

void CreateTypeMembers::name(AstStateDeclaration* node)
{
}

void CreateTypeMembers::add_parameters(const std::vector<AstMethodParameter*>& ast_parameters, std::vector<MethodFixedParameter*>& parameters)
{
	for (size_t i = 0; i < ast_parameters.size(); i++)
	{
		if (AstMethodFixedParameter* param = dynamic_cast<AstMethodFixedParameter*>(ast_parameters[i]))
		{
			MethodFixedParameter* type_param = type_system.newFixedParameter<MethodFixedParameter>();
			type_param->is_out = param->is_out;
			type_param->is_ref = param->is_ref;
			type_param->is_optional = param->is_optional;
			type_param->coerce = param->coerce;
			type_param->skip = param->skip;
			type_param->type = type_scope.lookup_type(param->type);
			type_param->name = param->identifier;

			parameters.push_back(type_param);
		}
	}
}

TypeName* CreateTypeMembers::get_parent()
{
	if (current_class)
		return current_class;
	else if (current_struct)
		return current_struct;
	else
		return nullptr;
}
