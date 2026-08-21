
#include "Precomp.h"
#include "SemanticAnalysis.h"
#include "CreateTypeNames.h"
#include "CreateTypeMembers.h"
#include "CreateTypeExpressions.h"
#include "MethodSema.h"

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
