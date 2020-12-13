
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"

void Frame::Step()
{
	if (Code->Statements.size() != StatementIndex)
	{
		ExpressionEvaluator evaluator;
		evaluator.Eval(Code->Statements[StatementIndex++]);
	}
}
