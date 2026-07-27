#pragma once

#include "UInfo.h"

class UMutator;
class UGameReplicationInfo;
class UStatLog;

class UGameInfo : public UInfo
{
public:
	using UInfo::UInfo;

	std::string& AdminPassword() { return Value<std::string>(PropOffsets_GameInfo.AdminPassword); }
	float& AutoAim() { return Value<float>(PropOffsets_GameInfo.AutoAim); }
	UMutator*& BaseMutator() { return Value<UMutator*>(PropOffsets_GameInfo.BaseMutator); }
	std::string& BeaconName() { return Value<std::string>(PropOffsets_GameInfo.BeaconName); }
	std::string& BotMenuType() { return Value<std::string>(PropOffsets_GameInfo.BotMenuType); }
	int& CurrentID() { return Value<int>(PropOffsets_GameInfo.CurrentID); }
	UClass*& DMMessageClass() { return Value<UClass*>(PropOffsets_GameInfo.DMMessageClass); }
	UMutator*& DamageMutator() { return Value<UMutator*>(PropOffsets_GameInfo.DamageMutator); }
	UClass*& DeathMessageClass() { return Value<UClass*>(PropOffsets_GameInfo.DeathMessageClass); }
	UClass*& DefaultPlayerClass() { return Value<UClass*>(PropOffsets_GameInfo.DefaultPlayerClass); }
	std::string& DefaultPlayerName() { return Value<std::string>(PropOffsets_GameInfo.DefaultPlayerName); }
	NameString& DefaultPlayerState() { return Value<NameString>(PropOffsets_GameInfo.DefaultPlayerState); }
	UClass*& DefaultWeapon() { return Value<UClass*>(PropOffsets_GameInfo.DefaultWeapon); }
	int& DemoBuild() { return Value<int>(PropOffsets_GameInfo.DemoBuild); }
	int& DemoHasTuts() { return Value<int>(PropOffsets_GameInfo.DemoHasTuts); }
	uint8_t& Difficulty() { return Value<uint8_t>(PropOffsets_GameInfo.Difficulty); }
	std::string& EnabledMutators() { return Value<std::string>(PropOffsets_GameInfo.EnabledMutators); }
	std::string& EnteredMessage() { return Value<std::string>(PropOffsets_GameInfo.EnteredMessage); }
	std::string& FailedPlaceMessage() { return Value<std::string>(PropOffsets_GameInfo.FailedPlaceMessage); }
	std::string& FailedSpawnMessage() { return Value<std::string>(PropOffsets_GameInfo.FailedSpawnMessage); }
	std::string& FailedTeamMessage() { return Value<std::string>(PropOffsets_GameInfo.FailedTeamMessage); }
	UClass*& GameMenuType() { return Value<UClass*>(PropOffsets_GameInfo.GameMenuType); }
	std::string& GameName() { return Value<std::string>(PropOffsets_GameInfo.GameName); }
	std::string& GameOptionsMenuType() { return Value<std::string>(PropOffsets_GameInfo.GameOptionsMenuType); }
	std::string& GamePassword() { return Value<std::string>(PropOffsets_GameInfo.GamePassword); }
	UGameReplicationInfo*& GameReplicationInfo() { return Value<UGameReplicationInfo*>(PropOffsets_GameInfo.GameReplicationInfo); }
	UClass*& GameReplicationInfoClass() { return Value<UClass*>(PropOffsets_GameInfo.GameReplicationInfoClass); }
	float& GameSpeed() { return Value<float>(PropOffsets_GameInfo.GameSpeed); }
	std::string& GameUMenuType() { return Value<std::string>(PropOffsets_GameInfo.GameUMenuType); }
	UClass*& HUDType() { return Value<UClass*>(PropOffsets_GameInfo.HUDType); }
	std::string& IPBanned() { return Value<std::string>(PropOffsets_GameInfo.IPBanned); }
	std::string& IPPolicies() { return Value<std::string>(PropOffsets_GameInfo.IPPolicies); }
	int& ItemGoals() { return Value<int>(PropOffsets_GameInfo.ItemGoals); }
	int& KillGoals() { return Value<int>(PropOffsets_GameInfo.KillGoals); }
	std::string& LeftMessage() { return Value<std::string>(PropOffsets_GameInfo.LeftMessage); }
	UStatLog*& LocalLog() { return Value<UStatLog*>(PropOffsets_GameInfo.LocalLog); }
	std::string& LocalLogFileName() { return Value<std::string>(PropOffsets_GameInfo.LocalLogFileName); }
	UClass*& MapListType() { return Value<UClass*>(PropOffsets_GameInfo.MapListType); }
	std::string& MapPrefix() { return Value<std::string>(PropOffsets_GameInfo.MapPrefix); }
	int& MaxPlayers() { return Value<int>(PropOffsets_GameInfo.MaxPlayers); }
	int& MaxSpectators() { return Value<int>(PropOffsets_GameInfo.MaxSpectators); }
	std::string& MaxedOutMessage() { return Value<std::string>(PropOffsets_GameInfo.MaxedOutMessage); }
	UMutator*& MessageMutator() { return Value<UMutator*>(PropOffsets_GameInfo.MessageMutator); }
	std::string& MultiplayerUMenuType() { return Value<std::string>(PropOffsets_GameInfo.MultiplayerUMenuType); }
	UClass*& MutatorClass() { return Value<UClass*>(PropOffsets_GameInfo.MutatorClass); }
	std::string& NameChangedMessage() { return Value<std::string>(PropOffsets_GameInfo.NameChangedMessage); }
	std::string& NeedPassword() { return Value<std::string>(PropOffsets_GameInfo.NeedPassword); }
	int& NumPlayers() { return Value<int>(PropOffsets_GameInfo.NumPlayers); }
	int& NumSpectators() { return Value<int>(PropOffsets_GameInfo.NumSpectators); }
	std::string& RulesMenuType() { return Value<std::string>(PropOffsets_GameInfo.RulesMenuType); }
	UClass*& ScoreBoardType() { return Value<UClass*>(PropOffsets_GameInfo.ScoreBoardType); }
	int& SecretGoals() { return Value<int>(PropOffsets_GameInfo.SecretGoals); }
	int& SentText() { return Value<int>(PropOffsets_GameInfo.SentText); }
	std::string& ServerLogName() { return Value<std::string>(PropOffsets_GameInfo.ServerLogName); }
	std::string& SettingsMenuType() { return Value<std::string>(PropOffsets_GameInfo.SettingsMenuType); }
	std::string& SpecialDamageString() { return Value<std::string>(PropOffsets_GameInfo.SpecialDamageString); }
	float& StartTime() { return Value<float>(PropOffsets_GameInfo.StartTime); }
	UClass*& StatLogClass() { return Value<UClass*>(PropOffsets_GameInfo.StatLogClass); }
	std::string& SwitchLevelMessage() { return Value<std::string>(PropOffsets_GameInfo.SwitchLevelMessage); }
	UClass*& WaterZoneType() { return Value<UClass*>(PropOffsets_GameInfo.WaterZoneType); }
	UStatLog*& WorldLog() { return Value<UStatLog*>(PropOffsets_GameInfo.WorldLog); }
	std::string& WorldLogFileName() { return Value<std::string>(PropOffsets_GameInfo.WorldLogFileName); }
	std::string& WrongPassword() { return Value<std::string>(PropOffsets_GameInfo.WrongPassword); }
	BitfieldBool bAllowFOV() { return BoolValue(PropOffsets_GameInfo.bAllowFOV); }
	BitfieldBool bAlternateMode() { return BoolValue(PropOffsets_GameInfo.bAlternateMode); }
	BitfieldBool bBatchLocal() { return BoolValue(PropOffsets_GameInfo.bBatchLocal); }
	BitfieldBool bCanChangeSkin() { return BoolValue(PropOffsets_GameInfo.bCanChangeSkin); }
	BitfieldBool bCanViewOthers() { return BoolValue(PropOffsets_GameInfo.bCanViewOthers); }
	BitfieldBool bClassicDeathMessages() { return BoolValue(PropOffsets_GameInfo.bClassicDeathMessages); }
	BitfieldBool bCoopWeaponMode() { return BoolValue(PropOffsets_GameInfo.bCoopWeaponMode); }
	BitfieldBool bDeathMatch() { return BoolValue(PropOffsets_GameInfo.bDeathMatch); }
	BitfieldBool bExternalBatcher() { return BoolValue(PropOffsets_GameInfo.bExternalBatcher); }
	BitfieldBool bGameEnded() { return BoolValue(PropOffsets_GameInfo.bGameEnded); }
	BitfieldBool bHumansOnly() { return BoolValue(PropOffsets_GameInfo.bHumansOnly); }
	BitfieldBool bLocalLog() { return BoolValue(PropOffsets_GameInfo.bLocalLog); }
	BitfieldBool bLoggingGame() { return BoolValue(PropOffsets_GameInfo.bLoggingGame); }
	BitfieldBool bLowGore() { return BoolValue(PropOffsets_GameInfo.bLowGore); }
	BitfieldBool bMuteSpectators() { return BoolValue(PropOffsets_GameInfo.bMuteSpectators); }
	BitfieldBool bNoCheating() { return BoolValue(PropOffsets_GameInfo.bNoCheating); }
	BitfieldBool bNoMonsters() { return BoolValue(PropOffsets_GameInfo.bNoMonsters); }
	BitfieldBool bOverTime() { return BoolValue(PropOffsets_GameInfo.bOverTime); }
	BitfieldBool bPauseable() { return BoolValue(PropOffsets_GameInfo.bPauseable); }
	BitfieldBool bRestartLevel() { return BoolValue(PropOffsets_GameInfo.bRestartLevel); }
	BitfieldBool bTeamGame() { return BoolValue(PropOffsets_GameInfo.bTeamGame); }
	BitfieldBool bVeryLowGore() { return BoolValue(PropOffsets_GameInfo.bVeryLowGore); }
	BitfieldBool bWorldLog() { return BoolValue(PropOffsets_GameInfo.bWorldLog); }

	// 227 Additions
	std::string& LastPreloginIP() { return Value<std::string>(PropOffsets_GameInfo.LastPreloginIP); }
	std::string& LastLoginPlayerNames() { return Value<std::string>(PropOffsets_GameInfo.LastLoginPlayerNames); }
	std::string& LastPreloginIdentity() { return Value<std::string>(PropOffsets_GameInfo.LastPreloginIdentity); }
	std::string& LastPreloginIdent() { return Value<std::string>(PropOffsets_GameInfo.LastPreloginIdent); }
	std::string& MaleGender() { return Value<std::string>(PropOffsets_GameInfo.MaleGender); }
	std::string& FemaleGender() { return Value<std::string>(PropOffsets_GameInfo.FemaleGender); }
	// GameRules and AdminAccessManager are not native classes
	// GameRules& GameRules() { return Value<GameRules>(PropOffsets_GameInfo.GameRules); }
	// AdminAccessManager& AccessManager() { return Value<AdminAccessManager>(PropOffsets_GameInfo.AccessManager); }
	std::string& AccessManagerClass() { return Value<std::string>(PropOffsets_GameInfo.AccessManagerClass); }
	int& BleedingDamageMin() { return Value<int>(PropOffsets_GameInfo.BleedingDamageMin); }
	int& BleedingDamageMax() { return Value<int>(PropOffsets_GameInfo.BleedingDamageMax); }

	BitfieldBool bBleedingEnabled() { return BoolValue(PropOffsets_GameInfo.bBleedingEnabled); }
	BitfieldBool bBleedingDamageEnabled() { return BoolValue(PropOffsets_GameInfo.bBleedingDamageEnabled); }
	BitfieldBool bAllHealthStopsBleeding() { return BoolValue(PropOffsets_GameInfo.bAllHealthStopsBleeding); }
	BitfieldBool bBandagesStopBleeding() { return BoolValue(PropOffsets_GameInfo.bBandagesStopBleeding); }
	BitfieldBool bMessageAdminsAliases() { return BoolValue(PropOffsets_GameInfo.bMessageAdminsAliases); }
	BitfieldBool bLogNewPlayerAliases() { return BoolValue(PropOffsets_GameInfo.bLogNewPlayerAliases); }
	BitfieldBool bLogDownloadsToClient() { return BoolValue(PropOffsets_GameInfo.bLogDownloadsToClient); }
	BitfieldBool bHandleDownloadMessaging() { return BoolValue(PropOffsets_GameInfo.bHandleDownloadMessaging); }
	BitfieldBool bShowRecoilAnimations() { return BoolValue(PropOffsets_GameInfo.bShowRecoilAnimations); }
	BitfieldBool bCastShadow() { return BoolValue(PropOffsets_GameInfo.bCastShadow); }
	BitfieldBool bDecoShadows() { return BoolValue(PropOffsets_GameInfo.bDecoShadows); }
	BitfieldBool bCastProjectorShadows() { return BoolValue(PropOffsets_GameInfo.bCastProjectorShadows); }
	BitfieldBool bUseRealtimeShadow() { return BoolValue(PropOffsets_GameInfo.bUseRealtimeShadow); }
	BitfieldBool bNoWalkInAir() { return BoolValue(PropOffsets_GameInfo.bNoWalkInAir); }
	BitfieldBool bProjectorDecals() { return BoolValue(PropOffsets_GameInfo.bProjectorDecals); }
	BitfieldBool bIsSavedGame() { return BoolValue(PropOffsets_GameInfo.bIsSavedGame); }
	BitfieldBool bAlwaysEnhancedSightCheck() { return BoolValue(PropOffsets_GameInfo.bAlwaysEnhancedSightCheck); }
};
