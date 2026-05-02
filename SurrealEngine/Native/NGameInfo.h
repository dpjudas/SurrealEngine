#pragma once

#include "UObject/UObject.h"

class NGameInfo
{
public:
	static void RegisterFunctions();

	static void GetNetworkNumber(UObject* Self, std::string& ReturnValue);
	static void ParseKillMessage(const std::string& KillerName, const std::string& VictimName, const std::string& WeaponName, const std::string& DeathMessage, std::string& ReturnValue);
	static void GetNetworkNumber_U227(UObject* Self, std::string& ReturnValue);
	static void LoadTravelInventory_U227(UObject* Other, BitfieldBool& ReturnValue);
	static void MakeColorCode_U227(const Color& color, std::string& ReturnValue);
	static void StripColorCodes_U227(std::string& S, std::string& ReturnValue);
};
