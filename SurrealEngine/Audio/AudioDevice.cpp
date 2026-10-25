
#include "Precomp.h"
#include "AudioDevice.h"
#include "OpenAL/OpenALAudioDevice.h"

std::unique_ptr<AudioDevice> AudioDevice::Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize)
{
	return std::make_unique<OpenALAudioDevice>(frequency, numVoices, musicBufferCount, musicBufferSize);
}
