#pragma once

#include "UObject/UObject.h"
#include "UObject/UProperty.h"
#include "Utils/AlignedAlloc.h"
#include "Math/quaternion.h"
#include "Math/coords.h"
#include <optional>

class UProperty;

class StructValue
{
public:
	StructValue() = default;

	StructValue(const StructValue& other)
	{
		Load(other.Struct, other.Ptr);
	}

	StructValue(StructValue&& other)
	{
		*this = std::move(other);
	}

	~StructValue()
	{
		Reset();
	}

	void Store(void* dest) const
	{
		if (Struct)
		{
			for (UProperty* prop : Struct->Properties)
				prop->CopyArray(
					static_cast<uint8_t*>(dest) + prop->DataOffset.DataOffset,
					static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset);
		}
	}

	void Load(UStruct* type, void* src)
	{
		Reset();
		Struct = type;
		if (Struct)
		{
			Ptr = AlignedAlloc(Struct->StructAlignment, Struct->StructSize);
			for (UProperty* prop : Struct->Properties)
				prop->CopyConstructArray(
					static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset,
					static_cast<uint8_t*>(src) + prop->DataOffset.DataOffset);
		}
	}

	void Init(UStruct* type)
	{
		Reset();
		Struct = type;
		if (Struct)
		{
			Ptr = AlignedAlloc(Struct->StructAlignment, Struct->StructSize);
			for (UProperty* prop : Struct->Properties)
				prop->ConstructArray(static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset);
		}
	}

	void Reset()
	{
		if (Struct)
		{
			for (UProperty* prop : Struct->Properties)
				prop->DestructArray(static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset);
			AlignedFree(Ptr);
			Struct = nullptr;
		}
	}

	StructValue& operator=(const StructValue& other)
	{
		if (this != &other)
		{
			Load(other.Struct, other.Ptr);
		}
		return *this;
	}

	StructValue& operator=(StructValue&& other)
	{
		if (this != &other)
		{
			Struct = other.Struct;
			Ptr = other.Ptr;
			other.Struct = nullptr;
			other.Ptr = nullptr;
		}
		return *this;
	}

	UStruct* Struct = nullptr;
	void* Ptr = nullptr;
};
