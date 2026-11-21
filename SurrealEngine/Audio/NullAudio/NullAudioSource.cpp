
#include "Precomp.h"
#include "NullAudioSource.h"

void NullAudioSource::Play()
{
	m_isPlaying = true;
}

void NullAudioSource::Stop()
{
	m_isPlaying = false;
}

bool NullAudioSource::isPlaying() const
{
	return m_isPlaying;
}
