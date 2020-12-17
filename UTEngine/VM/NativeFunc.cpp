
#include "Precomp.h"
#include "NativeFunc.h"

std::vector<NativeFuncHandler> NativeFunctions::NativeByIndex;
std::map<std::pair<std::string, std::string>, NativeFuncHandler> NativeFunctions::NativeByName;

void NativeFunctions::RegisterHandler(const std::string& className, const std::string& funcName, int nativeIndex, NativeFuncHandler handler)
{
	if (nativeIndex != 0)
	{
		if (NativeByIndex.size() < (size_t)nativeIndex) NativeByIndex.resize((size_t)nativeIndex + 1);
		NativeByIndex[nativeIndex] = handler;
	}
	else
	{
		NativeByName[{ funcName, className }] = handler;
	}
}
