#pragma once

#include "Packages/Extension/UExtensionObject.h"

class UFlag;

enum class EFlagType : uint8_t
{
	Bool,
	Byte,
	Int,
	Float,
	Name,
	Vector,
	Rotator,
};

class UFlagBase : public UExtensionObject
{
public:
	using UExtensionObject::UExtensionObject;

	bool CheckFlag(const NameString& FlagName, uint8_t flagType);
	int CreateIterator(std::optional<uint8_t> flagType);
	void DeleteAllFlags();
	void DeleteExpiredFlags(int criteria);
	bool DeleteFlag(const NameString& FlagName, uint8_t flagType);
	void DestroyIterator(int Iterator);
	bool GetBool(const NameString& FlagName);
	uint8_t GetByte(const NameString& FlagName);
	int GetExpiration(const NameString& FlagName, uint8_t flagType);
	float GetFloat(const NameString& FlagName);
	int GetInt(const NameString& FlagName);
	NameString GetName(const NameString& FlagName);
	bool GetNextFlag(int Iterator, NameString& FlagName, uint8_t& flagType);
	bool GetNextFlagName(int Iterator, NameString& FlagName);
	Rotator GetRotator(const NameString& FlagName);
	vec3 GetVector(const NameString& FlagName);
	bool SetBool(const NameString& FlagName, bool NewValue, std::optional<bool> bAdd, std::optional<int> expiration);
	bool SetByte(const NameString& FlagName, uint8_t NewValue, std::optional<bool> bAdd, std::optional<int> expiration);
	void SetDefaultExpiration(int expiration);
	void SetExpiration(const NameString& FlagName, uint8_t flagType, int expiration);
	bool SetFloat(const NameString& FlagName, float NewValue, std::optional<bool> bAdd, std::optional<int> expiration);
	bool SetInt(const NameString& FlagName, int NewValue, std::optional<bool> bAdd, std::optional<int> expiration);
	bool SetName(const NameString& FlagName, const NameString& NewValue, std::optional<bool> bAdd, std::optional<int> expiration);
	bool SetRotator(const NameString& FlagName, const Rotator& NewValue, std::optional<bool> bAdd, std::optional<int> expiration);
	bool SetVector(const NameString& FlagName, const vec3& NewValue, std::optional<bool> bAdd, std::optional<int> expiration);

	UFlag* GetFlag(const NameString& flagName, uint8_t flagType);

	template<typename T>
	T* GetOrCreateFlag(const NameString& FlagName, std::optional<bool> bAdd, std::optional<int> expiration, EFlagType flagType, const char* flagClassName);

	int& defaultFlagExpiration() { return Value<int>(PropOffsets_FlagBase.defaultFlagExpiration); }
	FixedArrayView<UFlag*, 64> hashTable() { return FixedArray<UFlag*, 64>(PropOffsets_FlagBase.hashTable); }

	struct FlagIterator
	{
		int HashPos = 0;
		uint8_t FlagType = 0;
		bool FlagTypeSet = false;
	};

	std::unordered_map<int, FlagIterator> FlagIterators;
	int NextIterator = 1;

	std::vector<UClass*> FlagClasses;

	static const int HashTableSize = 64;
};
