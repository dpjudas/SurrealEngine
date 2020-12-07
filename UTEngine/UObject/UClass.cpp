
#include "Precomp.h"
#include "UClass.h"

UField::UField(ObjectStream* stream) : UObject(stream->GetObjectName(), stream->GetObjectBase())
{
	if (AllFlags(stream->GetFlags(), ObjectFlags::Native))
		return;

	Name = stream->GetObjectName();
	Base = stream->GetObjectBase();
	SuperField = stream->ReadIndex();
	Next = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UConst::UConst(ObjectStream* stream) : UField(stream)
{
	int size = stream->ReadIndex();
	std::vector<char> buffer(size + (size_t)1);
	stream->ReadBytes(buffer.data(), size);
	buffer.back() = 0;
	Constant = buffer.data();
}

/////////////////////////////////////////////////////////////////////////////

UEnum::UEnum(ObjectStream* stream) : UField(stream)
{
	int size = stream->ReadIndex();
	for (int i = 0; i < size; i++)
		ElementNames.push_back(stream->ReadString());
}

/////////////////////////////////////////////////////////////////////////////

UProperty::UProperty(ObjectStream* stream) : UField(stream)
{
	ArrayDimension = stream->ReadUInt16();
	ElementSize = stream->ReadUInt16();
	PropFlags = (PropertyFlags)stream->ReadUInt32();
	Category = stream->ReadName();
	if (AllFlags(PropFlags, PropertyFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
}

/////////////////////////////////////////////////////////////////////////////

UByteProperty::UByteProperty(ObjectStream* stream) : UProperty(stream)
{
	EnumType = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UObjectProperty::UObjectProperty(ObjectStream* stream) : UProperty(stream)
{
	ObjectType = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UFixedArrayProperty::UFixedArrayProperty(ObjectStream* stream) : UProperty(stream)
{
	ElementType = stream->ReadIndex();
	Count = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UArrayProperty::UArrayProperty(ObjectStream* stream) : UProperty(stream)
{
	ElementType = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UMapProperty::UMapProperty(ObjectStream* stream) : UProperty(stream)
{
	Key = stream->ReadIndex();
	Value = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UClassProperty::UClassProperty(ObjectStream* stream) : UProperty(stream)
{
	Type = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UStructProperty::UStructProperty(ObjectStream* stream) : UProperty(stream)
{
	Type = stream->ReadIndex();
}

/////////////////////////////////////////////////////////////////////////////

UStruct::UStruct(ObjectStream* stream) : UField(stream)
{
	if (AllFlags(stream->GetFlags(), ObjectFlags::Native))
		return;

	int ScriptText = stream->ReadIndex();
	int Children = stream->ReadIndex();
	std::string FriendlyName = stream->ReadName();
	uint32_t Line = stream->ReadUInt32();
	uint32_t TextPos = stream->ReadUInt32();
	uint32_t BytecodeSize = stream->ReadUInt32();
	// To do: read the script bytecode 
}

/////////////////////////////////////////////////////////////////////////////

UFunction::UFunction(ObjectStream* stream) : UStruct(stream)
{
	ParmsSize = stream->ReadIndex();
	NativeFuncIndex = stream->ReadUInt16();
	NumParms = stream->ReadIndex();
	OperatorPrecedence = stream->ReadUInt8();
	ReturnValueOffset = stream->ReadIndex();
	FuncFlags = (FunctionFlags)stream->ReadUInt32();
	if (AllFlags(FuncFlags, FunctionFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
}

/////////////////////////////////////////////////////////////////////////////

UState::UState(ObjectStream* stream) : UStruct(stream)
{
	if (AllFlags(stream->GetFlags(), ObjectFlags::Native))
		return;

	ProbeMask = stream->ReadUInt64();
	IgnoreMask = stream->ReadUInt64();
	LabelTableOffset = stream->ReadUInt16();
	StateFlags = (ScriptStateFlags)stream->ReadUInt32();
}

/////////////////////////////////////////////////////////////////////////////

UClass::UClass(ObjectStream* stream) : UState(stream)
{
	if (AllFlags(stream->GetFlags(), ObjectFlags::Native))
		return;

	if (stream->GetVersion() <= 61)
		OldClassRecordSize = stream->ReadUInt32();

	ClassFlags = stream->ReadUInt32();
	stream->ReadBytes(ClassGuid.Data, 16);

	int NumDependencies = stream->ReadIndex();
	for (int i = 0; i < NumDependencies; i++)
	{
		ClassDependency dep;
		dep.Class = stream->ReadIndex();
		dep.Deep = stream->ReadUInt32();
		dep.ScriptTextCRC = stream->ReadUInt32();
		Dependencies.push_back(dep);
	}

	int NumPackageImports = stream->ReadIndex();
	for (int i = 0; i < NumPackageImports; i++)
		PackageImports.push_back(stream->ReadIndex());

	if (stream->GetVersion() >= 62)
	{
		ClassWithin = stream->ReadIndex();
		ClassConfigName = stream->ReadName();
	}

	ReadProperties(stream);
}
