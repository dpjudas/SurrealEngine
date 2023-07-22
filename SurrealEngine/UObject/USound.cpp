
#include "Precomp.h"
#include "USound.h"
#include "Engine.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioDevice.h"
#include "Audio/AudioSubsystem.h"

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

void USound::GetSound()
{
	if (samples.size() > 0)
		return;

	std::unique_ptr<AudioSource> source = AudioSource::CreateWav(Data);

	#define ALIGN(x, a) ((x & ~(a-1)) + a)
	samples.resize(ALIGN(source->GetSamples(), 4));
	samples.resize(ALIGN(source->ReadSamples(samples.data(), samples.size()), 4));

	frequency = source->GetFrequency();
	duration = samples.size() / (float)frequency;
	channels = source->GetChannels();

	engine->audio->GetDevice()->AddSound(this);
}

float USound::GetDuration()
{
	if (duration == 0.0f)
		GetSound();

	return duration;
}

int USound::GetChannels()
{
	if (channels == 0)
		GetSound();

	return channels;
}