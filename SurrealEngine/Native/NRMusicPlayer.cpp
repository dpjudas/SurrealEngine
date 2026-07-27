
#include "Precomp.h"
#include "NRMusicPlayer.h"
#include "Packages/RMusicPlayer/URMusic_Player.h"
#include "Utils/Logger.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"

void NRMusicPlayer::RegisterFunctions()
{
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_Startup", &NRMusicPlayer::RMusic_Startup, 0);
    RegisterVMNativeFunc_2("RMusic_Player", "RMusic_LoadPlugin", &NRMusicPlayer::RMusic_LoadPlugin, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_UnloadPlugin", &NRMusicPlayer::RMusic_UnloadPlugin, 0);
    RegisterVMNativeFunc_3("RMusic_Player", "RMusic_SetDSPParam", &NRMusicPlayer::RMusic_SetDSPParam, 0);
    RegisterVMNativeFunc_0("RMusic_Player", "RMusic_Update", &NRMusicPlayer::RMusic_Update, 0);
    RegisterVMNativeFunc_3("RMusic_Player", "RMusic_Play", &NRMusicPlayer::RMusic_Play, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_Pause", &NRMusicPlayer::RMusic_Pause, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_IsPlaying", &NRMusicPlayer::RMusic_IsPlaying, 0);
    RegisterVMNativeFunc_0("RMusic_Player", "RMusic_SetCfgVolume", &NRMusicPlayer::RMusic_SetCfgVolume, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_SetVolume", &NRMusicPlayer::RMusic_SetVolume, 0);
    RegisterVMNativeFunc_0("RMusic_Player", "RMusic_IncVolume", &NRMusicPlayer::RMusic_IncVolume, 0);
    RegisterVMNativeFunc_0("RMusic_Player", "RMusic_DecVolume", &NRMusicPlayer::RMusic_DecVolume, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_GetVolume", &NRMusicPlayer::RMusic_GetVolume, 0);
    RegisterVMNativeFunc_0("RMusic_Player", "RMusic_Stop", &NRMusicPlayer::RMusic_Stop, 0);
    RegisterVMNativeFunc_0("RMusic_Player", "RMusic_Close", &NRMusicPlayer::RMusic_Close, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_GetTotalTime", &NRMusicPlayer::RMusic_GetTotalTime, 0);
    RegisterVMNativeFunc_1("RMusic_Player", "RMusic_GetCurrentTime", &NRMusicPlayer::RMusic_GetCurrentTime, 0);
}

void NRMusicPlayer::RMusic_Startup(UObject* Self, BitfieldBool& ReturnValue)
{
    // Since we'll use the built-in music support, we don't have to initialize anything here.
    ReturnValue = true;
}

void NRMusicPlayer::RMusic_LoadPlugin(UObject* Self, std::string& Plugin, BitfieldBool& ReturnValue)
{
    // no-op
}

void NRMusicPlayer::RMusic_UnloadPlugin(UObject* Self, std::string& Plugin)
{
    // no-op
}

void NRMusicPlayer::RMusic_SetDSPParam(UObject* Self, std::string& Plugin, int index, float value)
{
    // maybe a no-op?
    LogUnimplemented("RMusic_Player.RMusic_SetDSPParam()");
}

void NRMusicPlayer::RMusic_Update(UObject* Self)
{
    // Since we don't use fmod, this is a no-op
}

void NRMusicPlayer::RMusic_Play(UObject* Self, std::string& File, bool Loop, BitfieldBool& ReturnValue)
{
    const auto MusicPlayer = UObject::Cast<URMusic_Player>(Self);
    ReturnValue = MusicPlayer->RMusic_Play(File, Loop);
}

void NRMusicPlayer::RMusic_Pause(UObject* Self, bool bPause)
{
    LogUnimplemented("RMusic_Player.RMusic_Pause()");
}

void NRMusicPlayer::RMusic_IsPlaying(UObject* Self, BitfieldBool& ReturnValue)
{
    const auto MusicPlayer = UObject::Cast<URMusic_Player>(Self);
    ReturnValue = MusicPlayer->RMusic_IsPlaying();
}

void NRMusicPlayer::RMusic_SetCfgVolume(UObject* Self)
{
    // Probably not a no-op
    LogUnimplemented("RMusic_Player.RMusic_SetCfgVolume()");
}

void NRMusicPlayer::RMusic_SetVolume(UObject* Self, int NewVolume)
{
    const auto MusicPlayer = UObject::Cast<URMusic_Player>(Self);
    MusicPlayer->RMusic_SetNewVolume(NewVolume);
}

void NRMusicPlayer::RMusic_IncVolume(UObject* Self)
{
    LogUnimplemented("RMusic_Player.RMusic_IncVolume()");
}

void NRMusicPlayer::RMusic_DecVolume(UObject* Self)
{
    LogUnimplemented("RMusic_Player.RMusic_DecVolume()");
}

void NRMusicPlayer::RMusic_GetVolume(UObject* Self, int& ReturnValue)
{
    ReturnValue = UObject::Cast<URMusic_Player>(Self)->RMusic_Volume();
}

void NRMusicPlayer::RMusic_Stop(UObject* Self)
{
    UObject::Cast<URMusic_Player>(Self)->RMusic_Stop();
}

void NRMusicPlayer::RMusic_Close(UObject* Self)
{
    // no-op
}

void NRMusicPlayer::RMusic_GetTotalTime(UObject* Self, int& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_GetTotalTime()");
    ReturnValue = -1;
}

void NRMusicPlayer::RMusic_GetCurrentTime(UObject* Self, int& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_GetCurrentTime()");
    ReturnValue = -1;
}
