#pragma once

#include "ExpressionValue.h"

class Bytecode;
class UObject;
class UFunction;
struct ExpressionEvalResult;

class Frame
{
public:
	static ExpressionValue Call(UFunction* func, UObject* instance, std::vector<ExpressionValue> args);
	static std::string GetCallstack();

private:
	ExpressionEvalResult Run();
	void ProcessSwitch(const ExpressionValue& condition);

	std::unique_ptr<uint64_t[]> Variables;
	UObject* Object = nullptr;
	Bytecode* Code = nullptr;
	size_t StatementIndex = 0;
};
