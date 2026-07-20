#pragma once

#include "UObject/UObject.h"

/* RMusic_Player compatibility stuff
 * Since RMusic_Player uses fmod and we have our solutions for playing most of the files this class plays,
 * most of these functions will basically be no-ops
 */

class NRMusicPlayer
{
public:
    static void RegisterFunctions();

    static void RMusic_Startup(BitfieldBool& ReturnValue);
    static void RMusic_LoadPlugin(std::string& Plugin, BitfieldBool& ReturnValue);
    static void RMusic_UnloadPlugin(std::string& Plugin);
    static void RMusic_SetDSPParam(std::string& Plugin, int index, float value);
    static void RMusic_Update();
    static void RMusic_Play(std::string File, bool Loop, BitfieldBool& ReturnValue);
    static void RMusic_Pause(bool bPause);
    static void RMusic_IsPlaying(BitfieldBool& ReturnValue);
    static void RMusic_SetCfgVolume();
    static void RMusic_SetVolume(int NewVolume);
    static void RMusic_IncVolume();
    static void RMusic_DecVolume();
    static void RMusic_GetVolume(int& ReturnValue);
    static void RMusic_Stop();
    static void RMusic_Close();
    static void RMusic_GetTotalTime(int& ReturnValue);
    static void RMusic_GetCurrentTime(int& ReturnValue);
};