
#include "Precomp.h"
#include "NLevelInfo.h"
#include "Engine.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"

void NLevelInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_1("LevelInfo", "GetAddressURL", &NLevelInfo::GetAddressURL, 0);
	RegisterVMNativeFunc_1("LevelInfo", "GetLocalURL", &NLevelInfo::GetLocalURL, 0);
	RegisterVMNativeFunc_0("LevelInfo", "InitEventManager", &NLevelInfo::InitEventManager, 650);
	if (engine->LaunchInfo.IsUnreal1_227())
		RegisterVMNativeFunc_2("LevelInfo", "GetLocZone", &NLevelInfo::GetLocZone_U227, 1709);
}

void NLevelInfo::GetAddressURL(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->URL.GetAddressURL();
}

void NLevelInfo::GetLocalURL(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->URL.ToString();
}

void NLevelInfo::GetLocZone_U227(UObject* Self, const vec3& Pos, PointRegion& ReturnValue)
{
	ReturnValue = UObject::Cast<ULevelInfo>(Self)->GetLocZone(Pos);
}

void NLevelInfo::InitEventManager(UObject* Self)
{
	LogUnimplemented("LevelInfo.InitEventManager");
	// Deus Ex
}
