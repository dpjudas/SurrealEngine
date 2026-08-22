
#include "Precomp.h"
#include "SemanticAnalysis.h"
#include "CreateTypeNames.h"
#include "CreateTypeMembers.h"
#include "CreateTypeExpressions.h"
#include "MethodSema.h"
#include "Compiler/Typesystem/TypeSystem.h"

SemanticAnalysis::SemanticAnalysis(TypeSystem& type_system) : _type_system(type_system)
{
}

void SemanticAnalysis::analyze(std::vector<std::shared_ptr<AstCompilationUnit>> asts)
{
#if 0
	CreateTypeNames create_types(type_system());
	for (size_t i = 0; i < asts.size(); i++)
	{
		create_types.exec(asts[i].get());
	}

	_type_system.setupVectorType();

	ResolveTypeBases resolve_type_bases(type_system());
	for (size_t i = 0; i < asts.size(); i++)
	{
		resolve_type_bases.exec(asts[i].get());
	}

	CreateTypeMembers create_members(type_system());
	for (size_t i = 0; i < asts.size(); i++)
	{
		create_members.exec(asts[i].get());
	}

	CreateTypeExpressions create_expressions(*this);
	for (size_t i = 0; i < asts.size(); i++)
	{
		create_expressions.exec(asts[i].get());
	}

	MethodSema method_sema(*this);
	for (size_t i = 0; i < asts.size(); i++)
	{
		method_sema.analyze(asts[i].get());
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////

SemaException::SemaException(const std::string& message, AstNode* location) : CompilerException(message), sourceIndex(location ? location->sourceIndex : -1), line(location ? location->line : -1), column(location ? location->column : -1)
{
}

/////////////////////////////////////////////////////////////////////////////

TypeScope::TypeScope(TypeSystem& ts) : ts(ts)
{
}

void TypeScope::push_scope(AstNameDeclaration* ast_name_declaration)
{
	scopes.push_back(ast_name_declaration->type);
}

void TypeScope::pop_scope()
{
	scopes.pop_back();
}

Type* TypeScope::lookup_type(AstName* name)
{
	if (auto keyword = dynamic_cast<AstKeywordType*>(name))
	{
		NameString typeName = keyword->type;
		auto it = ts.nameToType.find(typeName);
		if (it != ts.nameToType.end())
			return it->second;
	}
	else if (auto identifier = dynamic_cast<AstIdentifierName*>(name))
	{
		NameString typeName = identifier->name;
		for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
		{
			TypeName* scope = *it;
			if (auto cls = dynamic_cast<ClassType*>(scope))
			{
				while (cls)
				{
					for (Type* subtype : cls->subtypes)
					{
						if (typeName == subtype->name)
						{
							return subtype;
						}
					}
					cls = cls->base;
				}
			}
		}
		auto it = ts.nameToType.find(typeName);
		if (it != ts.nameToType.end())
			return it->second;

		throw SemaException("Unknown type '" + identifier->name + "'", name);
	}
	else if (auto clsName = dynamic_cast<AstClassName*>(name))
	{
		return ts.classObject;
	}
	else if (auto objName = dynamic_cast<AstObjectName*>(name))
	{
		// What should we do here?
	}
	throw SemaException("Unknown type", name);
}

/////////////////////////////////////////////////////////////////////////////

MemberLookup::MemberLookup(TypeSystem& type_system) : type_system(type_system)
{
}

void MemberLookup::lookup(TypeName* type, const std::string& name)
{
	NameString typeName = name;
	if (auto cls = dynamic_cast<ClassType*>(type))
	{
		while (cls)
		{
			for (ConstantTypeMember* constant : cls->constants)
			{
				if (typeName == constant->name)
					members.insert(constant);
			}

			for (FieldTypeMember* field : cls->fields)
			{
				if (typeName == field->name)
					members.insert(field);
			}

			for (MethodTypeMember* method : cls->methods)
			{
				if (typeName == method->name)
					members.insert(method);
			}

			for (OperatorTypeMember* operator_ : cls->operators)
			{
				if (typeName == operator_->name)
					members.insert(operator_);
			}

			for (Type* subtype : cls->subtypes)
			{
				if (typeName == subtype->name)
					members.insert(subtype);
			}

			cls = cls->base;
		}
	}
}
