
#include "Precomp.h"
#include "UTextBuffer.h"

UTextBuffer::UTextBuffer(ObjectStream* stream) : UObject(stream)
{
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
