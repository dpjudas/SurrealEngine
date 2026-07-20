
#include "Precomp.h"
#include "NRMusicPlayer.h"

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

void NRMusicPlayer::RMusic_Startup(BitfieldBool& ReturnValue)
{
    // Since we'll use the built-in music support, we don't have to initialize anything here.
    ReturnValue = true;
}

void NRMusicPlayer::RMusic_LoadPlugin(std::string& Plugin, BitfieldBool& ReturnValue)
{
    // no-op
}

void NRMusicPlayer::RMusic_UnloadPlugin(std::string& Plugin)
{
    // no-op
}

void NRMusicPlayer::RMusic_SetDSPParam(std::string& Plugin, int index, float value)
{
    // maybe a no-op?
    LogUnimplemented("RMusic_Player.RMusic_SetDSPParam()");
}

void NRMusicPlayer::RMusic_Update()
{
    // Since we don't use fmod, this is a no-op
}

void NRMusicPlayer::RMusic_Play(std::string File, bool Loop, BitfieldBool& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_Play()");
    ReturnValue = false;
}

void NRMusicPlayer::RMusic_Pause(bool bPause)
{
    LogUnimplemented("RMusic_Player.RMusic_Pause()");
}

void NRMusicPlayer::RMusic_IsPlaying(BitfieldBool& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_IsPlaying()");
    ReturnValue = false;
}

void NRMusicPlayer::RMusic_SetCfgVolume()
{
    // Probably not a no-op
    LogUnimplemented("RMusic_Player.RMusic_SetCfgVolume()");
}

void NRMusicPlayer::RMusic_SetVolume(int NewVolume)
{
    LogUnimplemented("RMusic_Player.RMusic_SetVolume()");
}

void NRMusicPlayer::RMusic_IncVolume()
{
    LogUnimplemented("RMusic_Player.RMusic_IncVolume()");
}

void NRMusicPlayer::RMusic_DecVolume()
{
    LogUnimplemented("RMusic_Player.RMusic_DecVolume()");
}

void NRMusicPlayer::RMusic_GetVolume(int& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_GetVolume()");
    ReturnValue = 0;
}

void NRMusicPlayer::RMusic_Stop()
{
    LogUnimplemented("RMusic_Player.RMusic_Stop()");
}

void NRMusicPlayer::RMusic_Close()
{
    // no-op
}

void NRMusicPlayer::RMusic_GetTotalTime(int& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_GetTotalTime()");
    ReturnValue = -1;
}

void NRMusicPlayer::RMusic_GetCurrentTime(int& ReturnValue)
{
    LogUnimplemented("RMusic_Player.RMusic_GetCurrentTime()");
    ReturnValue = -1;
}
