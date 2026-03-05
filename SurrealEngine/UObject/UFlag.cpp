
#include "Precomp.h"
#include "UFlag.h"
#include "Engine.h"
#include "Package/PackageManager.h"

UFlag* UFlagBase::GetFlag(const NameString& flagName, uint8_t flagType)
{
	// To do: calculate a proper hash and use it

	UFlag** table = hashTable();
	for (int i = 0; i < HashTableSize; i++)
	{
		if (table[i] && table[i]->FlagName() == flagName && table[i]->flagType() == flagType)
		{
			return table[i];
		}
	}
	return nullptr;
}

bool UFlagBase::DeleteFlag(const NameString& FlagName, uint8_t flagType)
{
	UFlag** table = hashTable();
	for (int i = 0; i < HashTableSize; i++)
	{
		if (table[i] && table[i]->FlagName() == FlagName && table[i]->flagType() == flagType)
		{
			table[i] = nullptr;
			return true;
		}
	}
	return false;
}

void UFlagBase::DeleteAllFlags()
{
	UFlag** table = hashTable();
	for (int i = 0; i < HashTableSize; i++)
		table[i] = nullptr;
}

void UFlagBase::DeleteExpiredFlags(int criteria)
{
	LogUnimplemented("FlagBase.DeleteExpiredFlags");
}

bool UFlagBase::CheckFlag(const NameString& FlagName, uint8_t flagType)
{
	return GetFlag(FlagName, flagType) != nullptr;
}

int UFlagBase::CreateIterator(uint8_t* flagType)
{
	int handle = NextIterator++;
	FlagIterator flagIt;
	if (flagType)
	{
		flagIt.FlagType = *flagType;
		flagIt.FlagTypeSet = true;
	}
	FlagIterators[handle] = flagIt;
	return handle;
}

void UFlagBase::DestroyIterator(int Iterator)
{
	FlagIterators.erase(Iterator);
}

bool UFlagBase::GetNextFlag(int Iterator, NameString& FlagName, uint8_t& flagType)
{
	FlagName = {}; // ResetFlags() expects this to be set to '' if no more flags were found

	auto it = FlagIterators.find(Iterator);
	if (it == FlagIterators.end() || it->second.HashPos == HashTableSize)
		return false;

	UFlag** table = hashTable();
	FlagIterator& flagIt = it->second;

	for (int i = flagIt.HashPos; i < HashTableSize; i++)
	{
		if (table[i] && (!flagIt.FlagTypeSet || table[i]->flagType() == flagIt.FlagType))
		{
			FlagName = table[i]->FlagName();
			flagType = table[i]->flagType();
			flagIt.HashPos = i + 1;
			return true;
		}
	}

	flagIt.HashPos = HashTableSize;
	return false;
}

bool UFlagBase::GetNextFlagName(int Iterator, NameString& FlagName)
{
	uint8_t flagType = 0;
	return GetNextFlag(Iterator, FlagName, flagType);
}

bool UFlagBase::GetBool(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Bool))
		return UObject::Cast<UFlagBool>(flag)->bValue();
	return false;
}

uint8_t UFlagBase::GetByte(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Byte))
		return UObject::Cast<UFlagByte>(flag)->byteValue();
	return 0;
}

int UFlagBase::GetExpiration(const NameString& FlagName, uint8_t flagType)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Bool))
		return flag->expiration();
	return 0;
}

float UFlagBase::GetFloat(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Float))
		return UObject::Cast<UFlagFloat>(flag)->floatValue();
	return 0.0f;
}

int UFlagBase::GetInt(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Int))
		return UObject::Cast<UFlagInt>(flag)->intValue();
	return 0;
}

NameString UFlagBase::GetName(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Name))
		return UObject::Cast<UFlagName>(flag)->nameValue();
	return {};
}

Rotator UFlagBase::GetRotator(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Rotator))
		return UObject::Cast<UFlagRotator>(flag)->rotatorValue();
	return {};
}

vec3 UFlagBase::GetVector(const NameString& FlagName)
{
	if (UFlag* flag = GetFlag(FlagName, (uint8_t)EFlagType::Vector))
		return UObject::Cast<UFlagVector>(flag)->vectorValue();
	return vec3(0.0f);
}

void UFlagBase::SetDefaultExpiration(int expiration)
{
	defaultFlagExpiration() = expiration;
}

void UFlagBase::SetExpiration(const NameString& FlagName, uint8_t flagType, int expiration)
{
	if (UFlag* flag = GetFlag(FlagName, flagType))
		flag->expiration() = expiration;
}

template<typename T>
T* UFlagBase::GetOrCreateFlag(const NameString& FlagName, BitfieldBool* bAdd, int* expiration, EFlagType flagType, const char* flagClassName)
{
	int newExpiration = expiration ? *expiration : defaultFlagExpiration();

	// Try get the flag
	if (auto flag = UObject::Cast<T>(GetFlag(FlagName, (uint8_t)flagType)))
	{
		flag->expiration() = newExpiration;
		return flag;
	}

	// Flag didn't exist. Create the flag, if requested
	if (!bAdd || *bAdd)
	{
		size_t classIndex = (size_t)flagType;
		if (classIndex >= FlagClasses.size())
			FlagClasses.resize(classIndex + 1);

		UClass*& cls = FlagClasses[classIndex];
		if (!cls)
		{
			cls = engine->packages->FindClass(flagClassName);
			if (cls == nullptr)
				throw std::runtime_error(std::string("Could not find class ") + flagClassName);
		}

		auto flag = UObject::Cast<T>(engine->packages->GetTransientPackage()->NewObject(FlagName, cls, ObjectFlags::Transient));

		UFlag** table = hashTable();
		for (int i = 0; i < HashTableSize; i++)
		{
			if (!table[i])
			{
				flag->flagHash() = i; // Lets pretend until we have a proper hash
				table[i] = flag;
				return flag;
			}
		}
	}

	return nullptr;
}

bool UFlagBase::SetBool(const NameString& FlagName, bool NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagBool>(FlagName, bAdd, expiration, EFlagType::Bool, "Extension.FlagBool"))
	{
		flag->bValue() = NewValue;
		return true;
	}
	return false;
}

bool UFlagBase::SetByte(const NameString& FlagName, uint8_t NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagByte>(FlagName, bAdd, expiration, EFlagType::Byte, "Extension.FlagByte"))
	{
		flag->byteValue() = NewValue;
		return true;
	}
	return false;
}

bool UFlagBase::SetFloat(const NameString& FlagName, float NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagFloat>(FlagName, bAdd, expiration, EFlagType::Float, "Extension.FlagFloat"))
	{
		flag->floatValue() = NewValue;
		return true;
	}
	return false;
}

bool UFlagBase::SetInt(const NameString& FlagName, int NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagInt>(FlagName, bAdd, expiration, EFlagType::Int, "Extension.FlagInt"))
	{
		flag->intValue() = NewValue;
		return true;
	}
	return false;
}

bool UFlagBase::SetName(const NameString& FlagName, const NameString& NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagName>(FlagName, bAdd, expiration, EFlagType::Name, "Extension.FlagName"))
	{
		flag->nameValue() = NewValue;
		return true;
	}
	return false;
}

bool UFlagBase::SetRotator(const NameString& FlagName, const Rotator& NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagRotator>(FlagName, bAdd, expiration, EFlagType::Rotator, "Extension.FlagRotator"))
	{
		flag->rotatorValue() = NewValue;
		return true;
	}
	return false;
}

bool UFlagBase::SetVector(const NameString& FlagName, const vec3& NewValue, BitfieldBool* bAdd, int* expiration)
{
	if (auto flag = GetOrCreateFlag<UFlagVector>(FlagName, bAdd, expiration, EFlagType::Vector, "Extension.FlagVector"))
	{
		flag->vectorValue() = NewValue;
		return true;
	}
	return false;
}
