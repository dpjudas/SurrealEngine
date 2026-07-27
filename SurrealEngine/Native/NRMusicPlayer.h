#pragma once

#include "Packages/Core/UObject.h"

/* RMusic_Player compatibility stuff
 * Since RMusic_Player uses fmod and we have our solutions for playing most of the files this class plays,
 * most of these functions will basically be no-ops
 */

class NRMusicPlayer
{
public:
    static void RegisterFunctions();

    static void RMusic_Startup(UObject* Self, BitfieldBool& ReturnValue);
    static void RMusic_LoadPlugin(UObject* Self, std::string& Plugin, BitfieldBool& ReturnValue);
    static void RMusic_UnloadPlugin(UObject* Self, std::string& Plugin);
    static void RMusic_SetDSPParam(UObject* Self, std::string& Plugin, int index, float value);
    static void RMusic_Update(UObject* Self);
    static void RMusic_Play(UObject* Self, std::string& File, bool Loop, BitfieldBool& ReturnValue);
    static void RMusic_Pause(UObject* Self, bool bPause);
    static void RMusic_IsPlaying(UObject* Self, BitfieldBool& ReturnValue);
    static void RMusic_SetCfgVolume(UObject* Self);
    static void RMusic_SetVolume(UObject* Self, int NewVolume);
    static void RMusic_IncVolume(UObject* Self);
    static void RMusic_DecVolume(UObject* Self);
    static void RMusic_GetVolume(UObject* Self, int& ReturnValue);
    static void RMusic_Stop(UObject* Self);
    static void RMusic_Close(UObject* Self);
    static void RMusic_GetTotalTime(UObject* Self, int& ReturnValue);
    static void RMusic_GetCurrentTime(UObject* Self, int& ReturnValue);
};