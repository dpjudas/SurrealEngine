#pragma once

#include "UState.h"

struct Guid
{
	uint8_t Data[16] = {};
};

struct ClassDependency
{
	UClass* Class = nullptr;
	uint32_t Deep = 0;
	uint32_t ScriptTextCRC = 0;
};

enum class ClassFlags : uint32_t
{
	Abstract = 0x00001,
	Compiled = 0x00002,
	Config = 0x00004,
	Transient = 0x00008,
	Parsed = 0x00010,
	Localized = 0x00020,
	SafeReplace = 0x00040,
	RuntimeStatic = 0x00080,
	NoExport = 0x00100,
	NoUserCreate = 0x00200,
	PerObjectConfig = 0x00400,
	NativeReplication = 0x00800,
};

static uint32_t operator&(const ClassFlags lhs, const ClassFlags rhs) { return uint32_t(lhs) & uint32_t(rhs); }
static uint32_t operator|(const ClassFlags lhs, const ClassFlags rhs) { return uint32_t(lhs) | uint32_t(rhs); }
static uint32_t operator^(const ClassFlags lhs, const ClassFlags rhs) { return uint32_t(lhs) ^ uint32_t(rhs); }

class UClass : public UState
{
public:
	UClass(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	UProperty* GetProperty(const NameString& name);

	template<typename T>
	T* GetDefaultObject()
	{
		// Note: this is a horrible hack. The object is not actually the type returned, but our property getters are members on that type.
		return static_cast<T*>(static_cast<UObject*>(this));
	}

	void SaveConfig() override;

	void LoadProperties(PropertyDataBlock* propertyBlock);
	void SaveProperties(PropertyDataBlock* propertyBlock);

	uint32_t OldClassRecordSize = 0;
	ClassFlags ClsFlags = {};
	Guid ClassGuid;
	Array<ClassDependency> Dependencies;
	Array<int> PackageImports;
	int ClassWithin = 0;
	NameString ClassConfigName;

	UState* GetState(const NameString& name) { auto it = States.find(name); if (it != States.end()) return it->second; else return nullptr; }
	std::map<NameString, UState*> States;

private:
	std::map<NameString, std::string> ParseStructValue(const std::string& text);
};
