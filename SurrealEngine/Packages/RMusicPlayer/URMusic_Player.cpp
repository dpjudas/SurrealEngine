
#include "Precomp.h"
#include "URMusic_Player.h"
#include "Engine.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Audio/AudioSource.h"
#include "Package/PackageManager.h"

bool URMusic_Player::RMusic_Play(const std::string& File, bool Loop)
{
    const auto FilePathStr = (engine->packages->GetSystemFolderPath() / RMusic_Directory() / File).lexically_normal().string();
    auto source = AudioSource::TryCreateFromFile(FilePathStr, Loop);

    if (!source)
    {
        bIsPlaying = false;
        return false;
    }

    engine->audiodev->PlayMusic(std::move(source));
    bIsPlaying = true;
    return true;
}

void URMusic_Player::RMusic_Stop()
{
    engine->audiodev->StopMusic();
    bIsPlaying = false;
}

void URMusic_Player::RMusic_SetNewVolume(int NewVolume)
{
    NewVolume = std::max(std::min(NewVolume, 255), 0);
    engine->audiodev->SetInstantMusicVolume(NewVolume);
    RMusic_Volume() = NewVolume;
}
