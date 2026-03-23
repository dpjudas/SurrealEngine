
#pragma once

#include "AstNode.h"
#include "AstName.h"
#include <vector>

class AstTypeParameter : public AstNode
{
public:
	AstName *name = nullptr;
};

class AstTypeParameterList : public AstNode
{
public:
	std::vector<AstTypeParameter *> parameters;
};
