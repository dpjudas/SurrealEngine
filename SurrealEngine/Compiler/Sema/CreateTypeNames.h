
#pragma once

#include "Compiler/Ast/Ast.h"
#include "Compiler/Typesystem/TypeSystem.h"
#include "SemanticAnalysis.h"

class CreateTypeNames : public AstNameVisitor
{
public:
	CreateTypeNames(TypeSystem& type_system);

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

	TypeSystem& type_system;

	ClassType* current_class = nullptr;
	StructType* current_struct = nullptr;
	EnumType* current_enum = nullptr;
};

class ResolveTypeBases : public AstNameVisitor
{
public:
	ResolveTypeBases(TypeSystem& type_system);

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

	TypeSystem& type_system;
	TypeScope type_scope;
};
