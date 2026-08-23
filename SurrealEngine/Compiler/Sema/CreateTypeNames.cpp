
#include "Precomp.h"
#include "CreateTypeNames.h"

CreateTypeNames::CreateTypeNames(TypeSystem& type_system) : type_system(type_system)
{
}

void CreateTypeNames::exec(AstCompilationUnit* ast)
{
	ast->class_decl->visit(this);
}

void CreateTypeNames::name(AstClassDeclaration* node)
{
	TypeName* parent = nullptr;
	if (current_class)
		parent = current_class;
	else if (current_struct)
		parent = current_struct;

	ClassType* type;
	if (node->identifier == "Object" && !current_class && !current_struct)
	{
		type = type_system.object;
	}
	else
	{
		type = type_system.newType<ClassType>(parent, node->identifier, node->is_abstract);
		if (current_class)
			current_class->subtypes.push_back(type);
		else if (current_struct)
			current_struct->subtypes.push_back(type);
		else
			type_system.addType(type);
	}

	node->type = type;

	ClassType* last_class = current_class;
	StructType* last_struct = current_struct;

	current_class = type;
	current_struct = nullptr;

	node->visit_children(this);

	current_class = last_class;
	current_struct = last_struct;
}

void CreateTypeNames::name(AstStructDeclaration* node)
{
	TypeName* parent = nullptr;
	if (current_class)
		parent = current_class;
	else if (current_struct)
		parent = current_struct;

	StructType* type = type_system.newType<StructType>(parent, node->identifier);

	node->type = type;

	if (current_class)
		current_class->subtypes.push_back(type);
	else if (current_struct)
		current_struct->subtypes.push_back(type);
	else
		type_system.addType(type);

	ClassType* last_class = current_class;
	StructType* last_struct = current_struct;

	current_class = nullptr;
	current_struct = type;

	node->visit_children(this);

	current_class = last_class;
	current_struct = last_struct;
}

void CreateTypeNames::name(AstEnumDeclaration* node)
{
	TypeName* parent = nullptr;
	if (current_class)
		parent = current_class;
	else if (current_struct)
		parent = current_struct;

	EnumType* type = type_system.newType<EnumType>(parent, node->identifier);

	if (current_class)
		current_class->subtypes.push_back(type);
	else if (current_struct)
		current_struct->subtypes.push_back(type);
	else
		type_system.addType(type);

	current_enum = type;
	node->visit_children(this);
	current_enum = nullptr;
}

void CreateTypeNames::name(AstEnumValueDeclaration* node)
{
	EnumValueTypeMember* value = type_system.newType<EnumValueTypeMember>(current_enum, node->identifier);
	node->sema_type = value;
	current_enum->values.push_back(value);
}

void CreateTypeNames::name(AstConstantDeclaration* node)
{
}

void CreateTypeNames::name(AstFieldDeclaration* node)
{
	if (node->struct_decl)
		node->struct_decl->visit(this);
	else if (node->enum_decl)
		node->enum_decl->visit(this);
}

void CreateTypeNames::name(AstMethodDeclaration* node)
{
}

void CreateTypeNames::name(AstStateDeclaration* node)
{
}

/////////////////////////////////////////////////////////////////////////////

ResolveTypeBases::ResolveTypeBases(TypeSystem& type_system) : type_system(type_system), type_scope(type_system)
{
}

void ResolveTypeBases::exec(AstCompilationUnit* ast)
{
	ast->class_decl->visit(this);
}

void ResolveTypeBases::name(AstClassDeclaration* node)
{
	type_scope.push_scope(node);

	ClassType* current_class = static_cast<ClassType*>(node->type);
	if (node->base_type)
	{
		Type* base_type = type_scope.lookup_type(node->base_type);
		if (dynamic_cast<ClassType*>(base_type))
		{
			current_class->base = static_cast<ClassType*>(base_type);
		}
	}
	else if (current_class != type_system.object)
	{
		current_class->base = type_system.object;
	}

	node->visit_children(this);
	type_scope.pop_scope();
}

void ResolveTypeBases::name(AstStructDeclaration* node)
{
	type_scope.push_scope(node);

	StructType* current_struct = static_cast<StructType*>(node->type);
	if (node->base)
	{
		Type* base_type = type_scope.lookup_type(node->base);
		if (dynamic_cast<StructType*>(base_type))
		{
			current_struct->base = static_cast<StructType*>(base_type);
		}
	}

	node->visit_children(this);
	type_scope.pop_scope();
}

void ResolveTypeBases::name(AstEnumDeclaration* node)
{
}

void ResolveTypeBases::name(AstEnumValueDeclaration* node)
{
}

void ResolveTypeBases::name(AstConstantDeclaration* node)
{
}

void ResolveTypeBases::name(AstFieldDeclaration* node)
{
}

void ResolveTypeBases::name(AstMethodDeclaration* node)
{
}

void ResolveTypeBases::name(AstStateDeclaration* node)
{
}
