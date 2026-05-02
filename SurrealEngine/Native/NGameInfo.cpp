
#include "Precomp.h"
#include "NGameInfo.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "Package/PackageManager.h"

#include <sstream>

void NGameInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("GameInfo", "GetNetworkNumber", &NGameInfo::GetNetworkNumber, 0);
	RegisterVMNativeFunc_5("GameInfo", "ParseKillMessage", &NGameInfo::ParseKillMessage, 0);
	if (engine->LaunchInfo.IsUnreal1_227())
	{
		RegisterVMNativeFunc_2("GameInfo", "GetNetworkNumber", &NGameInfo::GetNetworkNumber_U227, 0);
		RegisterVMNativeFunc_2("GameInfo", "LoadTravelInventory", &NGameInfo::LoadTravelInventory_U227, 920);
		RegisterVMNativeFunc_2("GameInfo", "MakeColorCode", &NGameInfo::MakeColorCode_U227, 0);
		RegisterVMNativeFunc_2("GameInfo", "StripColorCodes", &NGameInfo::StripColorCodes_U227, 0);
	}

}

void NGameInfo::GetNetworkNumber(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = "0";
	LogUnimplemented("GetNetworkNumber");
}

void NGameInfo::ParseKillMessage(const std::string& KillerName, const std::string& VictimName, const std::string& WeaponName, const std::string& DeathMessage, std::string& ReturnValue)
{
	size_t len = DeathMessage.size();
	std::string result;
	result.reserve(DeathMessage.size() * 2);
	for (size_t i = 0; i < len; i++)
	{
		if (DeathMessage[i] == '%' && i + 1 < len)
		{
			if (DeathMessage[i + 1] == 'k')
			{
				result += KillerName;
				i++;
			}
			else if (DeathMessage[i + 1] == 'o')
			{
				result += VictimName;
				i++;
			}
			else if (DeathMessage[i + 1] == 'w')
			{
				result += WeaponName;
				i++;
			}
		}
		else
		{
			result.push_back(DeathMessage[i]);
		}
	}

	ReturnValue = result;
}

void NGameInfo::GetNetworkNumber_U227(UObject* Self, std::string& ReturnValue)
{
	// auto GISelf = UObject::Cast<UGameInfo>(Self);
	LogUnimplemented("GameInfo.GetNetworkNumber() [U227]");
	ReturnValue = "";
}

void NGameInfo::LoadTravelInventory_U227(UObject* Other, BitfieldBool& ReturnValue)
{
	// auto PPOther = UObject::Cast<UPlayerPawn>(Other);
	LogUnimplemented("GameInfo.LoadTravelInventory() [U227]");
	ReturnValue = false;
}

void NGameInfo::MakeColorCode_U227(const Color& color, std::string& ReturnValue)
{
	std::stringstream ss;
	ss << "#";
	ss << std::hex << std::setfill('0') << std::setw(2);
	ss << color.R << color.G << color.B << color.A;

	ReturnValue = ss.str();
}

void NGameInfo::StripColorCodes_U227(std::string& S, std::string& ReturnValue)
{
	auto pos = S.find('#');

	while (pos != std::string::npos)
	{
		S.erase(pos, 9); // #RRGGBBAA
		pos = S.find('#');
	}

	ReturnValue = S;
}
