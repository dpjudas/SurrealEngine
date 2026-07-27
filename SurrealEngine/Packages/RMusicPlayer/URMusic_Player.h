#pragma once

#include "URMusic_Component.h"

class UPlayerPawn;
class ULevelInfo;

class URMusic_Player : public URMusic_Component
{
public:
    using URMusic_Component::URMusic_Component;

    int& RMusic_Volume() { return Value<int>(PropOffsets_RMusic_Player.RMusic_Volume); }
    BitfieldBool bAlwaysLoadCodecs() { return BoolValue(PropOffsets_RMusic_Player.bAlwaysLoadCodecs); }
    std::string& RMusic_PluginsDirectory() { return Value<std::string>(PropOffsets_RMusic_Player.RMusic_PluginsDirectory); }
    std::string& RMusic_Directory() { return Value<std::string>(PropOffsets_RMusic_Player.RMusic_Directory); }
    BitfieldBool bIncludeDebugInfo() { return BoolValue(PropOffsets_RMusic_Player.bIncludeDebugInfo); }
    BitfieldBool bAuthoritative() { return BoolValue(PropOffsets_RMusic_Player.bAuthoritative); }
    BitfieldBool bUseCurrentPaths() { return BoolValue(PropOffsets_RMusic_Player.bUseCurrentPaths); }
    float& FaderUpdateTime() { return Value<float>(PropOffsets_RMusic_Player.FaderUpdateTime); }
    std::string& NextTrack() { return Value<std::string>(PropOffsets_RMusic_Player.NextTrack); }
    BitfieldBool NextLoop() { return BoolValue(PropOffsets_RMusic_Player.NextLoop); }
    BitfieldBool bIsOn() { return BoolValue(PropOffsets_RMusic_Player.bIsOn); }
    UPlayerPawn*& RMusic_LocalPlayer() { return Value<UPlayerPawn*>(PropOffsets_RMusic_Player.RMusic_LocalPlayer); }
    ULevelInfo*& RMusic_CurLevel() { return Value<ULevelInfo*>(PropOffsets_RMusic_Player.RMusic_CurLevel); }
    ULevelInfo*& RMusic_OldLevel() { return Value<ULevelInfo*>(PropOffsets_RMusic_Player.RMusic_OldLevel); }
    BitfieldBool bHasDSP() { return BoolValue(PropOffsets_RMusic_Player.bHasDSP); }
    float& fDSPUpdateTime() { return Value<float>(PropOffsets_RMusic_Player.fDSPUpdateTime); }
    float& fDSPUpdateDelay() { return Value<float>(PropOffsets_RMusic_Player.fDSPUpdateDelay); }
    // nextEvent's RMusic_ControllerEx is not a native class

    bool RMusic_Play(const std::string& File, bool Loop);
    void RMusic_Stop();
    bool RMusic_IsPlaying() const { return bIsPlaying; }
    void RMusic_SetNewVolume(int NewVolume);

private:
    bool bIsPlaying = false;
};
