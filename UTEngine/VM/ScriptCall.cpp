
#include "Precomp.h"
#include "ScriptCall.h"
#include "Frame.h"

ExpressionValue CallEvent(UObject* Context, const std::string& name, std::vector<ExpressionValue> args)
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
						return Frame::Call(func, Context, std::move(args));
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
				return Frame::Call(func, Context, std::move(args));
			}
		}
	}

	throw std::runtime_error("Event " + name + " not found on object");
}
