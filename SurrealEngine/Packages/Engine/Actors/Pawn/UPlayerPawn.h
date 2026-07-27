#pragma once

#include "UPawn.h"

class USavedMove;
class UPlayer;
class UMusic;
class UHUD;
class UGameReplicationInfo;

class UPlayerPawn : public UPawn
{
public:
	using UPawn::UPawn;

	void PausedInput(float elapsed);

	void Tick(float elapsed) override;
	void TickRotating(float elapsed) override;

	// Unreal 227 addition
	bool IsPressing(uint8_t KeyNum);

	float& AppliedBob() { return Value<float>(PropOffsets_PlayerPawn.AppliedBob); }
	float& Bob() { return Value<float>(PropOffsets_PlayerPawn.Bob); }
	float& BorrowedMouseX() { return Value<float>(PropOffsets_PlayerPawn.BorrowedMouseX); }
	float& BorrowedMouseY() { return Value<float>(PropOffsets_PlayerPawn.BorrowedMouseY); }
	UClass*& CarcassType() { return Value<UClass*>(PropOffsets_PlayerPawn.CarcassType); }
	uint8_t& CdTrack() { return Value<uint8_t>(PropOffsets_PlayerPawn.CdTrack); }
	float& ClientUpdateTime() { return Value<float>(PropOffsets_PlayerPawn.ClientUpdateTime); }
	vec3& ConstantGlowFog() { return Value<vec3>(PropOffsets_PlayerPawn.ConstantGlowFog); }
	float& ConstantGlowScale() { return Value<float>(PropOffsets_PlayerPawn.ConstantGlowScale); }
	float& CurrentTimeStamp() { return Value<float>(PropOffsets_PlayerPawn.CurrentTimeStamp); }
	float& DefaultFOV() { return Value<float>(PropOffsets_PlayerPawn.DefaultFOV); }
	std::string& DelayedCommand() { return Value<std::string>(PropOffsets_PlayerPawn.DelayedCommand); }
	int& DemoViewPitch() { return Value<int>(PropOffsets_PlayerPawn.DemoViewPitch); }
	int& DemoViewYaw() { return Value<int>(PropOffsets_PlayerPawn.DemoViewYaw); }
	float& DesiredFOV() { return Value<float>(PropOffsets_PlayerPawn.DesiredFOV); }
	vec3& DesiredFlashFog() { return Value<vec3>(PropOffsets_PlayerPawn.DesiredFlashFog); }
	float& DesiredFlashScale() { return Value<float>(PropOffsets_PlayerPawn.DesiredFlashScale); }
	float& DodgeClickTime() { return Value<float>(PropOffsets_PlayerPawn.DodgeClickTime); }
	float& DodgeClickTimer() { return Value<float>(PropOffsets_PlayerPawn.DodgeClickTimer); }
	std::string& FailedView() { return Value<std::string>(PropOffsets_PlayerPawn.FailedView); }
	vec3& FlashFog() { return Value<vec3>(PropOffsets_PlayerPawn.FlashFog); }
	vec3& FlashScale() { return Value<vec3>(PropOffsets_PlayerPawn.FlashScale); }
	USavedMove*& FreeMoves() { return Value<USavedMove*>(PropOffsets_PlayerPawn.FreeMoves); }
	UGameReplicationInfo*& GameReplicationInfo() { return Value<UGameReplicationInfo*>(PropOffsets_PlayerPawn.GameReplicationInfo); }
	UClass*& HUDType() { return Value<UClass*>(PropOffsets_PlayerPawn.HUDType); }
	float& Handedness() { return Value<float>(PropOffsets_PlayerPawn.Handedness); }
	float& InstantFlash() { return Value<float>(PropOffsets_PlayerPawn.InstantFlash); }
	vec3& InstantFog() { return Value<vec3>(PropOffsets_PlayerPawn.InstantFog); }
	USound*& JumpSound() { return Value<USound*>(PropOffsets_PlayerPawn.JumpSound); }
	float& LandBob() { return Value<float>(PropOffsets_PlayerPawn.LandBob); }
	float& LastMessageWindow() { return Value<float>(PropOffsets_PlayerPawn.LastMessageWindow); }
	float& LastPlaySound() { return Value<float>(PropOffsets_PlayerPawn.LastPlaySound); }
	float& LastUpdateTime() { return Value<float>(PropOffsets_PlayerPawn.LastUpdateTime); }
	float& MaxTimeMargin() { return Value<float>(PropOffsets_PlayerPawn.MaxTimeMargin); }
	int& Misc1() { return Value<int>(PropOffsets_PlayerPawn.Misc1); }
	int& Misc2() { return Value<int>(PropOffsets_PlayerPawn.Misc2); }
	float& MouseSensitivity() { return Value<float>(PropOffsets_PlayerPawn.MouseSensitivity); }
	float& MouseSmoothThreshold() { return Value<float>(PropOffsets_PlayerPawn.MouseSmoothThreshold); }
	float& MouseZeroTime() { return Value<float>(PropOffsets_PlayerPawn.MouseZeroTime); }
	float& MyAutoAim() { return Value<float>(PropOffsets_PlayerPawn.MyAutoAim); }
	std::string& NoPauseMessage() { return Value<std::string>(PropOffsets_PlayerPawn.NoPauseMessage); }
	std::string& OwnCamera() { return Value<std::string>(PropOffsets_PlayerPawn.OwnCamera); }
	std::string& Password() { return Value<std::string>(PropOffsets_PlayerPawn.Password); }
	USavedMove*& PendingMove() { return Value<USavedMove*>(PropOffsets_PlayerPawn.PendingMove); }
	UPlayer*& Player() { return Value<UPlayer*>(PropOffsets_PlayerPawn.Player); }
	FixedArrayView<Color*, 5> ProgressColor() { return FixedArray<Color*, 5>(PropOffsets_PlayerPawn.ProgressColor); }
	FixedArrayView<std::optional<std::string>, 5> ProgressMessage() { return FixedArray<std::optional<std::string>, 5>(PropOffsets_PlayerPawn.ProgressMessage); }
	float& ProgressTimeOut() { return Value<float>(PropOffsets_PlayerPawn.ProgressTimeOut); }
	std::string& QuickSaveString() { return Value<std::string>(PropOffsets_PlayerPawn.QuickSaveString); }
	BitfieldBool ReceivedSecretChecksum() { return BoolValue(PropOffsets_PlayerPawn.ReceivedSecretChecksum); }
	int& RendMap() { return Value<int>(PropOffsets_PlayerPawn.RendMap); }
	USavedMove*& SavedMoves() { return Value<USavedMove*>(PropOffsets_PlayerPawn.SavedMoves); }
	// UScoreBoard*& Scoring() { return Value<UScoreBoard*>(PropOffsets_PlayerPawn.Scoring); }
	UClass*& ScoringType() { return Value<UClass*>(PropOffsets_PlayerPawn.ScoringType); }
	float& ServerTimeStamp() { return Value<float>(PropOffsets_PlayerPawn.ServerTimeStamp); }
	int& ShowFlags() { return Value<int>(PropOffsets_PlayerPawn.ShowFlags); }
	float& SmoothMouseX() { return Value<float>(PropOffsets_PlayerPawn.SmoothMouseX); }
	float& SmoothMouseY() { return Value<float>(PropOffsets_PlayerPawn.SmoothMouseY); }
	UMusic*& Song() { return Value<UMusic*>(PropOffsets_PlayerPawn.Song); }
	uint8_t& SongSection() { return Value<uint8_t>(PropOffsets_PlayerPawn.SongSection); }
	UClass*& SpecialMenu() { return Value<UClass*>(PropOffsets_PlayerPawn.SpecialMenu); }
	float& TargetEyeHeight() { return Value<float>(PropOffsets_PlayerPawn.TargetEyeHeight); }
	Rotator& TargetViewRotation() { return Value<Rotator>(PropOffsets_PlayerPawn.TargetViewRotation); }
	vec3& TargetWeaponViewOffset() { return Value<vec3>(PropOffsets_PlayerPawn.TargetWeaponViewOffset); }
	float& TimeMargin() { return Value<float>(PropOffsets_PlayerPawn.TimeMargin); }
	uint8_t& Transition() { return Value<uint8_t>(PropOffsets_PlayerPawn.Transition); }
	UActor*& ViewTarget() { return Value<UActor*>(PropOffsets_PlayerPawn.ViewTarget); }
	std::string& ViewingFrom() { return Value<std::string>(PropOffsets_PlayerPawn.ViewingFrom); }
	FixedArrayView<NameString*, 20> WeaponPriority() { return FixedArray<NameString*, 20>(PropOffsets_PlayerPawn.WeaponPriority); }
	float& ZoomLevel() { return Value<float>(PropOffsets_PlayerPawn.ZoomLevel); }
	float& aBaseX() { return Value<float>(PropOffsets_PlayerPawn.aBaseX); }
	float& aBaseY() { return Value<float>(PropOffsets_PlayerPawn.aBaseY); }
	float& aBaseZ() { return Value<float>(PropOffsets_PlayerPawn.aBaseZ); }
	float& aExtra0() { return Value<float>(PropOffsets_PlayerPawn.aExtra0); }
	float& aExtra1() { return Value<float>(PropOffsets_PlayerPawn.aExtra1); }
	float& aExtra2() { return Value<float>(PropOffsets_PlayerPawn.aExtra2); }
	float& aExtra3() { return Value<float>(PropOffsets_PlayerPawn.aExtra3); }
	float& aExtra4() { return Value<float>(PropOffsets_PlayerPawn.aExtra4); }
	float& aForward() { return Value<float>(PropOffsets_PlayerPawn.aForward); }
	float& aLookUp() { return Value<float>(PropOffsets_PlayerPawn.aLookUp); }
	float& aMouseX() { return Value<float>(PropOffsets_PlayerPawn.aMouseX); }
	float& aMouseY() { return Value<float>(PropOffsets_PlayerPawn.aMouseY); }
	float& aStrafe() { return Value<float>(PropOffsets_PlayerPawn.aStrafe); }
	float& aTurn() { return Value<float>(PropOffsets_PlayerPawn.aTurn); }
	float& aUp() { return Value<float>(PropOffsets_PlayerPawn.aUp); }
	BitfieldBool bAdmin() { return BoolValue(PropOffsets_PlayerPawn.bAdmin); }
	BitfieldBool bAlwaysMouseLook() { return BoolValue(PropOffsets_PlayerPawn.bAlwaysMouseLook); }
	BitfieldBool bAnimTransition() { return BoolValue(PropOffsets_PlayerPawn.bAnimTransition); }
	BitfieldBool bBadConnectionAlert() { return BoolValue(PropOffsets_PlayerPawn.bBadConnectionAlert); }
	BitfieldBool bCenterView() { return BoolValue(PropOffsets_PlayerPawn.bCenterView); }
	BitfieldBool bCheatsEnabled() { return BoolValue(PropOffsets_PlayerPawn.bCheatsEnabled); }
	BitfieldBool bDelayedCommand() { return BoolValue(PropOffsets_PlayerPawn.bDelayedCommand); }
	BitfieldBool bEdgeBack() { return BoolValue(PropOffsets_PlayerPawn.bEdgeBack); }
	BitfieldBool bEdgeForward() { return BoolValue(PropOffsets_PlayerPawn.bEdgeForward); }
	BitfieldBool bEdgeLeft() { return BoolValue(PropOffsets_PlayerPawn.bEdgeLeft); }
	BitfieldBool bEdgeRight() { return BoolValue(PropOffsets_PlayerPawn.bEdgeRight); }
	BitfieldBool bFixedCamera() { return BoolValue(PropOffsets_PlayerPawn.bFixedCamera); }
	BitfieldBool bFrozen() { return BoolValue(PropOffsets_PlayerPawn.bFrozen); }
	BitfieldBool bInvertMouse() { return BoolValue(PropOffsets_PlayerPawn.bInvertMouse); }
	BitfieldBool bIsCrouching() { return BoolValue(PropOffsets_PlayerPawn.bIsCrouching); }
	BitfieldBool bIsTurning() { return BoolValue(PropOffsets_PlayerPawn.bIsTurning); }
	BitfieldBool bIsTyping() { return BoolValue(PropOffsets_PlayerPawn.bIsTyping); }
	BitfieldBool bJumpStatus() { return BoolValue(PropOffsets_PlayerPawn.bJumpStatus); }
	BitfieldBool bJustAltFired() { return BoolValue(PropOffsets_PlayerPawn.bJustAltFired); }
	BitfieldBool bJustFired() { return BoolValue(PropOffsets_PlayerPawn.bJustFired); }
	BitfieldBool bKeyboardLook() { return BoolValue(PropOffsets_PlayerPawn.bKeyboardLook); }
	BitfieldBool bLookUpStairs() { return BoolValue(PropOffsets_PlayerPawn.bLookUpStairs); }
	BitfieldBool bMaxMouseSmoothing() { return BoolValue(PropOffsets_PlayerPawn.bMaxMouseSmoothing); }
	BitfieldBool bMessageBeep() { return BoolValue(PropOffsets_PlayerPawn.bMessageBeep); }
	BitfieldBool bMouseZeroed() { return BoolValue(PropOffsets_PlayerPawn.bMouseZeroed); }
	BitfieldBool bNeverAutoSwitch() { return BoolValue(PropOffsets_PlayerPawn.bNeverAutoSwitch); }
	BitfieldBool bNoFlash() { return BoolValue(PropOffsets_PlayerPawn.bNoFlash); }
	BitfieldBool bNoVoices() { return BoolValue(PropOffsets_PlayerPawn.bNoVoices); }
	BitfieldBool bPressedJump() { return BoolValue(PropOffsets_PlayerPawn.bPressedJump); }
	BitfieldBool bReadyToPlay() { return BoolValue(PropOffsets_PlayerPawn.bReadyToPlay); }
	BitfieldBool bReducedVis() { return BoolValue(PropOffsets_PlayerPawn.bReducedVis); }
	BitfieldBool bRising() { return BoolValue(PropOffsets_PlayerPawn.bRising); }
	BitfieldBool bShakeDir() { return BoolValue(PropOffsets_PlayerPawn.bShakeDir); }
	BitfieldBool bShowMenu() { return BoolValue(PropOffsets_PlayerPawn.bShowMenu); }
	BitfieldBool bShowScores() { return BoolValue(PropOffsets_PlayerPawn.bShowScores); }
	BitfieldBool bSinglePlayer() { return BoolValue(PropOffsets_PlayerPawn.bSinglePlayer); }
	BitfieldBool bSnapToLevel() { return BoolValue(PropOffsets_PlayerPawn.bSnapToLevel); }
	BitfieldBool bSpecialMenu() { return BoolValue(PropOffsets_PlayerPawn.bSpecialMenu); }
	BitfieldBool bUpdatePosition() { return BoolValue(PropOffsets_PlayerPawn.bUpdatePosition); }
	BitfieldBool bUpdating() { return BoolValue(PropOffsets_PlayerPawn.bUpdating); }
	BitfieldBool bWasBack() { return BoolValue(PropOffsets_PlayerPawn.bWasBack); }
	BitfieldBool bWasForward() { return BoolValue(PropOffsets_PlayerPawn.bWasForward); }
	BitfieldBool bWasLeft() { return BoolValue(PropOffsets_PlayerPawn.bWasLeft); }
	BitfieldBool bWasRight() { return BoolValue(PropOffsets_PlayerPawn.bWasRight); }
	BitfieldBool bWokeUp() { return BoolValue(PropOffsets_PlayerPawn.bWokeUp); }
	BitfieldBool bZooming() { return BoolValue(PropOffsets_PlayerPawn.bZooming); }
	float& bobtime() { return Value<float>(PropOffsets_PlayerPawn.bobtime); }
	float& maxshake() { return Value<float>(PropOffsets_PlayerPawn.maxshake); }
	UHUD*& myHUD() { return Value<UHUD*>(PropOffsets_PlayerPawn.myHUD); }
	BitfieldBool ngSecretSet() { return BoolValue(PropOffsets_PlayerPawn.ngSecretSet); }
	std::string& ngWorldSecret() { return Value<std::string>(PropOffsets_PlayerPawn.ngWorldSecret); }
	int& shakemag() { return Value<int>(PropOffsets_PlayerPawn.shakemag); }
	float& shaketimer() { return Value<float>(PropOffsets_PlayerPawn.shaketimer); }
	float& shakevert() { return Value<float>(PropOffsets_PlayerPawn.shakevert); }
	float& verttimer() { return Value<float>(PropOffsets_PlayerPawn.verttimer); }

	void LoadProperties(); // Always loaded from User.ini
	void SaveConfig() override;
};
