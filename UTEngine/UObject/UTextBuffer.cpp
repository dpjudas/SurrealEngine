
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
		if (stream->ReadUInt8() != 0)
			throw std::runtime_error("UTextBuffer is not null terminated");
	}
}
