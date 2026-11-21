
#include "Precomp.h"
#include "AudioDevice.h"
#include "OpenAL/OpenALAudioDevice.h"
#include "AudioPlayer/AudioPlayerDevice.h"
#include "NullAudio/NullAudioDevice.h"
#include "LauncherSettings.h"

std::unique_ptr<AudioDevice> AudioDevice::Create(int frequency, int numVoices, int musicBufferCount, int musicBufferSize)
{
	const auto devName = LauncherSettings::Get().AudioDevice.Type;

	if (devName == AudioDeviceType::OpenAL)
		return std::make_unique<OpenALAudioDevice>(frequency, numVoices, musicBufferCount, musicBufferSize);

	return std::make_unique<NullAudioDevice>();
	//return std::make_unique<AudioPlayerDevice>();
}
