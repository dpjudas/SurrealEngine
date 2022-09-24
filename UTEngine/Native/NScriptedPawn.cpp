
#include "Precomp.h"
#include "NScriptedPawn.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NScriptedPawn::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ScriptedPawn", "AddCarcass", &NScriptedPawn::AddCarcass, 2109);
	RegisterVMNativeFunc_0("ScriptedPawn", "ConBindEvents", &NScriptedPawn::ConBindEvents, 2102);
	RegisterVMNativeFunc_2("ScriptedPawn", "GetAllianceType", &NScriptedPawn::GetAllianceType, 2106);
	RegisterVMNativeFunc_2("ScriptedPawn", "GetPawnAllianceType", &NScriptedPawn::GetPawnAllianceType, 2107);
	RegisterVMNativeFunc_2("ScriptedPawn", "HaveSeenCarcass", &NScriptedPawn::HaveSeenCarcass, 2108);
	RegisterVMNativeFunc_3("ScriptedPawn", "IsValidEnemy", &NScriptedPawn::IsValidEnemy, 2105);
}

void NScriptedPawn::AddCarcass(UObject* Self, const NameString& CarcassName)
{
	throw std::runtime_error("ScriptedPawn.AddCarcass not implemented");
}

void NScriptedPawn::ConBindEvents(UObject* Self)
{
	throw std::runtime_error("ScriptedPawn.ConBindEvents not implemented");
}

void NScriptedPawn::GetAllianceType(UObject* Self, const NameString& AllianceName, uint8_t& ReturnValue)
{
	throw std::runtime_error("ScriptedPawn.GetAllianceType not implemented");
}

void NScriptedPawn::GetPawnAllianceType(UObject* Self, UObject* QueryPawn, uint8_t& ReturnValue)
{
	throw std::runtime_error("ScriptedPawn.GetPawnAllianceType not implemented");
}

void NScriptedPawn::HaveSeenCarcass(UObject* Self, const NameString& CarcassName, bool& ReturnValue)
{
	throw std::runtime_error("ScriptedPawn.HaveSeenCarcass not implemented");
}

void NScriptedPawn::IsValidEnemy(UObject* Self, UObject* TestEnemy, bool* bCheckAlliance, bool& ReturnValue)
{
	throw std::runtime_error("ScriptedPawn.IsValidEnemy not implemented");
}
