#pragma once

#include "ExpressionEvaluator.h"

class Bytecode;
class UObject;

class Frame
{
public:
	ExpressionEvalResult Run();

	UObject* Object = nullptr;
	Bytecode* Code = nullptr;
	size_t StatementIndex = 0;
};
