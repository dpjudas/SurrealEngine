
#include "Precomp.h"
#include "UMusic.h"

UMusic::UMusic(ObjectStream* stream) : UObject(stream)
{
	Format = stream->ReadName();
	if (stream->GetVersion() > 61)
		stream->ReadUInt32(); // lazy array skip offset
	uint32_t size = stream->ReadIndex();
	Data.resize(size);
	stream->ReadBytes(Data.data(), size);
}
