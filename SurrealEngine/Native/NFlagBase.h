#pragma once

#include "UObject/UObject.h"

class NFlagBase
{
public:
	static void RegisterFunctions();

	static void CheckFlag(UObject* Self, const NameString& FlagName, uint8_t flagType, BitfieldBool& ReturnValue);
	static void CreateIterator(UObject* Self, uint8_t* flagType, int& ReturnValue);
	static void DeleteAllFlags(UObject* Self);
	static void DeleteExpiredFlags(UObject* Self, int criteria);
	static void DeleteFlag(UObject* Self, const NameString& FlagName, uint8_t flagType, BitfieldBool& ReturnValue);
	static void DestroyIterator(UObject* Self, int Iterator);
	static void GetBool(UObject* Self, const NameString& FlagName, BitfieldBool& ReturnValue);
	static void GetByte(UObject* Self, const NameString& FlagName, uint8_t& ReturnValue);
	static void GetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int& ReturnValue);
	static void GetFloat(UObject* Self, const NameString& FlagName, float& ReturnValue);
	static void GetInt(UObject* Self, const NameString& FlagName, int& ReturnValue);
	static void GetName(UObject* Self, const NameString& FlagName, NameString& ReturnValue);
	static void GetNextFlag(UObject* Self, int Iterator, NameString& FlagName, uint8_t& flagType, BitfieldBool& ReturnValue);
	static void GetNextFlagName(UObject* Self, int Iterator, NameString& FlagName, BitfieldBool& ReturnValue);
	static void GetRotator(UObject* Self, const NameString& FlagName, Rotator& ReturnValue);
	static void GetVector(UObject* Self, const NameString& FlagName, vec3& ReturnValue);
	static void SetBool(UObject* Self, const NameString& FlagName, bool NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
	static void SetByte(UObject* Self, const NameString& FlagName, uint8_t NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
	static void SetDefaultExpiration(UObject* Self, int expiration);
	static void SetExpiration(UObject* Self, const NameString& FlagName, uint8_t flagType, int expiration);
	static void SetFloat(UObject* Self, const NameString& FlagName, float NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
	static void SetInt(UObject* Self, const NameString& FlagName, int NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
	static void SetName(UObject* Self, const NameString& FlagName, const NameString& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
	static void SetRotator(UObject* Self, const NameString& FlagName, const Rotator& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
	static void SetVector(UObject* Self, const NameString& FlagName, const vec3& NewValue, BitfieldBool* bAdd, int* expiration, BitfieldBool& ReturnValue);
};
