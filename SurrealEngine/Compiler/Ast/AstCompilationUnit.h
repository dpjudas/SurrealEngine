
#pragma once

#include <vector>
#include <memory>
#include "AstNode.h"

class AstClassDeclaration;

class AstCompilationUnit
{
public:
	AstCompilationUnit() = default;

	AstClassDeclaration* class_decl = nullptr;

	std::vector<std::unique_ptr<AstNode>> allocatedNodes;

private:
	AstCompilationUnit(const AstCompilationUnit &other) = delete;
	AstCompilationUnit &operator=(const AstCompilationUnit &other) = delete;
};
