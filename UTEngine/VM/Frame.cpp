
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"

void Frame::Run()
{
	if (Code->Statements.size() != StatementIndex)
	{
		ExpressionEvaluator evaluator;
		ExpressionEvalResult result = evaluator.Eval(Code->Statements[StatementIndex], Object);
		switch (result.Result)
		{
		case StatementResult::Next:
			StatementIndex++;
			break;
		case StatementResult::Jump:
//			StatementIndex = Code->FindStatementIndex(result.JumpAddress);
//			break;
		case StatementResult::LatentWait:
		case StatementResult::Return:
		case StatementResult::Stop:
			return;
		}
	}
}
