
#include "Precomp.h"
#include "NDebugInfo.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NDebugInfo::RegisterFunctions()
{
	RegisterVMNativeFunc_3("DebugInfo", "AddTimingData", &NDebugInfo::AddTimingData, 4000);
	RegisterVMNativeFunc_1("DebugInfo", "Command", &NDebugInfo::Command, 4001);
	RegisterVMNativeFunc_2("DebugInfo", "GetString", &NDebugInfo::GetString, 4003);
	RegisterVMNativeFunc_2("DebugInfo", "SetString", &NDebugInfo::SetString, 4002);
}

void NDebugInfo::AddTimingData(UObject* Self, const std::string& obj, const std::string& objName, int Time)
{
	LogUnimplemented("DebugInfo.AddTimingData");
}

void NDebugInfo::Command(UObject* Self, const std::string& Cmd)
{
	LogUnimplemented("DebugInfo.Command");
}

void NDebugInfo::GetString(UObject* Self, const std::string& Hash, std::string& ReturnValue)
{
	LogUnimplemented("DebugInfo.GetString");
	ReturnValue = "";
}

void NDebugInfo::SetString(UObject* Self, const std::string& Hash, const std::string& Value)
{
	LogUnimplemented("DebugInfo.SetString");
}
