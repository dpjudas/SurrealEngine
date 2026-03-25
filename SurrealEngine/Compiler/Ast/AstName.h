
#pragma once

#include "AstNode.h"
#include <string>
#include <vector>

class AstTypeParameterList;

class AstName : public AstNode
{
public:
};

class AstIdentifierName : public AstName // N.I<A,B>
{
public:
	AstIdentifierName *prev_name = nullptr;

	std::string name;
	AstTypeParameterList *parameter_list = nullptr;
};

class AstClassName : public AstName // class<I>
{
public:
	std::string name;
};

class AstObjectName : public AstName // type'name'
{
public:
	std::string class_name;
	std::string object_name;
};

class AstKeywordType : public AstName // bool, byte, int, float
{
public:
	std::string type;
};
