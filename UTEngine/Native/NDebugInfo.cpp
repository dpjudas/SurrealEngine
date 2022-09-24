
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
	throw std::runtime_error("DebugInfo.AddTimingData not implemented");
}

void NDebugInfo::Command(UObject* Self, const std::string& Cmd)
{
	throw std::runtime_error("DebugInfo.Command not implemented");
}

void NDebugInfo::GetString(UObject* Self, const std::string& Hash, std::string& ReturnValue)
{
	throw std::runtime_error("DebugInfo.GetString not implemented");
}

void NDebugInfo::SetString(UObject* Self, const std::string& Hash, const std::string& Value)
{
	throw std::runtime_error("DebugInfo.SetString not implemented");
}
