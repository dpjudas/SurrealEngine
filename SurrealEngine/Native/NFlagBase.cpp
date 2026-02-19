#include "Precomp.h"
#include "NFlagBase.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

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
	LogUnimplemented("FlagBase.CheckFlag");
	ReturnValue = false;
}

void NFlagBase::CreateIterator(UObject* Self, uint8_t* flagType, int& ReturnValue)
{
	LogUnimplemented("FlagBase.CreateIterator");
	ReturnValue = 0;
}

void NFlagBase::DeleteAllFlags(UObject* Self)
{
	LogUnimplemented("FlagBase.DeleteAllFlags");
}

void NFlagBase::DeleteExpiredFlags(UObject* Self, int criteria)
{
	LogUnimplemented("FlagBase.DeleteExpiredFlags");
}

void NFlagBase::DeleteFlag(UObject* Self, const NameString& FlagName, uint8_t flagType, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.DeleteFlag");
	ReturnValue = false;
}

void NFlagBase::DestroyIterator(UObject* Self, int Iterator)
{
	LogUnimplemented("FlagBase.DestroyIterator");
}

void NFlagBase::GetBool(UObject* Self, const NameString& FlagName, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.GetBool");
	ReturnValue = false;
}

void NFlagBase::GetByte(UObject* Self, const NameString& FlagName, uint8_t& ReturnValue)
{
	LogUnimplemented("FlagBase.GetByte");
	ReturnValue = false;
}

void NFlagBase::GetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int& ReturnValue)
{
	LogUnimplemented("FlagBase.GetExpiration");
	ReturnValue = 0;
}

void NFlagBase::GetFloat(UObject* Self, const NameString& FlagName, float& ReturnValue)
{
	LogUnimplemented("FlagBase.GetFloat");
	ReturnValue = 0.0f;
}

void NFlagBase::GetInt(UObject* Self, const NameString& FlagName, int& ReturnValue)
{
	LogUnimplemented("FlagBase.GetInt");
	ReturnValue = 0;
}

void NFlagBase::GetName(UObject* Self, const NameString& FlagName, NameString& ReturnValue)
{
	LogUnimplemented("FlagBase.GetName");
	ReturnValue = {};
}

void NFlagBase::GetNextFlag(UObject* Self, int Iterator, NameString& FlagName, uint8_t& flagType, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.GetNextFlag");
	ReturnValue = false;
}

void NFlagBase::GetNextFlagName(UObject* Self, int Iterator, NameString& FlagName, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.GetNextFlagName");
	ReturnValue = false;
}

void NFlagBase::GetRotator(UObject* Self, const NameString& FlagName, Rotator& ReturnValue)
{
	LogUnimplemented("FlagBase.GetRotator");
	ReturnValue = {};
}

void NFlagBase::GetVector(UObject* Self, const NameString& FlagName, vec3& ReturnValue)
{
	LogUnimplemented("FlagBase.GetVector");
	ReturnValue = {};
}

void NFlagBase::SetBool(UObject* Self, const NameString& FlagName, bool NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetBool");
	ReturnValue = false;
}

void NFlagBase::SetByte(UObject* Self, const NameString& FlagName, uint8_t NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetByte");
	ReturnValue = false;
}

void NFlagBase::SetDefaultExpiration(UObject* Self, int expiration)
{
	LogUnimplemented("FlagBase.SetDefaultExpiration");
}

void NFlagBase::SetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int expiration)
{
	LogUnimplemented("FlagBase.SetExpiration");
}

void NFlagBase::SetFloat(UObject* Self, const NameString& FlagName, float NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetFloat");
	ReturnValue = false;
}

void NFlagBase::SetInt(UObject* Self, const NameString& FlagName, int NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetInt");
	ReturnValue = false;
}

void NFlagBase::SetName(UObject* Self, const NameString& FlagName, const NameString& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetName");
	ReturnValue = false;
}

void NFlagBase::SetRotator(UObject* Self, const NameString& FlagName, const Rotator& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetRotator");
	ReturnValue = false;
}

void NFlagBase::SetVector(UObject* Self, const NameString& FlagName, const vec3& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	LogUnimplemented("FlagBase.SetVector");
	ReturnValue = false;
}
