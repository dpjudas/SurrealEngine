
#include "Precomp.h"
#include "URMusicPlayer.h"
#include "Engine.h"
#include "USubsystem.h"
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

    engine->audiodev->GetDevice()->PlayMusic(std::move(source));
    bIsPlaying = true;
    return true;
}

void URMusic_Player::RMusic_Stop()
{
    engine->audiodev->GetDevice()->PlayMusic({});
    bIsPlaying = false;
}

void URMusic_Player::RMusic_SetNewVolume(int NewVolume)
{
    // This should be sync'd with the overall music volume
    float fNewVolume = std::clamp(NewVolume / 255.f, 0.f, 1.f);
    engine->audiodev->GetDevice()->SetMusicVolume(fNewVolume);
    RMusic_Volume() = static_cast<int>(fNewVolume * 255); // Just in case fNewVolume gets clamped
}
