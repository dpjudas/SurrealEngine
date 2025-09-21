
#include "Precomp.h"
#include "UTextBuffer.h"

void UTextBuffer::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	Pos = stream->ReadUInt32();
	Top = stream->ReadUInt32();

	int size = stream->ReadIndex();
	if (size > 0)
	{
		Text.resize(size);
		stream->ReadBytes(&Text[0], size);
		if (!Text.empty() && Text.back() == 0) Text.pop_back();
	}
}

void UTextBuffer::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	Exception::Throw("UTextBuffer::Save not implemented");
}
