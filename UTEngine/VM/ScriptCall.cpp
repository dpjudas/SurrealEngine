
#include "Precomp.h"
#include "ScriptCall.h"
#include "Frame.h"

ExpressionValue CallEvent(UObject* Context, const std::string& name, std::vector<ExpressionValue> args)
{
	UFunction* func = FindEventFunction(Context, name);
	if (func)
		return Frame::Call(func, Context, std::move(args));
	else
		throw std::runtime_error("Event " + name + " not found on object");
}

UFunction* FindEventFunction(UObject* Context, const std::string& name)
{
	// Search states first

	for (UClass* cls = Context->Base; cls != nullptr; cls = cls->Base)
	{
		for (UField* field = cls->Children; field != nullptr; field = field->Next)
		{
			UState* state = UObject::TryCast<UState>(field);
			if (state && state->Name == Context->StateName)
			{
				for (UField* field2 = state->Children; field2 != nullptr; field2 = field2->Next)
				{
					UFunction* func = UObject::TryCast<UFunction>(field2);
					if (func && func->Name == name)
					{
						return func;
					}
				}
			}
		}
	}

	// Search normal member functions next

	for (UClass* cls = Context->Base; cls != nullptr; cls = cls->Base)
	{
		for (UField* field = cls->Children; field != nullptr; field = field->Next)
		{
			UFunction* func = UObject::TryCast<UFunction>(field);
			if (func && func->Name == name)
			{
				return func;
			}
		}
	}

	return nullptr;
}
