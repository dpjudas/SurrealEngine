#pragma once

#include "Packages/Core/UField.h"

struct PropertyHeader;

enum class PropertyFlags : uint32_t
{
	Edit = 0x00000001,
	Const = 0x00000002,
	Input = 0x00000004,
	ExportObject = 0x00000008,
	OptionalParm = 0x00000010,
	Net = 0x00000020,
	ConstRef = 0x00000040,
	Parm = 0x00000080,
	OutParm = 0x00000100,
	SkipParm = 0x00000200,
	ReturnParm = 0x00000400,
	CoerceParm = 0x00000800,
	Native = 0x00001000,
	Transient = 0x00002000,
	Config = 0x00004000,
	Localized = 0x00008000,
	Travel = 0x00010000,
	EditConst = 0x00020000,
	GlobalConfig = 0x00040000,
	OnDemand = 0x00100000,
	New = 0x00200000,
	NeedCtorLink = 0x00400000
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
	ValueStruct,
	ValueCoords,
	ValueQuat,
	ValueArray
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

	GCAllocation* MarkProperty(GCAllocation* marklist, void* data);
	virtual GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) = 0;

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

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override { return marklist; }
};

// Dummy property class so we can keep the virtual functions in UProperty abstract
typedef UPropertyT<int> UAbstractProperty;

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
