#pragma once

#include "Package/ObjectStream.h"
#include "Math/vec.h"

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
	int Pitch, Yaw, Roll;
};

class UnrealPropertyValue
{
public:
	union
	{
		uint8_t ValueByte;
		int32_t ValueInt;
		bool ValueBool;
		float ValueFloat;
		int32_t ValueObject;
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

private:
	std::vector<UnrealProperty> Properties;
	friend class UObject;
};

class UObject
{
public:
	UObject() = default;
	UObject(ObjectStream* stream);
	virtual ~UObject() = default;

	std::string ClassName;
	UnrealProperties Properties;

	template<typename T>
	static T* Cast(UObject* obj)
	{
		T* target = dynamic_cast<T*>(obj);
		if (target == nullptr && obj != nullptr)
			throw std::runtime_error("Could not cast " + obj->ClassName + " to " + (std::string)typeid(T).name());
		return target;
	}

	template<typename T>
	static T* TryCast(UObject* obj)
	{
		return dynamic_cast<T*>(obj);
	}
};
