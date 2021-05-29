#pragma once

#include "Package/ObjectStream.h"
#include "Math/vec.h"

class UObject;
class UClass;
class UProperty;
class Package;

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
	std::string structName;
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

class Rotator
{
public:
	Rotator() = default;
	Rotator(int pitch, int yaw, int roll) : Pitch(pitch), Yaw(yaw), Roll(roll) { }

	int Pitch;
	int Yaw;
	int Roll;

	float PitchDegrees() const { return Pitch * (360.0f / 65536.0f); }
	float YawDegrees() const { return Yaw * (360.0f / 65536.0f); }
	float RollDegrees() const { return Roll * (360.0f / 65536.0f); }
};

class Color
{
public:
	uint8_t R, G, B, A;
};

inline bool operator==(const Rotator& a, const Rotator& b) { return a.Pitch == b.Pitch && a.Yaw == b.Yaw && a.Roll == b.Roll; }
inline bool operator!=(const Rotator& a, const Rotator& b) { return a.Pitch != b.Pitch || a.Yaw != b.Yaw || a.Roll != b.Roll; }

class ObjectDelayLoad
{
public:
	ObjectDelayLoad(Package* package, int index, std::string objName, UClass* cls) : package(package), Index(index), ObjName(objName), Class(cls) { }

	Package* package = nullptr;
	int Index = 0;
	std::string ObjName;
	UClass* Class = nullptr;
};

class PropertyDataBlock
{
public:
	PropertyDataBlock() = default;
	~PropertyDataBlock() { Reset(); }

	void Init(UClass* cls);
	void ReadProperties(ObjectStream* stream);

	void* Ptr(const UProperty* prop);
	const void* Ptr(const UProperty* prop) const;

	void* Data = nullptr;
	UClass* Class = nullptr;

private:
	void Reset();

	PropertyDataBlock(const PropertyDataBlock&) = delete;
	PropertyDataBlock& operator=(const PropertyDataBlock&) = delete;
};

class UObject
{
public:
	UObject(std::string name, UClass* base, ObjectFlags flags);
	virtual ~UObject() = default;

	void LoadNow();
	virtual void Load(ObjectStream* stream);

	bool HasProperty(const std::string& name) const;
	void* GetProperty(const std::string& name);
	const void* GetProperty(const std::string& name) const;

	uint8_t GetByte(const std::string& name) const;
	uint32_t GetInt(const std::string& name) const;
	bool GetBool(const std::string& name) const;
	float GetFloat(const std::string& name) const;
	vec3 GetVector(const std::string& name) const;
	Rotator GetRotator(const std::string& name) const;
	const std::string& GetString(const std::string& name) const;
	UObject* GetUObject(const std::string& name);

	std::unique_ptr<ObjectDelayLoad> DelayLoad;

	std::string Name;
	UClass* Base = nullptr;
	ObjectFlags Flags = ObjectFlags::None;

	PropertyDataBlock PropertyData;

	template<typename T>
	static T* Cast(UObject* obj)
	{
		T* target = dynamic_cast<T*>(obj);
		if (target == nullptr && obj != nullptr)
		{
			throw std::runtime_error("Could not cast object " + obj->Name + " (class " + GetUClassName(obj) + ") to " + (std::string)typeid(T).name());
		}
		return target;
	}

	template<typename T>
	static T* TryCast(UObject* obj)
	{
		return dynamic_cast<T*>(obj);
	}

	static std::string GetUClassName(UObject* obj);
};
