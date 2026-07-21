
#include "Precomp.h"
#include "NativeFunc.h"

Array<UFunction*> NativeFunctions::FuncByIndex;
Array<NativeFuncHandler> NativeFunctions::NativeByIndex;
std::map<std::pair<NameString, NameString>, NativeFuncHandler> NativeFunctions::NativeByName;
Array<LatentRunState> NativeFunctions::LatentActionByIndex;
std::map<LatentRunState, int> NativeFunctions::IndexForLatentAction;

void NativeFunctions::RegisterHandler(const NameString& className, const NameString& funcName, int nativeIndex, NativeFuncHandler handler)
{
	if (nativeIndex != 0)
	{
		if (nativeIndex == 2054 && funcName == "GetSoundLength") // DeusEx got two native functions with same intrinsic value!
			return;

		if (NativeByIndex.size() <= (size_t)nativeIndex) NativeByIndex.resize((size_t)nativeIndex + 1);

		if (NativeByIndex[nativeIndex] != nullptr)
			Exception::Throw("Attempted to assign native index "
				+ std::to_string(nativeIndex) + " to " + className.ToString() + "." + funcName.ToString()
				+ ", which was already assigned to a different native function.");

		NativeByIndex[nativeIndex] = handler;
	}
	else
	{
		NativeByName[{ funcName, className }] = handler;
	}
}

void NativeFunctions::RegisterNativeFunc(UFunction* func)
{
	int nativeIndex = func->NativeFuncIndex;
	
	if (nativeIndex != 0)
	{
		if (nativeIndex == 2054 && func->Name == "GetSoundLength") // DeusEx got two native functions with same intrinsic value!
			return;

		if (FuncByIndex.size() <= (size_t)nativeIndex) FuncByIndex.resize((size_t)nativeIndex + 1);

		if (FuncByIndex[nativeIndex] != nullptr)
			Exception::Throw("Attempted to assign native index "
				+ std::to_string(nativeIndex) + " to " + UObject::GetUClassFullName(func).ToString()
				+ ", which was already assigned to a different native function.");

		FuncByIndex[nativeIndex] = func;
	}
}

void RegisterLatentAction(int nativeIndex, LatentRunState latentAction)
{
	if (NativeFunctions::LatentActionByIndex.size() <= (size_t)nativeIndex) NativeFunctions::LatentActionByIndex.resize((size_t)nativeIndex + 1);
	NativeFunctions::LatentActionByIndex[nativeIndex] = latentAction;
	NativeFunctions::IndexForLatentAction[latentAction] = nativeIndex;
}
