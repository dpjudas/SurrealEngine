
#include "Precomp.h"
#include "Frame.h"
#include "Bytecode.h"
#include "ExpressionEvaluator.h"

ExpressionEvalResult Frame::Run()
{
	while (true)
	{
		if (StatementIndex >= Code->Statements.size())
			throw std::runtime_error("Unexpected end of code statements");

		ExpressionEvaluator evaluator;
		ExpressionEvalResult result = evaluator.Eval(Code->Statements[StatementIndex], Object, nullptr);
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
			return result;
		}
	}
}
