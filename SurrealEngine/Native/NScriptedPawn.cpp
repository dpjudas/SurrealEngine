
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
	auto SelfPawn = UObject::Cast<UScriptedPawn>(Self);
	SelfPawn->AddCarcass(CarcassName);
}

void NScriptedPawn::ConBindEvents(UObject* Self)
{
	auto SelfPawn = UObject::Cast<UScriptedPawn>(Self);
	SelfPawn->ConBindEvents();
}

void NScriptedPawn::GetAllianceType(UObject* Self, const NameString& AllianceName, uint8_t& ReturnValue)
{
	auto SelfPawn = UObject::Cast<UScriptedPawn>(Self);
	ReturnValue = SelfPawn->GetAllianceType(AllianceName);
}

void NScriptedPawn::GetPawnAllianceType(UObject* Self, UObject* QueryPawn, uint8_t& ReturnValue)
{
	auto SelfPawn = UObject::Cast<UScriptedPawn>(Self);
	ReturnValue = SelfPawn->GetPawnAllianceType(QueryPawn);
}

void NScriptedPawn::HaveSeenCarcass(UObject* Self, const NameString& CarcassName, BitfieldBool& ReturnValue)
{
	auto SelfPawn = UObject::Cast<UScriptedPawn>(Self);
	ReturnValue = SelfPawn->HaveSeenCarcass(CarcassName);
}

void NScriptedPawn::IsValidEnemy(UObject* Self, UObject* TestEnemy, std::optional<bool> bCheckAlliance, BitfieldBool& ReturnValue)
{
	auto SelfPawn = UObject::Cast<UScriptedPawn>(Self);
	ReturnValue = SelfPawn->IsValidEnemy(TestEnemy, bCheckAlliance);
}
