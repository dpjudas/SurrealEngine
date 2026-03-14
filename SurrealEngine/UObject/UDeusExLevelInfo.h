#pragma once

#include "UActor.h"


class UDeusExLevelInfo : public UInfo
{
public:
    using UInfo::UInfo;

    std::string& MapName() { return Value<std::string>(PropOffsets_DeusExLevelInfo.MapName); }
    std::string& MapAuthor() { return Value<std::string>(PropOffsets_DeusExLevelInfo.MapAuthor); }
    std::string& MissionLocation() { return Value<std::string>(PropOffsets_DeusExLevelInfo.MissionLocation); }
    int& MissionNumber() { return Value<int>(PropOffsets_DeusExLevelInfo.missionNumber); }
    BitfieldBool bMultiPlayerMap() { return BoolValue(PropOffsets_DeusExLevelInfo.bMultiPlayerMap); }
    // MissionScript* Script() { return Value<MissionScript>(PropOffsets_DeusExLevelInfo.Script); } // MissionScript isn't a native class
    int& TrueNorth() { return Value<int>(PropOffsets_DeusExLevelInfo.TrueNorth); }
    FixedArrayView<std::string, 4> StartupMessage() { return FixedArray<std::string, 4>(PropOffsets_DeusExLevelInfo.startupMessage); }
    std::string& ConversationPackage() { return Value<std::string>(PropOffsets_DeusExLevelInfo.ConversationPackage); }
};
