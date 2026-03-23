
#pragma once

#include "AstNode.h"
#include <string>
#include <vector>

class AstExpression;

class AstArrayVariableInitializer : public AstNode
{
public:
	AstExpression *expression = nullptr;
	AstArrayInitializer *array_initializer = nullptr;
};

class AstArrayInitializer : public AstNode
{
public:
	std::vector<AstArrayVariableInitializer *> variable_initializer_list;
};
