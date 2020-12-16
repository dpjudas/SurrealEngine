
#include "Precomp.h"
#include "UProperty.h"

void UProperty::Load(ObjectStream* stream)
{
	UField::Load(stream);

	ArrayDimension = stream->ReadUInt32();
	PropFlags = (PropertyFlags)stream->ReadUInt32();
	Category = stream->ReadName();
	if (AllFlags(PropFlags, PropertyFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
	if (stream->GetVersion() <= 61)
		PropFlags = (PropertyFlags)((uint32_t)PropFlags & ~0x00080040);
}

/////////////////////////////////////////////////////////////////////////////

void UByteProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	EnumType = stream->ReadObject<UEnum>();
}

/////////////////////////////////////////////////////////////////////////////

void UObjectProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	ObjectClass = stream->ReadObject<UClass>();
}

/////////////////////////////////////////////////////////////////////////////

void UFixedArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
}

/////////////////////////////////////////////////////////////////////////////

void UArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
}

/////////////////////////////////////////////////////////////////////////////

void UMapProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Key = stream->ReadObject<UProperty>();
	Value = stream->ReadObject<UProperty>();
}

/////////////////////////////////////////////////////////////////////////////

void UClassProperty::Load(ObjectStream* stream)
{
	UObjectProperty::Load(stream);
	MetaClass = stream->ReadObject<UClass>();
}

/////////////////////////////////////////////////////////////////////////////

void UStructProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Struct = stream->ReadObject<UStruct>();
}
