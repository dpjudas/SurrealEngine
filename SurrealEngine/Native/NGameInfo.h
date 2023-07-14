#pragma once

#include "UObject/UObject.h"

class NGameInfo
{
public:
	static void RegisterFunctions();

	static void GetNetworkNumber(UObject* Self, std::string& ReturnValue);
	static void ParseKillMessage(const std::string& KillerName, const std::string& VictimName, const std::string& WeaponName, const std::string& DeathMessage, std::string& ReturnValue);
};
