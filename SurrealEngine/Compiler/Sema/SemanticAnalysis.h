
#pragma once

#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Frontend/CompilerException.h"
#include <vector>
#include <memory>

class AstNode;
class AstCompilationUnit;

class SemaException : public CompilerException
{
public:
	SemaException(const std::string& message, AstNode* location);
	int sourceIndex;
	int line;
	int column;
};

class SemanticAnalysis
{
public:
	SemanticAnalysis(TypeSystem& type_system);

	void analyze(std::vector<std::shared_ptr<AstCompilationUnit>> asts);

	TypeSystem& type_system() { return _type_system; }

private:
	TypeSystem& _type_system;
};

class TypeScope
{
public:
	TypeScope(TypeSystem& ts);

	void push_scope(AstNameDeclaration* ast_name_declaration);
	void pop_scope();

	Type* lookup_type(AstName* name);

	std::vector<TypeName*> scopes;

private:
	TypeSystem& ts;
};

class MemberLookup
{
public:
	MemberLookup(TypeSystem& type_system);

	void lookup(TypeName* type, const std::string& name);

	std::set<TypeName*> members;

private:
	TypeSystem& type_system;
};
