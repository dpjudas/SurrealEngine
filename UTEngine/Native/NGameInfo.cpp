
#include "Precomp.h"
#include "NGameInfo.h"
#include "VM/NativeFunc.h"

void NGameInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("GameInfo", "GetNetworkNumber", &NGameInfo::GetNetworkNumber, 0);
	RegisterVMNativeFunc_5("GameInfo", "ParseKillMessage", &NGameInfo::ParseKillMessage, 0);
}

void NGameInfo::GetNetworkNumber(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("GameInfo.GetNetworkNumber not implemented");
}

void NGameInfo::ParseKillMessage(const std::string& KillerName, const std::string& VictimName, const std::string& WeaponName, const std::string& DeathMessage, std::string& ReturnValue)
{
	throw std::runtime_error("GameInfo.ParseKillMessage not implemented");
}
