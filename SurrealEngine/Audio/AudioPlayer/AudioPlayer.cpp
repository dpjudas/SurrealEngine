
#include "Precomp.h"
#include "AudioPlayer.h"
#include "../AudioSource.h"

#ifdef WIN32

#include "WindowsAudioPlayer.h"

std::unique_ptr<AudioPlayer> AudioPlayer::Create(std::unique_ptr<AudioSource> source)
{
	return std::make_unique<WindowsAudioPlayer>(std::move(source));
}

#else

#include "AlsaAudioPlayer.h"

std::unique_ptr<AudioPlayer> AudioPlayer::Create(std::unique_ptr<AudioSource> source)
{
	return std::make_unique<AlsaAudioPlayer>(std::move(source));
}

#endif
