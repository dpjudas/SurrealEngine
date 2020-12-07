
#include "Precomp.h"
#include "USound.h"

USound::USound(ObjectStream* stream) : UObject(stream)
{
	Format = stream->ReadName();
	if (stream->GetVersion() >= 63)
		stream->ReadUInt32(); // lazy array skip offset
	uint32_t size = stream->ReadIndex();
	Data.resize(size);
	stream->ReadBytes(Data.data(), size);
}
