#pragma once

class Bytecode;
class Expression;

class Frame
{
public:
	void Step();

	Bytecode* Code = nullptr;
	size_t StatementIndex = 0;
};
