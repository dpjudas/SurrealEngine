
#include "Precomp.h"
#include "UState.h"
#include "Packages/Core/UFunction.h"

void UState::Load(ObjectStream* stream)
{
	UStruct::Load(stream);
	ProbeMask = stream->ReadUInt64();
	IgnoreMask = stream->ReadUInt64();
	LabelTableOffset = stream->ReadUInt16();
	StateFlags = (ScriptStateFlags)stream->ReadUInt32();

	for (UField* child = Children; child; child = child->Next)
	{
		if (auto func = UObject::TryCast<UFunction>(child))
		{
			Functions[child->Name] = func;
		}
	}
}

void UState::Save(PackageStreamWriter* stream)
{
	UStruct::Save(stream);
	stream->WriteUInt64(ProbeMask);
	stream->WriteUInt64(IgnoreMask);
	stream->WriteUInt16(LabelTableOffset);
	stream->WriteUInt32((uint32_t)StateFlags);
}
