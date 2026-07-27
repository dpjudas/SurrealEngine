#pragma once

#include "Packages/Extension/UPlayerPawnExt.h"

struct MeshAnimSeq;
class UDXGameDirectory;
class UGameInfo;

struct BlendAnimChannel
{
	MeshAnimSeq* Sequence = nullptr;
	float AnimRate = 0.0f;
	float AnimProgressLimit = 0.0f;
	float BlendAlpha = 0.0f;
	float BlendRate = 0.0f;
	float TweenSpeed = 0.0f;
	float FrameStep = 0.0f;
	float PreviousRate = 0.0f;

	int InternalRate = 0;
	int InternalAnimRate = 0;
	int InternalTween = 0;
	int InternalProgressLimit = 0;
};

class UDeusExPlayer : public UPlayerPawnExt
{
public:
	using UPlayerPawnExt::UPlayerPawnExt;

	void ConBindEvents();
	UObject* CreateDataVaultImageNoteObject();
	UObject* CreateDumpLocationObject();
	UObject* CreateGameDirectoryObject();
	UObject* CreateHistoryEvent();
	UObject* CreateHistoryObject();
	UObject* CreateLogObject();
	void DeleteSaveGameFiles(std::optional<std::string> saveDirectory);
	std::string GetDeusExVersion();
	void SaveGame(int saveIndex, std::optional<std::string> saveDesc);
	NameString SetBoolFlagFromString(const std::string& flagNameString, bool bValue);
	void UnloadTexture(UObject* Texture);

	//UComputers*& ActiveComputer() { return Value<UComputers*>(PropOffsets_DeusExPlayer.ActiveComputer); }
	std::string& AddedNanoKey() { return Value<std::string>(PropOffsets_DeusExPlayer.AddedNanoKey); }
	NameString& AugPrefs() { return Value<NameString>(PropOffsets_DeusExPlayer.AugPrefs); }
	//UAugmentationManager*& AugmentationSystem() { return Value<UAugmentationManager*>(PropOffsets_DeusExPlayer.AugmentationSystem); }
	//UBarkManager*& BarkManager() { return Value<UBarkManager*>(PropOffsets_DeusExPlayer.BarkManager); }
	float& BleedRate() { return Value<float>(PropOffsets_DeusExPlayer.BleedRate); }
	std::string& BurnString() { return Value<std::string>(PropOffsets_DeusExPlayer.BurnString); }
	std::string& CanCarryOnlyOne() { return Value<std::string>(PropOffsets_DeusExPlayer.CanCarryOnlyOne); }
	std::string& CannotDropHere() { return Value<std::string>(PropOffsets_DeusExPlayer.CannotDropHere); }
	std::string& CannotLift() { return Value<std::string>(PropOffsets_DeusExPlayer.CannotLift); }
	UInventory*& ClientinHandPending() { return Value<UInventory*>(PropOffsets_DeusExPlayer.ClientinHandPending); }
	float& ClotPeriod() { return Value<float>(PropOffsets_DeusExPlayer.ClotPeriod); }
	float& CombatDifficulty() { return Value<float>(PropOffsets_DeusExPlayer.CombatDifficulty); }
	//UConHistory*& ConHistory() { return Value<UConHistory*>(PropOffsets_DeusExPlayer.ConHistory); }
	//UConPlay*& ConPlay() { return Value<UConPlay*>(PropOffsets_DeusExPlayer.ConPlay); }
	UActor*& ConversationActor() { return Value<UActor*>(PropOffsets_DeusExPlayer.ConversationActor); }
	int& Credits() { return Value<int>(PropOffsets_DeusExPlayer.Credits); }
	UGameInfo*& DXGame() { return Value<UGameInfo*>(PropOffsets_DeusExPlayer.DXGame); }
	//Ushieldeffect*& DamageShield() { return Value<Ushieldeffect*>(PropOffsets_DeusExPlayer.DamageShield); }
	//UDataLinkPlay*& DataLinkPlay() { return Value<UDataLinkPlay*>(PropOffsets_DeusExPlayer.DataLinkPlay); }
	float& DropCounter() { return Value<float>(PropOffsets_DeusExPlayer.DropCounter); }
	float& Energy() { return Value<float>(PropOffsets_DeusExPlayer.Energy); }
	std::string& EnergyDepleted() { return Value<std::string>(PropOffsets_DeusExPlayer.EnergyDepleted); }
	float& EnergyDrain() { return Value<float>(PropOffsets_DeusExPlayer.EnergyDrain); }
	float& EnergyDrainTotal() { return Value<float>(PropOffsets_DeusExPlayer.EnergyDrainTotal); }
	float& EnergyMax() { return Value<float>(PropOffsets_DeusExPlayer.EnergyMax); }
	//UDeusExGoal*& FirstGoal() { return Value<UDeusExGoal*>(PropOffsets_DeusExPlayer.FirstGoal); }
	//UDataVaultImage*& FirstImage() { return Value<UDataVaultImage*>(PropOffsets_DeusExPlayer.FirstImage); }
	//UDeusExLog*& FirstLog() { return Value<UDeusExLog*>(PropOffsets_DeusExPlayer.FirstLog); }
	//UDeusExNote*& FirstNote() { return Value<UDeusExNote*>(PropOffsets_DeusExPlayer.FirstNote); }
	float& FlashTimer() { return Value<float>(PropOffsets_DeusExPlayer.FlashTimer); }
	NameString& FloorMaterial() { return Value<NameString>(PropOffsets_DeusExPlayer.FloorMaterial); }
	UActor*& FrobTarget() { return Value<UActor*>(PropOffsets_DeusExPlayer.FrobTarget); }
	float& FrobTime() { return Value<float>(PropOffsets_DeusExPlayer.FrobTime); }
	//UDebugInfo*& GlobalDebugObj() { return Value<UDebugInfo*>(PropOffsets_DeusExPlayer.GlobalDebugObj); }
	std::string& GoalAdded() { return Value<std::string>(PropOffsets_DeusExPlayer.GoalAdded); }
	std::string& HUDThemeName() { return Value<std::string>(PropOffsets_DeusExPlayer.HUDThemeName); }
	std::string& HandsFull() { return Value<std::string>(PropOffsets_DeusExPlayer.HandsFull); }
	std::string& HeadString() { return Value<std::string>(PropOffsets_DeusExPlayer.HeadString); }
	std::string& HealedPointLabel() { return Value<std::string>(PropOffsets_DeusExPlayer.HealedPointLabel); }
	std::string& HealedPointsLabel() { return Value<std::string>(PropOffsets_DeusExPlayer.HealedPointsLabel); }
	std::string& InventoryFull() { return Value<std::string>(PropOffsets_DeusExPlayer.InventoryFull); }
	float& JoltMagnitude() { return Value<float>(PropOffsets_DeusExPlayer.JoltMagnitude); }
	//UNanoKeyInfo*& KeyList() { return Value<UNanoKeyInfo*>(PropOffsets_DeusExPlayer.KeyList); }
	//UNanoKeyRing*& KeyRing() { return Value<UNanoKeyRing*>(PropOffsets_DeusExPlayer.KeyRing); }
	//UDeusExGoal*& LastGoal() { return Value<UDeusExGoal*>(PropOffsets_DeusExPlayer.LastGoal); }
	//UDeusExLog*& LastLog() { return Value<UDeusExLog*>(PropOffsets_DeusExPlayer.LastLog); }
	//UDeusExNote*& LastNote() { return Value<UDeusExNote*>(PropOffsets_DeusExPlayer.LastNote); }
	float& LastRefreshTime() { return Value<float>(PropOffsets_DeusExPlayer.LastRefreshTime); }
	UInventory*& LastinHand() { return Value<UInventory*>(PropOffsets_DeusExPlayer.LastinHand); }
	std::string& LegsString() { return Value<std::string>(PropOffsets_DeusExPlayer.LegsString); }
	float& MPDamageMult() { return Value<float>(PropOffsets_DeusExPlayer.MPDamageMult); }
	float& MaxFrobDistance() { return Value<float>(PropOffsets_DeusExPlayer.MaxFrobDistance); }
	float& MaxRegenPoint() { return Value<float>(PropOffsets_DeusExPlayer.MaxRegenPoint); }
	std::string& MenuThemeName() { return Value<std::string>(PropOffsets_DeusExPlayer.MenuThemeName); }
	std::string& NextMap() { return Value<std::string>(PropOffsets_DeusExPlayer.NextMap); }
	float& NintendoImmunityTime() { return Value<float>(PropOffsets_DeusExPlayer.NintendoImmunityTime); }
	float& NintendoImmunityTimeLeft() { return Value<float>(PropOffsets_DeusExPlayer.NintendoImmunityTimeLeft); }
	std::string& NoRoomToLift() { return Value<std::string>(PropOffsets_DeusExPlayer.NoRoomToLift); }
	std::string& NoneString() { return Value<std::string>(PropOffsets_DeusExPlayer.NoneString); }
	std::string& NoteAdded() { return Value<std::string>(PropOffsets_DeusExPlayer.NoteAdded); }
	int& PlayerSkin() { return Value<int>(PropOffsets_DeusExPlayer.PlayerSkin); }
	std::string& PoisonString() { return Value<std::string>(PropOffsets_DeusExPlayer.PoisonString); }
	std::string& PrimaryGoalCompleted() { return Value<std::string>(PropOffsets_DeusExPlayer.PrimaryGoalCompleted); }
	std::string& QuickSaveGameTitle() { return Value<std::string>(PropOffsets_DeusExPlayer.QuickSaveGameTitle); }
	float& RegenRate() { return Value<float>(PropOffsets_DeusExPlayer.RegenRate); }
	float& RunSilentValue() { return Value<float>(PropOffsets_DeusExPlayer.RunSilentValue); }
	std::string& SecondaryGoalCompleted() { return Value<std::string>(PropOffsets_DeusExPlayer.SecondaryGoalCompleted); }
	float& ServerTimeDiff() { return Value<float>(PropOffsets_DeusExPlayer.ServerTimeDiff); }
	float& ServerTimeLastRefresh() { return Value<float>(PropOffsets_DeusExPlayer.ServerTimeLastRefresh); }
	uint8_t& ShieldStatus() { return Value<uint8_t>(PropOffsets_DeusExPlayer.ShieldStatus); }
	float& ShieldTimer() { return Value<float>(PropOffsets_DeusExPlayer.ShieldTimer); }
	int& SkillPointsAvail() { return Value<int>(PropOffsets_DeusExPlayer.SkillPointsAvail); }
	std::string& SkillPointsAward() { return Value<std::string>(PropOffsets_DeusExPlayer.SkillPointsAward); }
	int& SkillPointsTotal() { return Value<int>(PropOffsets_DeusExPlayer.SkillPointsTotal); }
	//USkillManager*& SkillSystem() { return Value<USkillManager*>(PropOffsets_DeusExPlayer.SkillSystem); }
	std::string& TakenOverString() { return Value<std::string>(PropOffsets_DeusExPlayer.TakenOverString); }
	//UColorThemeManager*& ThemeManager() { return Value<UColorThemeManager*>(PropOffsets_DeusExPlayer.ThemeManager); }
	std::string& TooHeavyToLift() { return Value<std::string>(PropOffsets_DeusExPlayer.TooHeavyToLift); }
	std::string& TooMuchAmmo() { return Value<std::string>(PropOffsets_DeusExPlayer.TooMuchAmmo); }
	std::string& TorsoString() { return Value<std::string>(PropOffsets_DeusExPlayer.TorsoString); }
	std::string& TruePlayerName() { return Value<std::string>(PropOffsets_DeusExPlayer.TruePlayerName); }
	int& UIBackground() { return Value<int>(PropOffsets_DeusExPlayer.UIBackground); }
	UActor*& ViewModelActor() { return Value<UActor*>(PropOffsets_DeusExPlayer.ViewModelActor); }
	NameString& WallMaterial() { return Value<NameString>(PropOffsets_DeusExPlayer.WallMaterial); }
	vec3& WallNormal() { return Value<vec3>(PropOffsets_DeusExPlayer.WallNormal); }
	int& WarrenSlot() { return Value<int>(PropOffsets_DeusExPlayer.WarrenSlot); }
	float& WarrenTimer() { return Value<float>(PropOffsets_DeusExPlayer.WarrenTimer); }
	std::string& WeaponUnCloak() { return Value<std::string>(PropOffsets_DeusExPlayer.WeaponUnCloak); }
	std::string& WithString() { return Value<std::string>(PropOffsets_DeusExPlayer.WithString); }
	std::string& WithTheString() { return Value<std::string>(PropOffsets_DeusExPlayer.WithTheString); }
	//USpyDrone*& aDrone() { return Value<USpyDrone*>(PropOffsets_DeusExPlayer.aDrone); }
	BitfieldBool bAlwaysRun() { return BoolValue(PropOffsets_DeusExPlayer.bAlwaysRun); }
	BitfieldBool bAmmoDisplayVisible() { return BoolValue(PropOffsets_DeusExPlayer.bAmmoDisplayVisible); }
	BitfieldBool bAskedToTrain() { return BoolValue(PropOffsets_DeusExPlayer.bAskedToTrain); }
	BitfieldBool bAugDisplayVisible() { return BoolValue(PropOffsets_DeusExPlayer.bAugDisplayVisible); }
	BitfieldBool bAutoReload() { return BoolValue(PropOffsets_DeusExPlayer.bAutoReload); }
	BitfieldBool bBeltIsMPInventory() { return BoolValue(PropOffsets_DeusExPlayer.bBeltIsMPInventory); }
	BitfieldBool bBuySkills() { return BoolValue(PropOffsets_DeusExPlayer.bBuySkills); }
	BitfieldBool bCanLean() { return BoolValue(PropOffsets_DeusExPlayer.bCanLean); }
	BitfieldBool bCompassVisible() { return BoolValue(PropOffsets_DeusExPlayer.bCompassVisible); }
	BitfieldBool bConfirmNoteDeletes() { return BoolValue(PropOffsets_DeusExPlayer.bConfirmNoteDeletes); }
	BitfieldBool bConfirmSaveDeletes() { return BoolValue(PropOffsets_DeusExPlayer.bConfirmSaveDeletes); }
	BitfieldBool bCrosshairVisible() { return BoolValue(PropOffsets_DeusExPlayer.bCrosshairVisible); }
	BitfieldBool bCrouchOn() { return BoolValue(PropOffsets_DeusExPlayer.bCrouchOn); }
	BitfieldBool bDisplayAllGoals() { return BoolValue(PropOffsets_DeusExPlayer.bDisplayAllGoals); }
	BitfieldBool bDisplayAmmoByClip() { return BoolValue(PropOffsets_DeusExPlayer.bDisplayAmmoByClip); }
	BitfieldBool bDisplayCompletedGoals() { return BoolValue(PropOffsets_DeusExPlayer.bDisplayCompletedGoals); }
	BitfieldBool bFirstOptionsSynced() { return BoolValue(PropOffsets_DeusExPlayer.bFirstOptionsSynced); }
	BitfieldBool bForceDuck() { return BoolValue(PropOffsets_DeusExPlayer.bForceDuck); }
	BitfieldBool bHUDBackgroundTranslucent() { return BoolValue(PropOffsets_DeusExPlayer.bHUDBackgroundTranslucent); }
	BitfieldBool bHUDBordersTranslucent() { return BoolValue(PropOffsets_DeusExPlayer.bHUDBordersTranslucent); }
	BitfieldBool bHUDBordersVisible() { return BoolValue(PropOffsets_DeusExPlayer.bHUDBordersVisible); }
	BitfieldBool bHUDShowAllAugs() { return BoolValue(PropOffsets_DeusExPlayer.bHUDShowAllAugs); }
	BitfieldBool bHelpMessages() { return BoolValue(PropOffsets_DeusExPlayer.bHelpMessages); }
	BitfieldBool bHitDisplayVisible() { return BoolValue(PropOffsets_DeusExPlayer.bHitDisplayVisible); }
	BitfieldBool bIgnoreNextShowMenu() { return BoolValue(PropOffsets_DeusExPlayer.bIgnoreNextShowMenu); }
	BitfieldBool bInHandTransition() { return BoolValue(PropOffsets_DeusExPlayer.bInHandTransition); }
	BitfieldBool bKillerProfile() { return BoolValue(PropOffsets_DeusExPlayer.bKillerProfile); }
	BitfieldBool bMenusTranslucent() { return BoolValue(PropOffsets_DeusExPlayer.bMenusTranslucent); }
	BitfieldBool bNPCHighlighting() { return BoolValue(PropOffsets_DeusExPlayer.bNPCHighlighting); }
	BitfieldBool bNintendoImmunity() { return BoolValue(PropOffsets_DeusExPlayer.bNintendoImmunity); }
	BitfieldBool bObjectBeltVisible() { return BoolValue(PropOffsets_DeusExPlayer.bObjectBeltVisible); }
	BitfieldBool bObjectNames() { return BoolValue(PropOffsets_DeusExPlayer.bObjectNames); }
	BitfieldBool bQuotesEnabled() { return BoolValue(PropOffsets_DeusExPlayer.bQuotesEnabled); }
	BitfieldBool bSavingSkillsAugs() { return BoolValue(PropOffsets_DeusExPlayer.bSavingSkillsAugs); }
	BitfieldBool bSecondOptionsSynced() { return BoolValue(PropOffsets_DeusExPlayer.bSecondOptionsSynced); }
	BitfieldBool bShowAmmoDescriptions() { return BoolValue(PropOffsets_DeusExPlayer.bShowAmmoDescriptions); }
	BitfieldBool bSpyDroneActive() { return BoolValue(PropOffsets_DeusExPlayer.bSpyDroneActive); }
	BitfieldBool bStartNewGameAfterIntro() { return BoolValue(PropOffsets_DeusExPlayer.bStartNewGameAfterIntro); }
	BitfieldBool bStartingNewGame() { return BoolValue(PropOffsets_DeusExPlayer.bStartingNewGame); }
	BitfieldBool bSubtitles() { return BoolValue(PropOffsets_DeusExPlayer.bSubtitles); }
	BitfieldBool bToggleCrouch() { return BoolValue(PropOffsets_DeusExPlayer.bToggleCrouch); }
	BitfieldBool bToggleWalk() { return BoolValue(PropOffsets_DeusExPlayer.bToggleWalk); }
	BitfieldBool bWarrenEMPField() { return BoolValue(PropOffsets_DeusExPlayer.bWarrenEMPField); }
	BitfieldBool bWasCrouchOn() { return BoolValue(PropOffsets_DeusExPlayer.bWasCrouchOn); }
	float& curLeanDist() { return Value<float>(PropOffsets_DeusExPlayer.curLeanDist); }
	float& drugEffectTimer() { return Value<float>(PropOffsets_DeusExPlayer.drugEffectTimer); }
	UInventory*& inHand() { return Value<UInventory*>(PropOffsets_DeusExPlayer.inHand); }
	UInventory*& inHandPending() { return Value<UInventory*>(PropOffsets_DeusExPlayer.inHandPending); }
	uint8_t& invSlots() { return Value<uint8_t>(PropOffsets_DeusExPlayer.invSlots); }
	//UInvulnSphere*& invulnSph() { return Value<UInvulnSphere*>(PropOffsets_DeusExPlayer.invulnSph); }
	//UKillerProfile*& killProfile() { return Value<UKillerProfile*>(PropOffsets_DeusExPlayer.killProfile); }
	UActor*& lastFirstPersonConvoActor() { return Value<UActor*>(PropOffsets_DeusExPlayer.lastFirstPersonConvoActor); }
	float& lastFirstPersonConvoTime() { return Value<float>(PropOffsets_DeusExPlayer.lastFirstPersonConvoTime); }
	UActor*& lastThirdPersonConvoActor() { return Value<UActor*>(PropOffsets_DeusExPlayer.lastThirdPersonConvoActor); }
	float& lastThirdPersonConvoTime() { return Value<float>(PropOffsets_DeusExPlayer.lastThirdPersonConvoTime); }
	uint8_t& lastbDuck() { return Value<uint8_t>(PropOffsets_DeusExPlayer.lastbDuck); }
	float& logTimeout() { return Value<float>(PropOffsets_DeusExPlayer.logTimeout); }
	int& maxInvCols() { return Value<int>(PropOffsets_DeusExPlayer.maxInvCols); }
	int& maxInvRows() { return Value<int>(PropOffsets_DeusExPlayer.maxInvRows); }
	uint8_t& maxLogLines() { return Value<uint8_t>(PropOffsets_DeusExPlayer.maxLogLines); }
	int& mpMsgCode() { return Value<int>(PropOffsets_DeusExPlayer.mpMsgCode); }
	int& mpMsgFlags() { return Value<int>(PropOffsets_DeusExPlayer.mpMsgFlags); }
	int& mpMsgOptionalParam() { return Value<int>(PropOffsets_DeusExPlayer.mpMsgOptionalParam); }
	std::string& mpMsgOptionalString() { return Value<std::string>(PropOffsets_DeusExPlayer.mpMsgOptionalString); }
	int& mpMsgServerFlags() { return Value<int>(PropOffsets_DeusExPlayer.mpMsgServerFlags); }
	float& mpMsgTime() { return Value<float>(PropOffsets_DeusExPlayer.mpMsgTime); }
	float& musicChangeTimer() { return Value<float>(PropOffsets_DeusExPlayer.musicChangeTimer); }
	float& musicCheckTimer() { return Value<float>(PropOffsets_DeusExPlayer.musicCheckTimer); }
	uint8_t& musicMode() { return Value<uint8_t>(PropOffsets_DeusExPlayer.musicMode); }
	UPawn*& myBurner() { return Value<UPawn*>(PropOffsets_DeusExPlayer.myBurner); }
	UActor*& myKiller() { return Value<UActor*>(PropOffsets_DeusExPlayer.myKiller); }
	UPawn*& myPoisoner() { return Value<UPawn*>(PropOffsets_DeusExPlayer.myPoisoner); }
	UActor*& myProjKiller() { return Value<UActor*>(PropOffsets_DeusExPlayer.myProjKiller); }
	UActor*& myTurretKiller() { return Value<UActor*>(PropOffsets_DeusExPlayer.myTurretKiller); }
	int& poisonCounter() { return Value<int>(PropOffsets_DeusExPlayer.poisonCounter); }
	int& poisonDamage() { return Value<int>(PropOffsets_DeusExPlayer.poisonDamage); }
	float& poisonTimer() { return Value<float>(PropOffsets_DeusExPlayer.poisonTimer); }
	float& prevLeanDist() { return Value<float>(PropOffsets_DeusExPlayer.prevLeanDist); }
	int& saveCount() { return Value<int>(PropOffsets_DeusExPlayer.saveCount); }
	float& saveTime() { return Value<float>(PropOffsets_DeusExPlayer.saveTime); }
	uint8_t& savedSection() { return Value<uint8_t>(PropOffsets_DeusExPlayer.savedSection); }
	int& spyDroneLevel() { return Value<int>(PropOffsets_DeusExPlayer.spyDroneLevel); }
	float& spyDroneLevelValue() { return Value<float>(PropOffsets_DeusExPlayer.spyDroneLevelValue); }
	std::string& strStartMap() { return Value<std::string>(PropOffsets_DeusExPlayer.strStartMap); }
	float& swimBubbleTimer() { return Value<float>(PropOffsets_DeusExPlayer.swimBubbleTimer); }
	float& swimDuration() { return Value<float>(PropOffsets_DeusExPlayer.swimDuration); }
	float& swimTimer() { return Value<float>(PropOffsets_DeusExPlayer.swimTimer); }
	uint8_t& translucencyLevel() { return Value<uint8_t>(PropOffsets_DeusExPlayer.translucencyLevel); }

private:
	UDXGameDirectory* m_GameDirectory = nullptr;
};
