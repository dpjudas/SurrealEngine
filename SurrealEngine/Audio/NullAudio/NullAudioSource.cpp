
#include "Precomp.h"
#include "NullAudioSource.h"
#include "Packages/Engine/Resources/USound.h"

void NullAudioSource::Play(NullSound* newSound, uint64_t currentTime)
{
	sound = newSound;
	startTime = currentTime;
}

void NullAudioSource::Stop()
{
	sound = nullptr;
}

bool NullAudioSource::IsPlaying(uint64_t currentTime) const
{
	if (sound)
		return (sound->looping || currentTime - startTime < sound->length);
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////

NullSound::NullSound(USound* sound)
{
	length = sound->samples.size() * 1000 / sound->channels / sound->frequency;
	looping = sound->loopInfo.Looped;
}
