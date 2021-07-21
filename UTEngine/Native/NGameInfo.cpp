
#include "Precomp.h"
#include "NGameInfo.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NGameInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("GameInfo", "GetNetworkNumber", &NGameInfo::GetNetworkNumber, 0);
	RegisterVMNativeFunc_5("GameInfo", "ParseKillMessage", &NGameInfo::ParseKillMessage, 0);
}

void NGameInfo::GetNetworkNumber(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = "0";
	engine->Log.push_back("Warning: GetNetworkNumber not implemented");
}

void NGameInfo::ParseKillMessage(const std::string& KillerName, const std::string& VictimName, const std::string& WeaponName, const std::string& DeathMessage, std::string& ReturnValue)
{
	ReturnValue = KillerName + " killed " + VictimName + " using " + WeaponName + " while saying " + DeathMessage;
}
