
#include "Precomp.h"
#include "NLevelInfo.h"
#include "VM/NativeFunc.h"

void NLevelInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("LevelInfo", "GetAddressURL", &NLevelInfo::GetAddressURL, 0);
	RegisterVMNativeFunc_1("LevelInfo", "GetLocalURL", &NLevelInfo::GetLocalURL, 0);
}

void NLevelInfo::GetAddressURL(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("LevelInfo.GetAddressURL not implemented");
}

void NLevelInfo::GetLocalURL(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("LevelInfo.GetLocalURL not implemented");
}
