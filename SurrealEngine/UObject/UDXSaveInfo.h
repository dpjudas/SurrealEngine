#pragma once

#include "UObject.h"
#include "UTexture.h"

class UDXSaveInfo : public UObject
{
public:
    using UObject::UObject;

    int& Year() { return Value<int>(PropOffsets_DeusExSaveInfo.Year); }
    int& Month() { return Value<int>(PropOffsets_DeusExSaveInfo.Month); }
    int& Day() { return Value<int>(PropOffsets_DeusExSaveInfo.Day); }
    int& Hour() { return Value<int>(PropOffsets_DeusExSaveInfo.Hour); }
    int& Minute() { return Value<int>(PropOffsets_DeusExSaveInfo.Minute); }
    int& Second() { return Value<int>(PropOffsets_DeusExSaveInfo.Second); }

    int& DirectoryIndex() { return Value<int>(PropOffsets_DeusExSaveInfo.DirectoryIndex); }
    std::string& Description() { return Value<std::string>(PropOffsets_DeusExSaveInfo.Description); }
    std::string& MissionLocation() { return Value<std::string>(PropOffsets_DeusExSaveInfo.MissionLocation); }
    std::string& MapName() { return Value<std::string>(PropOffsets_DeusExSaveInfo.MapName); }
    UTexture*& Snapshot() { return Value<UTexture*>(PropOffsets_DeusExSaveInfo.Snapshot); }
    int& SaveCount() { return Value<int>(PropOffsets_DeusExSaveInfo.saveCount); }
    int& SaveTime() { return Value<int>(PropOffsets_DeusExSaveInfo.saveTime); }
    BitfieldBool bCheatsEnabled() { return BoolValue(PropOffsets_DeusExSaveInfo.bCheatsEnabled); }

    void UpdateTimeStamp();
};