
#pragma once

#include "AstNameVisitor.h"

class AstNode
{
public:
	virtual ~AstNode() = default;
	virtual void visit(AstNameVisitor *visitor) { }

	int line = -1;
	int column = -1;
};
