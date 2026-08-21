
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

	ClassType* type = type_system.newType<ClassType>(parent, node->identifier, node->is_abstract);

	node->type = type;

	if (current_class)
		current_class->subtypes.push_back(type);
	else if (current_struct)
		current_struct->subtypes.push_back(type);

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
}

void CreateTypeNames::name(AstMethodDeclaration* node)
{
}

void CreateTypeNames::name(AstOperatorDeclaration* node)
{
}

void CreateTypeNames::name(AstStateDeclaration* node)
{
}
