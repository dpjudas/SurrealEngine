
#include "Precomp.h"
#include "UProperty.h"

UProperty::UProperty(ObjectStream* stream) : UField(stream)
{
	if (stream->IsEmptyStream()) return;

	ArrayDimension = stream->ReadUInt32();
	PropFlags = (PropertyFlags)stream->ReadUInt32();
	Category = stream->ReadName();
	if (AllFlags(PropFlags, PropertyFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
	if (stream->GetVersion() <= 61)
		PropFlags = (PropertyFlags)((uint32_t)PropFlags & ~0x00080040);
}

/////////////////////////////////////////////////////////////////////////////

UByteProperty::UByteProperty(ObjectStream* stream) : UProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	EnumType = stream->ReadObject<UEnum>();
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UObjectProperty::UObjectProperty(ObjectStream* stream) : UProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	ObjectClass = stream->ReadObject<UClass>();
}

/////////////////////////////////////////////////////////////////////////////

UFixedArrayProperty::UFixedArrayProperty(ObjectStream* stream) : UProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	Inner = stream->ReadObject<UProperty>();
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UArrayProperty::UArrayProperty(ObjectStream* stream) : UProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	Inner = stream->ReadObject<UProperty>();
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UMapProperty::UMapProperty(ObjectStream* stream) : UProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	Key = stream->ReadObject<UProperty>();
	Value = stream->ReadObject<UProperty>();
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UClassProperty::UClassProperty(ObjectStream* stream) : UObjectProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	MetaClass = stream->ReadObject<UClass>();
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UStructProperty::UStructProperty(ObjectStream* stream) : UProperty(stream)
{
	if (stream->IsEmptyStream()) return;

	Struct = stream->ReadObject<UStruct>();
	stream->ThrowIfNotEnd();
}
