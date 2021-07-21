
#include "Precomp.h"
#include "NLevelInfo.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"

void NLevelInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("LevelInfo", "GetAddressURL", &NLevelInfo::GetAddressURL, 0);
	RegisterVMNativeFunc_1("LevelInfo", "GetLocalURL", &NLevelInfo::GetLocalURL, 0);
}

void NLevelInfo::GetAddressURL(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->AddressURL;
}

void NLevelInfo::GetLocalURL(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->LocalURL;
}
