#pragma once

#include "Package/ObjectStream.h"
#include "Math/vec.h"

class UObject;
class UClass;

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
	Rotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) { }

	float Pitch;
	float Yaw;
	float Roll;
};

class Color
{
public:
	uint8_t R, G, B, A;
};

inline bool operator==(const Rotator& a, const Rotator& b) { return a.Pitch == b.Pitch && a.Yaw == b.Yaw && a.Roll == b.Roll; }
inline bool operator!=(const Rotator& a, const Rotator& b) { return a.Pitch != b.Pitch || a.Yaw != b.Yaw || a.Roll != b.Roll; }

class UnrealPropertyValue
{
public:
	union
	{
		uint8_t ValueByte;
		int32_t ValueInt;
		bool ValueBool;
		float ValueFloat;
		UObject* ValueObject;
		vec3 ValueVector;
		Rotator ValueRotator;
	};
	std::string ValueString;
};

class UnrealProperty
{
public:
	std::string Name;
	bool IsArray;
	int ArrayIndex;
	UnrealPropertyType Type;
	UnrealPropertyValue Scalar;
};

class UnrealProperties
{
public:
	bool HasScalar(const std::string& name) const
	{
		for (const UnrealProperty& prop : Properties)
		{
			if (prop.Name == name)
				return true;
		}
		return false;
	}

	const UnrealPropertyValue& GetScalar(const std::string& name)
	{
		for (const UnrealProperty& prop : Properties)
		{
			if (prop.Name == name)
				return prop.Scalar;
		}
		throw std::runtime_error("Property '" + name + "' not found");
	}

	UObject* GetUObject(const std::string& name)
	{
		return GetScalar(name).ValueObject;
	}

private:
	std::vector<UnrealProperty> Properties;
	friend class UObject;
};

class UObject
{
public:
	UObject(std::string name, UClass* base, ObjectFlags flags);
	virtual ~UObject() = default;

	virtual void Load(ObjectStream* stream);

	bool HasScalar(const std::string& name) const;
	const UnrealPropertyValue& GetScalar(const std::string& name);
	UObject* GetUObject(const std::string& name);

	std::string Name;
	UClass* Base = nullptr;
	ObjectFlags Flags = ObjectFlags::None;

	UnrealProperties Properties;

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

	void ReadProperties(ObjectStream* stream);
};
