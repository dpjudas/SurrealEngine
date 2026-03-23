
#pragma once

#include "AstNode.h"
#include <string>
#include <vector>

class AstTypeParameterList;

class AstName : public AstNode
{
public:
	std::vector<int> array_rank_specifiers;
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

class AstKeywordType : public AstName // bool, decimal, sbyte, byte, short, ushort, int, uint, long, ulong, char, float, double
{
public:
	std::string type;
};
