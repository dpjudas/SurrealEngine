
#include "Precomp.h"
#include "UFlag.h"
#include "Engine.h"

bool UFlagBase::CheckFlag(const NameString& FlagName, uint8_t flagType)
{
	LogUnimplemented("FlagBase.CheckFlag");
	return false;
}

int UFlagBase::CreateIterator(uint8_t* flagType)
{
	LogUnimplemented("FlagBase.CreateIterator");
	return 0;
}

void UFlagBase::DeleteAllFlags()
{
	LogUnimplemented("FlagBase.DeleteAllFlags");
}

void UFlagBase::DeleteExpiredFlags(int criteria)
{
	LogUnimplemented("FlagBase.DeleteExpiredFlags");
}

bool UFlagBase::DeleteFlag(const NameString& FlagName, uint8_t flagType)
{
	LogUnimplemented("FlagBase.DeleteFlag");
	return false;
}

void UFlagBase::DestroyIterator(int Iterator)
{
	LogUnimplemented("FlagBase.DestroyIterator");
}

bool UFlagBase::GetBool(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetBool");
	return false;
}

uint8_t UFlagBase::GetByte(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetByte");
	return 0;
}

int UFlagBase::GetExpiration(const NameString& FlagName, uint8_t flagType)
{
	LogUnimplemented("FlagBase.GetExpiration");
	return 0;
}

float UFlagBase::GetFloat(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetFloat");
	return 0.0f;
}

int UFlagBase::GetInt(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetInt");
	return 0;
}

NameString UFlagBase::GetName(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetName");
	return {};
}

bool UFlagBase::GetNextFlag(int Iterator, NameString& FlagName, uint8_t& flagType)
{
	LogUnimplemented("FlagBase.GetNextFlag");
	return false;
}

bool UFlagBase::GetNextFlagName(int Iterator, NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetNextFlagName");
	return false;
}

Rotator UFlagBase::GetRotator(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetRotator");
	return {};
}

vec3 UFlagBase::GetVector(const NameString& FlagName)
{
	LogUnimplemented("FlagBase.GetVector");
	return vec3(0.0f);
}

bool UFlagBase::SetBool(const NameString& FlagName, bool NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetBool");
	return false;
}

bool UFlagBase::SetByte(const NameString& FlagName, uint8_t NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetByte");
	return false;
}

void UFlagBase::SetDefaultExpiration(int expiration)
{
	LogUnimplemented("FlagBase.SetDefaultExpiration");
}

void UFlagBase::SetExpiration(const NameString& FlagName, uint8_t flagType, int expiration)
{
	LogUnimplemented("FlagBase.SetExpiration");
}

bool UFlagBase::SetFloat(const NameString& FlagName, float NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetFloat");
	return false;
}

bool UFlagBase::SetInt(const NameString& FlagName, int NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetInt");
	return false;
}

bool UFlagBase::SetName(const NameString& FlagName, const NameString& NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetName");
	return false;
}

bool UFlagBase::SetRotator(const NameString& FlagName, const Rotator& NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetRotator");
	return false;
}

bool UFlagBase::SetVector(const NameString& FlagName, const vec3& NewValue, BitfieldBool* bAdd, int* expiration)
{
	LogUnimplemented("FlagBase.SetVector");
	return false;
}
