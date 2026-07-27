#pragma once

#include "UReplicationInfo.h"

class UDecoration;
class Ulocationid;
class UTexture;

class UPlayerReplicationInfo : public UReplicationInfo
{
public:
	using UReplicationInfo::UReplicationInfo;

	float& Deaths() { return Value<float>(PropOffsets_PlayerReplicationInfo.Deaths); }
	UDecoration*& HasFlag() { return Value<UDecoration*>(PropOffsets_PlayerReplicationInfo.HasFlag); }
	std::string& OldName() { return Value<std::string>(PropOffsets_PlayerReplicationInfo.OldName); }
	uint8_t& PacketLoss() { return Value<uint8_t>(PropOffsets_PlayerReplicationInfo.PacketLoss); }
	int& Ping() { return Value<int>(PropOffsets_PlayerReplicationInfo.Ping); }
	int& PlayerID() { return Value<int>(PropOffsets_PlayerReplicationInfo.PlayerID); }
	Ulocationid*& PlayerLocation() { return Value<Ulocationid*>(PropOffsets_PlayerReplicationInfo.PlayerLocation); }
	std::string& PlayerName() { return Value<std::string>(PropOffsets_PlayerReplicationInfo.PlayerName); }
	UZoneInfo*& PlayerZone() { return Value<UZoneInfo*>(PropOffsets_PlayerReplicationInfo.PlayerZone); }
	float& Score() { return Value<float>(PropOffsets_PlayerReplicationInfo.Score); }
	int& StartTime() { return Value<int>(PropOffsets_PlayerReplicationInfo.StartTime); }
	UTexture*& TalkTexture() { return Value<UTexture*>(PropOffsets_PlayerReplicationInfo.TalkTexture); }
	uint8_t& Team() { return Value<uint8_t>(PropOffsets_PlayerReplicationInfo.Team); }
	int& TeamID() { return Value<int>(PropOffsets_PlayerReplicationInfo.TeamID); }
	std::string& TeamName() { return Value<std::string>(PropOffsets_PlayerReplicationInfo.TeamName); }
	int& TimeAcc() { return Value<int>(PropOffsets_PlayerReplicationInfo.TimeAcc); }
	UClass*& VoiceType() { return Value<UClass*>(PropOffsets_PlayerReplicationInfo.VoiceType); }
	BitfieldBool bAdmin() { return BoolValue(PropOffsets_PlayerReplicationInfo.bAdmin); }
	BitfieldBool bFeigningDeath() { return BoolValue(PropOffsets_PlayerReplicationInfo.bFeigningDeath); }
	BitfieldBool bIsABot() { return BoolValue(PropOffsets_PlayerReplicationInfo.bIsABot); }
	BitfieldBool bIsFemale() { return BoolValue(PropOffsets_PlayerReplicationInfo.bIsFemale); }
	BitfieldBool bIsSpectator() { return BoolValue(PropOffsets_PlayerReplicationInfo.bIsSpectator); }
	BitfieldBool bWaitingPlayer() { return BoolValue(PropOffsets_PlayerReplicationInfo.bWaitingPlayer); }
};
