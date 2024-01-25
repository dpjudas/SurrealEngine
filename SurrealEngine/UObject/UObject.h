#pragma once

#include "Package/ObjectStream.h"
#include "Math/vec.h"
#include "Math/mat.h"
#include "Math/rotator.h"
#include "PropertyOffsets.h"
#include <set>

class UObject;
class UClass;
class UProperty;
class Package;
class Frame;
enum class EventName;

enum UnrealPropertyType
{
	UPT_Invalid,
	UPT_Byte,
	UPT_Int,
	UPT_Bool,
	UPT_Float,
	UPT_Object,
	UPT_Name,
	UPT_String,
	UPT_Class,
	UPT_Array,
	UPT_Struct,
	UPT_Vector,
	UPT_Rotator,
	UPT_Str,
	UPT_Map,
	UPT_FixedArray
};

enum UnrealUPropertyStruct
{
	UPS_Invalid,
	UPS_Vector,
	UPS_Matrix,
	UPS_Plane,
	UPS_Sphere,
	UPS_Scale,
	UPS_Coords,
	UPS_ModelCoords,
	UPS_Rotator,
	UPS_Box,
	UPS_Color,
	UPS_Palette,
	UPS_Mipmap,
	UPS_PointRegion
};

struct PropertyHeader
{
	UnrealPropertyType type;
	int arrayIndex = 0;
	bool boolValue = false;
	NameString structName;
	int size = 0;
};

enum UnrealSheerAxis
{
	SHEER_None = 0,
	SHEER_XY = 1,
	SHEER_XZ = 2,
	SHEER_YX = 3,
	SHEER_YZ = 4,
	SHEER_ZX = 5,
	SHEER_ZY = 6,
};

class Color
{
public:
	uint8_t R, G, B, A;
};

inline bool operator==(const Color& a, const Color& b) { return a.R == b.R && a.G == b.G && a.B == b.B && a.A == b.A; }
inline bool operator!=(const Color& a, const Color& b) { return a.R != b.R || a.G != b.G || a.B != b.B || a.A != b.A; }

struct IpAddr
{
	int32_t Addr;
	int32_t Port;
};

class ObjectDelayLoad
{
public:
	ObjectDelayLoad(Package* package, int index, NameString objName, UClass* cls) : package(package), Index(index), ObjName(objName), Class(cls) { }

	Package* package = nullptr;
	int Index = 0;
	NameString ObjName;
	UClass* Class = nullptr;
};

struct BitfieldBool
{
	uint32_t* Ptr;
	uint32_t Mask;

	bool Get() const
	{
		return ((*Ptr) & Mask) != 0;
	}

	void Set(bool value)
	{
		if (value)
			(*Ptr) |= Mask;
		else
			(*Ptr) &= ~Mask;
	}

	operator bool() const { return Get(); }
	BitfieldBool& operator=(bool v) { Set(v); return *this; }
};

class PropertyDataView
{
public:
	PropertyDataView() = default;
	PropertyDataView(void* data, size_t size) : d(data), s(size) {}
	void* data() const { return d; }
	size_t size() const { return s; }
private:
	void* d = nullptr;
	size_t s = 0;
};

class PropertyDataBlock
{
public:
	PropertyDataBlock() = default;
	~PropertyDataBlock() { Reset(); }

	void Init(UClass* cls);
	void ReadProperties(ObjectStream* stream);

	void* Ptr(const UProperty* prop);
	void* Ptr(size_t offset);
	const void* Ptr(const UProperty* prop) const;
	const void* Ptr(size_t offset) const;

	template<typename T>
	T& Value(size_t offset) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(Data) + offset); }

	void* Data = nullptr;
	size_t Size = 0;
	UClass* Class = nullptr;

private:
	void Reset();

	PropertyDataBlock(const PropertyDataBlock&) = delete;
	PropertyDataBlock& operator=(const PropertyDataBlock&) = delete;
};

class UObject
{
public:
	UObject(NameString name, UClass* base, ObjectFlags flags);
	virtual ~UObject() = default;

	void LoadNow();
	virtual void Load(ObjectStream* stream);

	bool HasProperty(const NameString& name) const;
	void* GetProperty(const NameString& name);
	const void* GetProperty(const NameString& name) const;
	PropertyDataOffset GetPropertyDataOffset(const NameString& name) const;

	virtual std::string GetPropertyAsString(const NameString& name) const;
	virtual void SetPropertyFromString(const NameString& name, const std::string& value);

	virtual void SaveConfig();

	uint8_t GetByte(const NameString& name) const;
	uint32_t GetInt(const NameString& name) const;
	bool GetBool(const NameString& name) const;
	float GetFloat(const NameString& name) const;
	vec3 GetVector(const NameString& name) const;
	Rotator GetRotator(const NameString& name) const;
	const std::string& GetString(const NameString& name) const;
	const NameString& GetName(const NameString& name) const;
	UObject* GetUObject(const NameString& name);
	Color GetColor(const NameString& name);

	void SetByte(const NameString& name, uint8_t value);
	void SetInt(const NameString& name, uint32_t value);
	void SetBool(const NameString& name, bool value);
	void SetFloat(const NameString& name, float value);
	void SetVector(const NameString& name, const vec3& value);
	void SetRotator(const NameString& name, const Rotator& value);
	void SetString(const NameString& name, const std::string& value);
	void SetName(const NameString& name, const NameString& value);
	void SetObject(const NameString& name, const UObject* value);

	bool IsA(const NameString& className) const;

	bool IsEventEnabled(const NameString& name) const;
	bool IsEventEnabled(EventName name) const;

	void EnableEvent(const NameString& name)
	{
		NameString stateName = GetStateName();
		DisabledEvents[stateName].erase(name);
	}

	void DisableEvent(const NameString& name)
	{
		NameString stateName = GetStateName();
		DisabledEvents[stateName].insert(name);
	}

	NameString GetStateName() const;
	void GotoState(NameString stateName, const NameString& labelName);

	std::string PrintProperties();

	std::map<NameString, std::set<NameString>> DisabledEvents;

	std::unique_ptr<ObjectDelayLoad> DelayLoad;

	NameString Name;
	UClass* Class = nullptr;
	ObjectFlags Flags = ObjectFlags::NoFlags;

	PropertyDataBlock PropertyData;
	std::shared_ptr<Frame> StateFrame;

	template<typename T>
	T& Value(PropertyDataOffset offset) { return *static_cast<T*>(PropertyData.Ptr(offset.DataOffset)); }

#ifdef _MSC_VER
	// MSVC has a non-standard extension that makes this possible
	template<> bool& Value(PropertyDataOffset offset) = delete; // Booleans must use BoolValue
#endif

	BitfieldBool BoolValue(PropertyDataOffset offset) { BitfieldBool b; b.Ptr = static_cast<uint32_t*>(PropertyData.Ptr(offset.DataOffset)); b.Mask = offset.BitfieldMask; return b; }

	template<typename T>
	T* FixedArray(PropertyDataOffset offset) { return static_cast<T*>(PropertyData.Ptr(offset.DataOffset)); }

	template<typename T>
	static T* Cast(UObject* obj)
	{
		T* target = dynamic_cast<T*>(obj);
		if (target == nullptr && obj != nullptr)
		{
			throw std::runtime_error("Could not cast object " + obj->Name.ToString() + " (class " + GetUClassName(obj).ToString() + ") to " + (std::string)typeid(T).name());
		}
		return target;
	}

	template<typename T>
	static T* TryCast(UObject* obj)
	{
		return dynamic_cast<T*>(obj);
	}

	static NameString GetUClassName(UObject* obj);

	UClass*& uc_Class() { return Value<UClass*>(PropOffsets_Object.Class); } // native
	NameString& uc_Name() { return Value<NameString>(PropOffsets_Object.Name); } // native
	int& uc_ObjectFlags() { return Value<int>(PropOffsets_Object.ObjectFlags); } // native
	int& uc_ObjectInternal() { return Value<int>(PropOffsets_Object.ObjectInternal); } // native
	UObject*& Outer() { return Value<UObject*>(PropOffsets_Object.Outer); } // native
};

template<typename T>
T* ObjectStream::ReadObject()
{
	return UObject::Cast<T>(package->GetUObject(ReadIndex()));
}

#ifndef _MSC_VER
// Same deal with above, but for non-MSVC compilers
template<> bool& UObject::Value(PropertyDataOffset offset) = delete; // Booleans must use BoolValue
#endif
