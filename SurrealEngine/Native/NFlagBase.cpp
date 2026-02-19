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
	Exception::Throw("NFlagBase::CheckFlag not implemented");
}

void NFlagBase::CreateIterator(UObject* Self, uint8_t* flagType, int& ReturnValue)
{
	Exception::Throw("NFlagBase::CreateIterator not implemented");
}

void NFlagBase::DeleteAllFlags(UObject* Self)
{
	Exception::Throw("NFlagBase::DeleteAllFlags not implemented");
}

void NFlagBase::DeleteExpiredFlags(UObject* Self, int criteria)
{
	Exception::Throw("NFlagBase::DeleteExpiredFlags not implemented");
}

void NFlagBase::DeleteFlag(UObject* Self, const NameString& FlagName, uint8_t flagType, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::DeleteFlag not implemented");
}

void NFlagBase::DestroyIterator(UObject* Self, int Iterator)
{
	Exception::Throw("NFlagBase::DestroyIterator not implemented");
}

void NFlagBase::GetBool(UObject* Self, const NameString& FlagName, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::GetBool not implemented");
}

void NFlagBase::GetByte(UObject* Self, const NameString& FlagName, uint8_t& ReturnValue)
{
	Exception::Throw("NFlagBase::GetByte not implemented");
}

void NFlagBase::GetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int& ReturnValue)
{
	Exception::Throw("NFlagBase::GetExpiration not implemented");
}

void NFlagBase::GetFloat(UObject* Self, const NameString& FlagName, float& ReturnValue)
{
	Exception::Throw("NFlagBase::GetFloat not implemented");
}

void NFlagBase::GetInt(UObject* Self, const NameString& FlagName, int& ReturnValue)
{
	Exception::Throw("NFlagBase::GetInt not implemented");
}

void NFlagBase::GetName(UObject* Self, const NameString& FlagName, NameString& ReturnValue)
{
	Exception::Throw("NFlagBase::GetName not implemented");
}

void NFlagBase::GetNextFlag(UObject* Self, int Iterator, NameString& FlagName, uint8_t& flagType, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::GetNextFlag not implemented");
}

void NFlagBase::GetNextFlagName(UObject* Self, int Iterator, NameString& FlagName, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::GetNextFlagName not implemented");
}

void NFlagBase::GetRotator(UObject* Self, const NameString& FlagName, Rotator& ReturnValue)
{
	Exception::Throw("NFlagBase::GetRotator not implemented");
}

void NFlagBase::GetVector(UObject* Self, const NameString& FlagName, vec3& ReturnValue)
{
	Exception::Throw("NFlagBase::GetVector not implemented");
}

void NFlagBase::SetBool(UObject* Self, const NameString& FlagName, bool NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetBool not implemented");
}

void NFlagBase::SetByte(UObject* Self, const NameString& FlagName, uint8_t NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetByte not implemented");
}

void NFlagBase::SetDefaultExpiration(UObject* Self, int expiration)
{
	Exception::Throw("NFlagBase::SetDefaultExpiration not implemented");
}

void NFlagBase::SetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int expiration)
{
	Exception::Throw("NFlagBase::SetExpiration not implemented");
}

void NFlagBase::SetFloat(UObject* Self, const NameString& FlagName, float NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetFloat not implemented");
}

void NFlagBase::SetInt(UObject* Self, const NameString& FlagName, int NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetInt not implemented");
}

void NFlagBase::SetName(UObject* Self, const NameString& FlagName, const NameString& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetName not implemented");
}

void NFlagBase::SetRotator(UObject* Self, const NameString& FlagName, const Rotator& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetRotator not implemented");
}

void NFlagBase::SetVector(UObject* Self, const NameString& FlagName, const vec3& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue)
{
	Exception::Throw("NFlagBase::SetVector not implemented");
}
