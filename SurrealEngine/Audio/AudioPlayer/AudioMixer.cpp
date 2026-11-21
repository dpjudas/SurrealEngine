
#include "Precomp.h"
#include "AudioMixerImpl.h"
#include "../AudioSource.h"
#include "AudioPlayer.h"
#include <mutex>
#include <stdexcept>
#include <map>
#include <cmath>

std::unique_ptr<AudioMixer> AudioMixer::Create()
{
	return std::make_unique<AudioMixerImpl>();
}
