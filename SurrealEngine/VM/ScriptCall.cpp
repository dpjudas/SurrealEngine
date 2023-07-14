
#include "Precomp.h"
#include "ScriptCall.h"
#include "Frame.h"

ExpressionValue CallEvent(UObject* Context, const NameString& name, std::vector<ExpressionValue> args)
{
	if (!Context->IsEventEnabled(name))
		return ExpressionValue::NothingValue();

	UFunction* func = FindEventFunction(Context, name);
	if (func)
		return Frame::Call(func, Context, std::move(args));
	else
		return ExpressionValue::NothingValue(); // throw std::runtime_error("Event " + name + " not found on object");
}

UFunction* FindEventFunction(UObject* Context, const NameString& name)
{
	// Search states first

	NameString stateName = Context->GetStateName();
	if (!stateName.IsNone())
	{
		for (UClass* cls = Context->Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
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

	for (UClass* cls = Context->Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		UFunction* func = cls->GetFunction(name);
		if (func)
		{
			return func;
		}
	}

	return nullptr;
}
