
#pragma once

#include "Compiler/Typesystem/TypeSystem.h"
#include "Compiler/Frontend/CompilerException.h"
#include <vector>
#include <memory>

class AstCompilationUnit;

class SemaException : public CompilerException
{
public:
	SemaException(const std::string& message) : CompilerException(message) {}
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
