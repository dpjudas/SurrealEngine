#include "Precomp.h"
#include "NFlagBase.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "UObject/UFlag.h"

void NFlagBase::RegisterFunctions()
{
	RegisterVMNativeFunc_3("FlagBase", "CheckFlag", &NFlagBase::CheckFlag, 1120);
	RegisterVMNativeFunc_2("FlagBase", "CreateIterator", &NFlagBase::CreateIterator, 1126);
	RegisterVMNativeFunc_0("FlagBase", "DeleteAllFlags", &NFlagBase::DeleteAllFlags, 1130);
	RegisterVMNativeFunc_1("FlagBase", "DeleteExpiredFlags", &NFlagBase::DeleteExpiredFlags, 1124);
	RegisterVMNativeFunc_3("FlagBase", "DeleteFlag", &NFlagBase::DeleteFlag, 1121);
	RegisterVMNativeFunc_1("FlagBase", "DestroyIterator", &NFlagBase::DestroyIterator, 1129);
	RegisterVMNativeFunc_2("FlagBase", "GetBool", &NFlagBase::GetBool, 1110);
	RegisterVMNativeFunc_2("FlagBase", "GetByte", &NFlagBase::GetByte, 1111);
	RegisterVMNativeFunc_3("FlagBase", "GetExpiration", &NFlagBase::GetExpiration, 1123);
	RegisterVMNativeFunc_2("FlagBase", "GetFloat", &NFlagBase::GetFloat, 1113);
	RegisterVMNativeFunc_2("FlagBase", "GetInt", &NFlagBase::GetInt, 1112);
	RegisterVMNativeFunc_2("FlagBase", "GetName", &NFlagBase::GetName, 1114);
	RegisterVMNativeFunc_4("FlagBase", "GetNextFlag", &NFlagBase::GetNextFlag, 1128);
	RegisterVMNativeFunc_3("FlagBase", "GetNextFlagName", &NFlagBase::GetNextFlagName, 1127);
	RegisterVMNativeFunc_2("FlagBase", "GetRotator", &NFlagBase::GetRotator, 1116);
	RegisterVMNativeFunc_2("FlagBase", "GetVector", &NFlagBase::GetVector, 1115);
	RegisterVMNativeFunc_5("FlagBase", "SetBool", &NFlagBase::SetBool, 1100);
	RegisterVMNativeFunc_5("FlagBase", "SetByte", &NFlagBase::SetByte, 1101);
	RegisterVMNativeFunc_1("FlagBase", "SetDefaultExpiration", &NFlagBase::SetDefaultExpiration, 1125);
	RegisterVMNativeFunc_3("FlagBase", "SetExpiration", &NFlagBase::SetExpiration, 1122);
	RegisterVMNativeFunc_5("FlagBase", "SetFloat", &NFlagBase::SetFloat, 1103);
	RegisterVMNativeFunc_5("FlagBase", "SetInt", &NFlagBase::SetInt, 1102);
	RegisterVMNativeFunc_5("FlagBase", "SetName", &NFlagBase::SetName, 1104);
	RegisterVMNativeFunc_5("FlagBase", "SetRotator", &NFlagBase::SetRotator, 1106);
	RegisterVMNativeFunc_5("FlagBase", "SetVector", &NFlagBase::SetVector, 1105);
}

void NFlagBase::CheckFlag(UObject* Self, const NameString& FlagName, uint8_t flagType, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->CheckFlag(FlagName, flagType);
}

void NFlagBase::CreateIterator(UObject* Self, uint8_t* flagType, int& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->CreateIterator(flagType);
}

void NFlagBase::DeleteAllFlags(UObject* Self)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	FlagBaseSelf->DeleteAllFlags();
}

void NFlagBase::DeleteExpiredFlags(UObject* Self, int criteria)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	FlagBaseSelf->DeleteExpiredFlags(criteria);
}

void NFlagBase::DeleteFlag(UObject* Self, const NameString& FlagName, uint8_t flagType, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->DeleteFlag(FlagName, flagType);
}

void NFlagBase::DestroyIterator(UObject* Self, int Iterator)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	FlagBaseSelf->DestroyIterator(Iterator);
}

void NFlagBase::GetBool(UObject* Self, const NameString& FlagName, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetBool(FlagName);
}

void NFlagBase::GetByte(UObject* Self, const NameString& FlagName, uint8_t& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetByte(FlagName);
}

void NFlagBase::GetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetExpiration(FlagName, flagType);
}

void NFlagBase::GetFloat(UObject* Self, const NameString& FlagName, float& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetFloat(FlagName);
}

void NFlagBase::GetInt(UObject* Self, const NameString& FlagName, int& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetInt(FlagName);
}

void NFlagBase::GetName(UObject* Self, const NameString& FlagName, NameString& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetName(FlagName);
}

void NFlagBase::GetNextFlag(UObject* Self, int Iterator, NameString& FlagName, uint8_t& flagType, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetNextFlag(Iterator, FlagName, flagType);
}

void NFlagBase::GetNextFlagName(UObject* Self, int Iterator, NameString& FlagName, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetNextFlagName(Iterator, FlagName);
}

void NFlagBase::GetRotator(UObject* Self, const NameString& FlagName, Rotator& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetRotator(FlagName);
}

void NFlagBase::GetVector(UObject* Self, const NameString& FlagName, vec3& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->GetVector(FlagName);
}

void NFlagBase::SetBool(UObject* Self, const NameString& FlagName, bool NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetBool(FlagName, NewValue, bAdd, expiration);
}

void NFlagBase::SetByte(UObject* Self, const NameString& FlagName, uint8_t NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetByte(FlagName, NewValue, bAdd, expiration);
}

void NFlagBase::SetDefaultExpiration(UObject* Self, int expiration)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	FlagBaseSelf->SetDefaultExpiration(expiration);
}

void NFlagBase::SetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int expiration)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	FlagBaseSelf->SetExpiration(FlagName, flagType, expiration);
}

void NFlagBase::SetFloat(UObject* Self, const NameString& FlagName, float NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetFloat(FlagName, NewValue, bAdd, expiration);
}

void NFlagBase::SetInt(UObject* Self, const NameString& FlagName, int NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetInt(FlagName, NewValue, bAdd, expiration);
}

void NFlagBase::SetName(UObject* Self, const NameString& FlagName, const NameString& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetName(FlagName, NewValue, bAdd, expiration);
}

void NFlagBase::SetRotator(UObject* Self, const NameString& FlagName, const Rotator& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetRotator(FlagName, NewValue, bAdd, expiration);
}

void NFlagBase::SetVector(UObject* Self, const NameString& FlagName, const vec3& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	auto FlagBaseSelf = UObject::Cast<UFlagBase>(Self);
	ReturnValue = FlagBaseSelf->SetVector(FlagName, NewValue, bAdd, expiration);
}
