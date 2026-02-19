
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
	LogUnimplemented("ScriptedPawn.AddCarcass");
}

void NScriptedPawn::ConBindEvents(UObject* Self)
{
	LogUnimplemented("ScriptedPawn.ConBindEvents");
}

void NScriptedPawn::GetAllianceType(UObject* Self, const NameString& AllianceName, uint8_t& ReturnValue)
{
	LogUnimplemented("ScriptedPawn.GetAllianceType");
	ReturnValue = 0;
}

void NScriptedPawn::GetPawnAllianceType(UObject* Self, UObject* QueryPawn, uint8_t& ReturnValue)
{
	LogUnimplemented("ScriptedPawn.GetPawnAllianceType");
	ReturnValue = 0;
}

void NScriptedPawn::HaveSeenCarcass(UObject* Self, const NameString& CarcassName, BitfieldBool& ReturnValue)
{
	LogUnimplemented("ScriptedPawn.HaveSeenCarcass");
	ReturnValue = false;
}

void NScriptedPawn::IsValidEnemy(UObject* Self, UObject* TestEnemy, BitfieldBool* bCheckAlliance, BitfieldBool& ReturnValue)
{
	LogUnimplemented("ScriptedPawn.IsValidEnemy");
	ReturnValue = false;
}
