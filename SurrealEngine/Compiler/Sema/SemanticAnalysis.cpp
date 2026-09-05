
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

int SemanticAnalysis::pass_count() const
{
	return 5;
}

void SemanticAnalysis::analyze_pass(std::shared_ptr<AstCompilationUnit> ast, int pass)
{
	switch (pass)
	{
	case 0:
	{
		CreateTypeNames create_types(type_system());
		create_types.exec(ast.get());
		break;
	}
	case 1:
	{
		ResolveTypeBases resolve_type_bases(type_system());
		resolve_type_bases.exec(ast.get());
		break;
	}
	case 2:
	{
		CreateTypeMembers create_members(type_system());
		create_members.exec(ast.get());
		break;
	}
	case 3:
	{
		CreateTypeExpressions create_expressions(*this);
		create_expressions.exec(ast.get());
		break;
	}
	case 4:
	{
		MethodSema method_sema(*this);
		method_sema.analyze(ast.get());
		break;
	}
	}
}

void SemanticAnalysis::end_pass(int pass)
{
	switch (pass)
	{
	case 0:
		_type_system.setupVectorType();
		break;
	case 2:
		_type_system.setupOperators();
		break;
	}
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

		throw SemaException("Unknown identifier '" + identifier->name + "'", name);
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
				{
					members.insert(constant);
					break;
				}
			}

			for (FieldTypeMember* field : cls->fields)
			{
				if (typeName == field->name)
				{
					members.insert(field);
					break;
				}
			}

			for (MethodTypeMember* method : cls->methods)
			{
				if (typeName == method->name)
				{
					members.insert(method);
					break;
				}
			}

			for (Type* subtype : cls->subtypes)
			{
				if (typeName == subtype->name)
					members.insert(subtype);
			}

			cls = cls->base;
		}
	}
	else if (auto struct_ = dynamic_cast<StructType*>(type))
	{
		while (struct_)
		{
			for (ConstantTypeMember* constant : struct_->constants)
			{
				if (typeName == constant->name)
				{
					members.insert(constant);
					break;
				}
			}

			for (FieldTypeMember* field : struct_->fields)
			{
				if (typeName == field->name)
				{
					members.insert(field);
					break;
				}
			}

			for (MethodTypeMember* method : struct_->methods)
			{
				if (typeName == method->name)
				{
					members.insert(method);
					break;
				}
			}

			for (Type* subtype : struct_->subtypes)
			{
				if (typeName == subtype->name)
				{
					members.insert(subtype);
					break;
				}
			}

			struct_ = struct_->base;
		}
	}
}
