#pragma once

#include "UClass.h"
#include "Utils/Convert.h"

struct PropertyHeader;

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

enum class ExpressionValueType
{
	Nothing,
	ValueByte,
	ValueInt,
	ValueBool,
	ValueFloat,
	ValueObject,
	ValueVector,
	ValueRotator,
	ValueString,
	ValueName,
	ValueColor,
	ValueStruct
};

class UProperty : public UField
{
public:
	using UField::UField;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	virtual void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header);
	virtual void LoadStructMemberValue(void* data, ObjectStream* stream);
	virtual void SaveHeader(void* data, PropertyHeader& header);
	virtual void SaveValue(void* data, PackageStreamWriter* stream);

	virtual size_t ElementAlignment() = 0;
	virtual size_t ElementSize() = 0;
	size_t ElementPitch() { size_t align = ElementAlignment(); return (ElementSize() + align - 1) / align * align; }

	void* GetElement(void* data, size_t index) { return static_cast<char*>(data) + index * ElementPitch(); }
	const void* GetElement(const void* data, size_t index) { return static_cast<const char*>(data) + index * ElementPitch(); }

	virtual void ConstructElement(void* data) = 0;
	virtual void CopyConstructElement(void* data, const void* src) = 0;
	virtual void CopyElement(void* data, const void* src) = 0;
	virtual void DestructElement(void* data) = 0;
	virtual bool CompareElement(const void* v1, const void* v2) = 0;
	virtual bool CompareLessElement(const void* v1, const void* v2) = 0;

	size_t ArrayAlignment() { return ElementAlignment(); }
	size_t ArraySize() { return ElementPitch() * (ArrayDimension - 1) + ElementSize(); }
	size_t ArrayPitch() { return ElementPitch() * ArrayDimension; }

	void ConstructArray(void* data);
	void CopyConstructArray(void* data, const void* src);
	void DestructArray(void* data);
	void CopyArray(void* data, const void* src);
	bool CompareArray(const void* v1, const void* v2);
	bool CompareLessArray(const void* v1, const void* v2);

	virtual void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i);
	virtual void GetExportText(std::string& buf, const std::string& whitespace, void* objval, void* defval, int i);
	virtual bool IsDefaultValue(void* val) { return false; }
	virtual std::string PrintValue(const void* data) { return "?"; }
	virtual void SetValueFromString(void* data, const std::string& valueString);

	static void ThrowIfTypeMismatch(const PropertyHeader& header, UnrealPropertyType type);

	int ArrayDimension = 1;
	PropertyFlags PropFlags = {};
	NameString Category;
	uint16_t ReplicationOffset = 0;

	PropertyDataOffset DataOffset = { 0, 1 };
	ExpressionValueType ValueType = ExpressionValueType::Nothing;

protected:
	static std::pair<NameString, std::string> ParseSingleProperty(std::string& propString);
	static std::map<NameString, std::string> ParsePropertiesFromString(std::string propertiesString);
};

class PropertyValue
{
public:
	PropertyValue() = default;
	PropertyValue(const PropertyValue& other);
	PropertyValue(UProperty* prop, const void* data = nullptr);
	~PropertyValue();

	PropertyValue& operator=(const PropertyValue& other);

	bool operator==(const PropertyValue& other) const;
	bool operator<(const PropertyValue& other) const;

	UProperty* Property = nullptr;
	void* Data = nullptr;

private:
	void Create(const PropertyValue& other);
	void Create(UProperty* prop, const void* data);
	void Destroy();
};

template<typename T>
class UPropertyT : public UProperty
{
public:
	using UProperty::UProperty;

	size_t ElementAlignment() override { return alignof(T); }
	size_t ElementSize() override { return sizeof(T); }

	void ConstructElement(void* data) override { new ((char*)data) T(); }
	void CopyConstructElement(void* data, const void* src) override { new ((char*)data) T(*static_cast<const T*>(src)); }
	void DestructElement(void* data) override { static_cast<T*>(data)->~T(); }

	void CopyElement(void* data, const void* src) override { *static_cast<T*>(data) = *static_cast<const T*>(src); }

	bool CompareElement(const void* v1, const void* v2) override { return *static_cast<const T*>(v1) == *static_cast<const T*>(v2); }
	bool CompareLessElement(const void* v1, const void* v2) override { return *static_cast<const T*>(v1) < *static_cast<const T*>(v2); }
};

// Dummy property class so we can keep the virtual functions in UProperty abstract
typedef UPropertyT<int> UAbstractProperty;

class UPointerProperty : public UPropertyT<void*>
{
public:
	using UPropertyT<void*>::UPropertyT;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override { return "pointer"; }
};

class UByteProperty : public UPropertyT<uint8_t>
{
public:
	UByteProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
	void SetValueFromString(void* data, const std::string& valueString) override;

	UEnum* EnumType = nullptr; // null if it is a normal byte, otherwise it is an enum type
};

class UObjectProperty : public UPropertyT<UObject*>
{
public:
	UObjectProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
	void SetValueFromString(void* data, const std::string& valueString) override;

	UClass* ObjectClass = nullptr;
};

class UFixedArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	void CopyElement(void* data, const void* src) override;
	bool CompareElement(const void* v1, const void* v2) override;
	bool CompareLessElement(const void* v1, const void* v2) override;

	std::string PrintValue(const void* data) override;

	UProperty* Inner = nullptr;
	int Count = 0;
};

class UArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	void CopyElement(void* data, const void* src) override;
	bool CompareElement(const void* v1, const void* v2) override;
	bool CompareLessElement(const void* v1, const void* v2) override;

	void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i);
	std::string PrintValue(const void* data) override;

	UProperty* Inner = nullptr;
};

class UMapProperty : public UProperty
{
public:
	using UProperty::UProperty;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	typedef std::map<PropertyValue, PropertyValue> Map;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	void CopyElement(void* data, const void* src) override;
	bool CompareElement(const void* a, const void* b) override;
	bool CompareLessElement(const void* a, const void* b) override;

	std::string PrintValue(const void* data) override;

	UProperty* Key = nullptr;
	UProperty* Value = nullptr;
};

class UClassProperty : public UObjectProperty
{
public:
	using UObjectProperty::UObjectProperty;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;

	UClass* MetaClass = nullptr;
};

class UStructProperty : public UProperty
{
public:
	UStructProperty(NameString name, UClass* base, ObjectFlags flags);

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void CopyElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	bool CompareElement(const void* v1, const void* v2) override;
	bool CompareLessElement(const void* v1, const void* v2) override;

	void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i) override;
	std::string PrintValue(const void* data) override;
	void SetValueFromString(void* data, const std::string& valueString) override;

	UStruct* Struct = nullptr;
};

class UIntProperty : public UPropertyT<int32_t>
{
public:
	UIntProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val);
	void SetValueFromString(void* data, const std::string& valueString) override;
};

class UBoolProperty : public UProperty
{
public:
	UBoolProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	void CopyElement(void* data, const void* src) override;
	bool CompareElement(const void* v1, const void* v2) override;
	bool CompareLessElement(const void* v1, const void* v2) override;

	void SetValueFromString(void* data, const std::string& valueString) override;
	bool IsDefaultValue(void* val) override;
	std::string PrintValue(const void* data) override;

	bool GetBool(const void* data) const;
	void SetBool(void* data, bool value);
};

class UFloatProperty : public UPropertyT<float>
{
public:
	UFloatProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
	void SetValueFromString(void* data, const std::string& valueString) override;
};

class UNameProperty : public UPropertyT<NameString>
{
public:
	UNameProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	void SetValueFromString(void* data, const std::string& valueString) override;
	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
};

class UStrProperty : public UPropertyT<std::string>
{
public:
	UStrProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	void SetValueFromString(void* data, const std::string& valueString) override;
	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
};

class UStringProperty : public UPropertyT<std::string>
{
public:
	UStringProperty(NameString name, UClass* base, ObjectFlags flags);

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	void SetValueFromString(void* data, const std::string& valueString) override;
	std::string PrintValue(const void* data) override;
	bool IsDefaultValue(void* val) override;
};

inline void* PropertyDataBlock::Ptr(const UProperty* prop)
{
	size_t offset = prop->DataOffset.DataOffset;
	if (offset < Size)
		return static_cast<uint8_t*>(Data) + offset;
	Exception::Throw("Property offset out of bounds!");
}

inline const void* PropertyDataBlock::Ptr(const UProperty* prop) const
{
	size_t offset = prop->DataOffset.DataOffset;
	if (offset < Size)
		return static_cast<const uint8_t*>(Data) + offset;
	Exception::Throw("Property offset out of bounds!");
}
