
#include "Precomp.h"
#include "UFunction.h"

void UFunction::Load(ObjectStream* stream)
{
	UStruct::Load(stream);
	if (stream->GetVersion() <= 63)
		ParmsSize = stream->ReadUInt16();
	NativeFuncIndex = stream->ReadUInt16();
	if (stream->GetVersion() <= 63)
		NumParms = stream->ReadUInt8();
	OperatorPrecedence = stream->ReadUInt8();
	if (stream->GetVersion() <= 63)
		ReturnValueOffset = stream->ReadUInt16();
	FuncFlags = (FunctionFlags)stream->ReadUInt32();
	if (AllFlags(FuncFlags, FunctionFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
}

void UFunction::Save(PackageStreamWriter* stream)
{
	UStruct::Save(stream);
	if (stream->GetVersion() <= 63)
		stream->WriteUInt16(ParmsSize);
	stream->WriteUInt16(NativeFuncIndex);
	if (stream->GetVersion() <= 63)
		stream->WriteUInt8(NumParms);
	stream->WriteUInt8(OperatorPrecedence);
	if (stream->GetVersion() <= 63)
		stream->WriteUInt16(ReturnValueOffset);
	stream->WriteUInt32((uint32_t)FuncFlags);
	if (AllFlags(FuncFlags, FunctionFlags::Net))
		stream->WriteUInt16(ReplicationOffset);
}
