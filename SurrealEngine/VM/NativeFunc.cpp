
#include "Precomp.h"
#include "NativeFunc.h"

Array<UFunction*> NativeFunctions::FuncByIndex;
Array<NativeFuncHandler> NativeFunctions::NativeByIndex;
std::map<std::pair<NameString, NameString>, NativeFuncHandler> NativeFunctions::NativeByName;

void NativeFunctions::RegisterHandler(const NameString& className, const NameString& funcName, int nativeIndex, NativeFuncHandler handler)
{
	if (nativeIndex != 0)
	{
		if (NativeByIndex.size() <= (size_t)nativeIndex) NativeByIndex.resize((size_t)nativeIndex + 1);
		NativeByIndex[nativeIndex] = handler;
	}
	else
	{
		NativeByName[{ funcName, className }] = handler;
	}
}

void NativeFunctions::RegisterNativeFunc(UFunction* func)
{
	int nativeIndex = func->NativeFuncIndex;;
	if (nativeIndex != 0)
	{
		if (FuncByIndex.size() <= (size_t)nativeIndex) FuncByIndex.resize((size_t)nativeIndex + 1);
		FuncByIndex[nativeIndex] = func;
	}
}
