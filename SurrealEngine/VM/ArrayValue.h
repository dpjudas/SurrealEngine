#pragma once

#include "UObject/UObject.h"
#include "UObject/UProperty.h"
#include "Utils/AlignedAlloc.h"
#include "Math/quaternion.h"
#include "Math/coords.h"
#include <optional>

class UArrayProperty;

class ArrayValue
{
public:
	ArrayValue() = default;

	ArrayValue(const ArrayValue& other)
	{
		Load(other.Type, other.Ptr);
	}

	ArrayValue(ArrayValue&& other)
	{
		*this = std::move(other);
	}

	~ArrayValue()
	{
		Reset();
	}

	void Store(void* dest) const
	{
		if (Type)
		{
			Type->CopyArray(dest, Ptr);
		}
	}

	void Load(UArrayProperty* type, void* src)
	{
		Reset();
		Type = type;
		if (Type)
		{
			Ptr = AlignedAlloc(Type->ArrayAlignment(), Type->ArraySize());
			Type->CopyConstructArray(Ptr, src);
		}
	}

	void Reset()
	{
		if (Type)
		{
			Type->DestructArray(Ptr);
			AlignedFree(Ptr);
			Type = nullptr;
		}
	}

	ArrayValue& operator=(const ArrayValue& other)
	{
		if (this != &other)
		{
			Load(other.Type, other.Ptr);
		}
		return *this;
	}

	ArrayValue& operator=(ArrayValue&& other)
	{
		if (this != &other)
		{
			Type = other.Type;
			Ptr = other.Ptr;
			other.Type = nullptr;
			other.Ptr = nullptr;
		}
		return *this;
	}

	UArrayProperty* Type = nullptr;
	void* Ptr = nullptr;
};
