
#include "Precomp.h"
#include "NLevelInfo.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"

void NLevelInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("LevelInfo", "GetAddressURL", &NLevelInfo::GetAddressURL, 0);
	RegisterVMNativeFunc_1("LevelInfo", "GetLocalURL", &NLevelInfo::GetLocalURL, 0);
	RegisterVMNativeFunc_0("LevelInfo", "InitEventManager", &NLevelInfo::InitEventManager, 650);
}

void NLevelInfo::GetAddressURL(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->URL.GetAddressURL();
}

void NLevelInfo::GetLocalURL(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->URL.ToString();
}

void NLevelInfo::InitEventManager(UObject* Self)
{
	// Deus Ex
}
