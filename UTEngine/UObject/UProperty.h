#pragma once

#include "UClass.h"

enum class PropertyFlags : uint32_t
{
	Edit = 0x00000001, // Property is user - settable in the editor.
	Const = 0x00000002, // Actor's property always matches class's default actor property.
	Input = 0x00000004, // Variable is writable by the input system.
	ExportObject = 0x00000008, // Object can be exported with actor.
	OptionalParm = 0x00000010, // Optional parameter(if Param is set).
	Net = 0x00000020, // Property is relevant to network replication
	ConstRef = 0x00000040, // Reference to a constant object.
	Parm = 0x00000080, // Function / When call parameter
	OutParm = 0x00000100, // Value is copied out after function call.
	SkipParm = 0x00000200, // Property is a short - circuitable evaluation function parm.
	ReturnParm = 0x00000400, // Return value.
	CoerceParm = 0x00000800, // Coerce args into this function parameter
	Native = 0x00001000, // Property is native : C++ code is responsible for serializing it.
	Transient = 0x00002000, // Property is transient : shouldn't be saved, zerofilled at load time.
	Config = 0x00004000, // Property should be loaded / saved as permanent profile.
	Localized = 0x00008000, // Property should be loaded as localizable text
	Travel = 0x00010000, // Property travels across levels / servers.
	EditConst = 0x00020000, // Property is uneditable in the editor
	GlobalConfig = 0x00040000, // Load config from base class, not subclass.
	OnDemand = 0x00100000, // Object or dynamic array loaded on demand only.
	New = 0x00200000, // Automatically create inner object
	NeedCtorLink = 0x00400000 // Fields need construction / destruction
};

inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b) { return (PropertyFlags)((uint32_t)a | (uint32_t)b); }
inline PropertyFlags operator&(PropertyFlags a, PropertyFlags b) { return (PropertyFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(PropertyFlags value, PropertyFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(PropertyFlags value, PropertyFlags flags) { return (uint32_t)(value & flags) != 0; }

class UProperty : public UField
{
public:
	using UField::UField;
	void Load(ObjectStream* stream) override;

	uint32_t ArrayDimension = 0;
	PropertyFlags PropFlags = {};
	std::string Category;
	uint16_t ReplicationOffset = 0;
};

class UByteProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;

	UEnum* EnumType = nullptr; // null if it is a normal byte, other for a reference to the Enum type object
};

class UObjectProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;

	UClass* ObjectClass = nullptr;
};

class UFixedArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;

	UProperty* Inner = nullptr;
};

class UArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;

	UProperty* Inner = nullptr;
};

class UMapProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;

	UProperty* Key = nullptr;
	UProperty* Value = nullptr;
};

class UClassProperty : public UObjectProperty
{
public:
	using UObjectProperty::UObjectProperty;
	void Load(ObjectStream* stream) override;

	UClass* MetaClass = nullptr;
};

class UStructProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;

	UStruct* Struct = nullptr;
};

class UIntProperty : public UProperty
{
public:
	using UProperty::UProperty;
};

class UBoolProperty : public UProperty
{
public:
	using UProperty::UProperty;
};

class UFloatProperty : public UProperty
{
public:
	using UProperty::UProperty;
};

class UNameProperty : public UProperty
{
public:
	using UProperty::UProperty;
};

class UStrProperty : public UProperty
{
public:
	using UProperty::UProperty;
};

class UStringProperty : public UProperty
{
public:
	using UProperty::UProperty;
};
