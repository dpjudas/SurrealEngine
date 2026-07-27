
#include "Precomp.h"
#include "UFixedArrayProperty.h"

void UFixedArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
	Count = stream->ReadInt32();
}

void UFixedArrayProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Inner);
	stream->WriteInt32(Count);
}

void UFixedArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_FixedArray);
	stream->Skip(header.size);
}

void UFixedArrayProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_FixedArray;
}

void UFixedArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	uint8_t* p = static_cast<uint8_t*>(data);
	for (int i = 0; i < Count; i++)
	{
		for (int arrayIndex = 0; arrayIndex < Inner->ArrayDimension; arrayIndex++)
		{
			Inner->SaveValue(p, stream);
			p += Inner->ElementPitch();
		}
	}
}

size_t UFixedArrayProperty::ElementAlignment()
{
	return Inner->ElementAlignment();
}

size_t UFixedArrayProperty::ElementSize()
{
	return Inner->ElementPitch() * Inner->ArrayDimension * Count;
}

void UFixedArrayProperty::ConstructElement(void* data)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->ConstructElement(static_cast<uint8_t*>(data) + i * pitch);
	}
}

void UFixedArrayProperty::CopyConstructElement(void* data, const void* src)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->CopyConstructElement(static_cast<uint8_t*>(data) + i * pitch, static_cast<const uint8_t*>(src) + i * pitch);
	}
}

void UFixedArrayProperty::DestructElement(void* data)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->DestructElement(static_cast<uint8_t*>(data) + i * pitch);
	}
}

void UFixedArrayProperty::CopyElement(void* data, const void* src)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		Inner->CopyElement(static_cast<uint8_t*>(data) + i * pitch, static_cast<const uint8_t*>(src) + i * pitch);
	}
}

bool UFixedArrayProperty::CompareElement(const void* v1, const void* v2)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		if (!Inner->CompareElement(static_cast<const uint8_t*>(v1) + i * pitch, static_cast<const uint8_t*>(v2) + i * pitch))
			return false;
	}
	return true;
}

bool UFixedArrayProperty::CompareLessElement(const void* v1, const void* v2)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		if (Inner->CompareLessElement(static_cast<const uint8_t*>(v1) + i * pitch, static_cast<const uint8_t*>(v2) + i * pitch))
			return true;
	}
	return false;
}

std::string UFixedArrayProperty::PrintValue(const void* data)
{
	return "fixed array";
}

GCAllocation* UFixedArrayProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	size_t pitch = Inner->ElementPitch();
	for (int i = 0; i < Count; i++)
	{
		marklist = Inner->MarkPropertyElement(marklist, static_cast<uint8_t*>(data) + i * pitch);
	}
	return marklist;
}
