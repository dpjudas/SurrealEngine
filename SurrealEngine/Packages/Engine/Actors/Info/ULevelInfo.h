#pragma once

#include "UZoneInfo.h"
#include "UnrealURL.h"

class UGameInfo;
class UNavigationPoint;
class UPawn;
class UMusic;
class USpawnNotify;
class ULevelSummary;
class UDynamicZoneInfo;

class ULevelInfo : public UZoneInfo
{
public:
	using UZoneInfo::UZoneInfo;

	void UpdateActorZone() override;

	// Unreal 227 addition
	PointRegion GetLocZone(const vec3& pos, std::optional<UActor*> InActor);

	UnrealURL URL;

	FixedArrayView<int, 8> AIProfile() { return FixedArray<int, 8>(PropOffsets_LevelInfo.AIProfile); }
	std::string& Author() { return Value<std::string>(PropOffsets_LevelInfo.Author); }
	float& AvgAITime() { return Value<float>(PropOffsets_LevelInfo.AvgAITime); }
	float& Brightness() { return Value<float>(PropOffsets_LevelInfo.Brightness); }
	uint8_t& CdTrack() { return Value<uint8_t>(PropOffsets_LevelInfo.CdTrack); }
	std::string& ComputerName() { return Value<std::string>(PropOffsets_LevelInfo.ComputerName); }
	int& Day() { return Value<int>(PropOffsets_LevelInfo.Day); }
	int& DayOfWeek() { return Value<int>(PropOffsets_LevelInfo.DayOfWeek); }
	UClass*& DefaultGameType() { return Value<UClass*>(PropOffsets_LevelInfo.DefaultGameType); }
	UTexture*& DefaultTexture() { return Value<UTexture*>(PropOffsets_LevelInfo.DefaultTexture); }
	std::string& EngineVersion() { return Value<std::string>(PropOffsets_LevelInfo.EngineVersion); }
	UGameInfo*& Game() { return Value<UGameInfo*>(PropOffsets_LevelInfo.Game); }
	int& Hour() { return Value<int>(PropOffsets_LevelInfo.Hour); }
	int& HubStackLevel() { return Value<int>(PropOffsets_LevelInfo.HubStackLevel); }
	std::string& IdealPlayerCount() { return Value<std::string>(PropOffsets_LevelInfo.IdealPlayerCount); }
	uint8_t& LevelAction() { return Value<uint8_t>(PropOffsets_LevelInfo.LevelAction); }
	std::string& LevelEnterText() { return Value<std::string>(PropOffsets_LevelInfo.LevelEnterText); }
	std::string& LocalizedPkg() { return Value<std::string>(PropOffsets_LevelInfo.LocalizedPkg); }
	int& Millisecond() { return Value<int>(PropOffsets_LevelInfo.Millisecond); }
	std::string& MinNetVersion() { return Value<std::string>(PropOffsets_LevelInfo.MinNetVersion); }
	int& Minute() { return Value<int>(PropOffsets_LevelInfo.Minute); }
	int& Month() { return Value<int>(PropOffsets_LevelInfo.Month); }
	UNavigationPoint*& NavigationPointList() { return Value<UNavigationPoint*>(PropOffsets_LevelInfo.NavigationPointList); }
	uint8_t& NetMode() { return Value<uint8_t>(PropOffsets_LevelInfo.NetMode); }
	float& NextSwitchCountdown() { return Value<float>(PropOffsets_LevelInfo.NextSwitchCountdown); }
	std::string& NextURL() { return Value<std::string>(PropOffsets_LevelInfo.NextURL); }
	std::string& Pauser() { return Value<std::string>(PropOffsets_LevelInfo.Pauser); }
	UPawn*& PawnList() { return Value<UPawn*>(PropOffsets_LevelInfo.PawnList); }
	float& PlayerDoppler() { return Value<float>(PropOffsets_LevelInfo.PlayerDoppler); }
	int& RecommendedEnemies() { return Value<int>(PropOffsets_LevelInfo.RecommendedEnemies); }
	int& RecommendedTeammates() { return Value<int>(PropOffsets_LevelInfo.RecommendedTeammates); }
	UTexture*& Screenshot() { return Value<UTexture*>(PropOffsets_LevelInfo.Screenshot); }
	int& Second() { return Value<int>(PropOffsets_LevelInfo.Second); }
	UMusic*& Song() { return Value<UMusic*>(PropOffsets_LevelInfo.Song); }
	uint8_t& SongSection() { return Value<uint8_t>(PropOffsets_LevelInfo.SongSection); }
	USpawnNotify*& SpawnNotify() { return Value<USpawnNotify*>(PropOffsets_LevelInfo.SpawnNotify); }
	ULevelSummary*& Summary() { return Value<ULevelSummary*>(PropOffsets_LevelInfo.Summary); }
	float& TimeDilation() { return Value<float>(PropOffsets_LevelInfo.TimeDilation); }
	float& TimeSeconds() { return Value<float>(PropOffsets_LevelInfo.TimeSeconds); }
	std::string& Title() { return Value<std::string>(PropOffsets_LevelInfo.Title); }
	std::string& VisibleGroups() { return Value<std::string>(PropOffsets_LevelInfo.VisibleGroups); }
	int& Year() { return Value<int>(PropOffsets_LevelInfo.Year); }
	BitfieldBool bAggressiveLOD() { return BoolValue(PropOffsets_LevelInfo.bAggressiveLOD); }
	BitfieldBool bAllowFOV() { return BoolValue(PropOffsets_LevelInfo.bAllowFOV); }
	BitfieldBool bBegunPlay() { return BoolValue(PropOffsets_LevelInfo.bBegunPlay); }
	BitfieldBool bCheckWalkSurfaces() { return BoolValue(PropOffsets_LevelInfo.bCheckWalkSurfaces); }
	BitfieldBool bDropDetail() { return BoolValue(PropOffsets_LevelInfo.bDropDetail); }
	BitfieldBool bHighDetailMode() { return BoolValue(PropOffsets_LevelInfo.bHighDetailMode); }
	BitfieldBool bHumansOnly() { return BoolValue(PropOffsets_LevelInfo.bHumansOnly); }
	BitfieldBool bLonePlayer() { return BoolValue(PropOffsets_LevelInfo.bLonePlayer); }
	BitfieldBool bLowRes() { return BoolValue(PropOffsets_LevelInfo.bLowRes); }
	BitfieldBool bNeverPrecache() { return BoolValue(PropOffsets_LevelInfo.bNeverPrecache); }
	BitfieldBool bNextItems() { return BoolValue(PropOffsets_LevelInfo.bNextItems); }
	BitfieldBool bNoCheating() { return BoolValue(PropOffsets_LevelInfo.bNoCheating); }
	BitfieldBool bPlayersOnly() { return BoolValue(PropOffsets_LevelInfo.bPlayersOnly); }
	BitfieldBool bStartup() { return BoolValue(PropOffsets_LevelInfo.bStartup); }

	// 227 exclusive properties
	BitfieldBool bSupportsRealCrouching() { return BoolValue(PropOffsets_LevelInfo.bSupportsRealCrouching); }
	int& EdBuildOpt() { return Value<int>(PropOffsets_LevelInfo.EdBuildOpt); }
	UMusic*& backup_Song() { return Value<UMusic*>(PropOffsets_LevelInfo.backup_Song); }
	uint8_t& backup_SongSection() { return Value<uint8_t>(PropOffsets_LevelInfo.backup_SongSection); }
	UTexture*& WhiteTexture() { return Value<UTexture*>(PropOffsets_LevelInfo.WhiteTexture); }
	UTexture*& TemplateLightTex() { return Value<UTexture*>(PropOffsets_LevelInfo.TemplateLightTex); }
	std::string& EngineSubVersion() { return Value<std::string>(PropOffsets_LevelInfo.EngineSubVersion); }
	// FootStepManager is not a native class
	TypedScriptArray<UObject*> ObjList() { return DynamicArray<UObject*>(PropOffsets_LevelInfo.ObjList); }
	UDynamicZoneInfo*& DynamicZonesList() { return Value<UDynamicZoneInfo*>(PropOffsets_LevelInfo.DynamicZonesList); }
};
