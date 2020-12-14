#pragma once

class Bytecode;
class Expression;
class UObject;

class Frame
{
public:
	void Run();

	UObject* Object = nullptr;
	Bytecode* Code = nullptr;
	size_t StatementIndex = 0;
};
