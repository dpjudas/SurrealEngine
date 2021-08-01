
#include "Precomp.h"
#include "USound.h"
#include "Engine.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioMixer.h"

void USound::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	Format = stream->ReadName();
	if (stream->GetVersion() >= 63)
		stream->ReadUInt32(); // lazy array skip offset
	uint32_t size = stream->ReadIndex();
	Data.resize(size);
	stream->ReadBytes(Data.data(), size);
}

AudioSound* USound::GetSound()
{
	if (Sound)
		return Sound;

	Sound = engine->audio->AddSound(AudioSource::CreateWav(Data));
	return Sound;
}
