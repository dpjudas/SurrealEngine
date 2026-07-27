#pragma once

#include "UReplicationInfo.h"

class UPlayerReplicationInfo;

class UGameReplicationInfo : public UReplicationInfo
{
public:
	using UReplicationInfo::UReplicationInfo;

	std::string& AdminEmail() { return Value<std::string>(PropOffsets_GameReplicationInfo.AdminEmail); }
	std::string& AdminName() { return Value<std::string>(PropOffsets_GameReplicationInfo.AdminName); }
	int& ElapsedTime() { return Value<int>(PropOffsets_GameReplicationInfo.ElapsedTime); }
	std::string& GameClass() { return Value<std::string>(PropOffsets_GameReplicationInfo.GameClass); }
	std::string& GameEndedComments() { return Value<std::string>(PropOffsets_GameReplicationInfo.GameEndedComments); }
	std::string& GameName() { return Value<std::string>(PropOffsets_GameReplicationInfo.GameName); }
	std::string& MOTDLine1() { return Value<std::string>(PropOffsets_GameReplicationInfo.MOTDLine1); }
	std::string& MOTDLine2() { return Value<std::string>(PropOffsets_GameReplicationInfo.MOTDLine2); }
	std::string& MOTDLine3() { return Value<std::string>(PropOffsets_GameReplicationInfo.MOTDLine3); }
	std::string& MOTDLine4() { return Value<std::string>(PropOffsets_GameReplicationInfo.MOTDLine4); }
	int& NumPlayers() { return Value<int>(PropOffsets_GameReplicationInfo.NumPlayers); }
	UPlayerReplicationInfo*& PRIArray() { return Value<UPlayerReplicationInfo*>(PropOffsets_GameReplicationInfo.PRIArray); }
	int& Region() { return Value<int>(PropOffsets_GameReplicationInfo.Region); }
	int& RemainingMinute() { return Value<int>(PropOffsets_GameReplicationInfo.RemainingMinute); }
	int& RemainingTime() { return Value<int>(PropOffsets_GameReplicationInfo.RemainingTime); }
	float& SecondCount() { return Value<float>(PropOffsets_GameReplicationInfo.SecondCount); }
	std::string& ServerName() { return Value<std::string>(PropOffsets_GameReplicationInfo.ServerName); }
	std::string& ShortName() { return Value<std::string>(PropOffsets_GameReplicationInfo.ShortName); }
	int& SumFrags() { return Value<int>(PropOffsets_GameReplicationInfo.SumFrags); }
	float& UpdateTimer() { return Value<float>(PropOffsets_GameReplicationInfo.UpdateTimer); }
	BitfieldBool bClassicDeathMessages() { return BoolValue(PropOffsets_GameReplicationInfo.bClassicDeathMessages); }
	BitfieldBool bStopCountDown() { return BoolValue(PropOffsets_GameReplicationInfo.bStopCountDown); }
	BitfieldBool bTeamGame() { return BoolValue(PropOffsets_GameReplicationInfo.bTeamGame); }
};
