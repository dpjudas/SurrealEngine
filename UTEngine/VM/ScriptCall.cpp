
#include "Precomp.h"
#include "ScriptCall.h"
#include "Frame.h"

ExpressionValue CallEvent(UObject* Context, const std::string& name, std::vector<ExpressionValue> args)
{
	if (!Context->IsEventEnabled(name))
		return ExpressionValue::NothingValue();

	UFunction* func = FindEventFunction(Context, name);
	if (func)
		return Frame::Call(func, Context, std::move(args));
	else
		throw std::runtime_error("Event " + name + " not found on object");
}

UFunction* FindEventFunction(UObject* Context, const std::string& name)
{
	// Search states first

	std::string stateName = Context->GetStateName();
	if (!stateName.empty())
	{
		for (UClass* cls = Context->Base; cls != nullptr; cls = cls->Base)
		{
			UState* state = cls->GetState(stateName);
			if (state)
			{
				UFunction* func = state->GetFunction(name);
				if (func)
				{
					return func;
				}
			}
		}
	}

	// Search normal member functions next

	for (UClass* cls = Context->Base; cls != nullptr; cls = cls->Base)
	{
		UFunction* func = cls->GetFunction(name);
		if (func)
		{
			return func;
		}
	}

	return nullptr;
}
