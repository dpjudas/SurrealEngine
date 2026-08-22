
#pragma once

#include "AstNameVisitor.h"

class AstNode
{
public:
	virtual ~AstNode() = default;
	virtual void visit(AstNameVisitor *visitor) { }

	int sourceIndex = -1;
	int line = -1;
	int column = -1;
};
