#pragma once

#include "Packages/Engine/Actors/UActor.h"

class USound;
class UNavigationPoint;
class UWeapon;
class UInventory;
class UDecal;
class UDecoration;
class UPlayerReplicationInfo;

class UPawn : public UActor
{
public:
	using UActor::UActor;

	void Tick(float elapsed) override;
	void TickRotating(float elapsed) override;

	void InitActorZone() override;
	void UpdateActorZone() override;

	void MoveTo(const vec3& newDestination, float speed);
	void MoveToward(UActor* newTarget, float speed);
	void StrafeFacing(const vec3& newDestination, UActor* newTarget);
	void StrafeTo(const vec3& newDestination, const vec3& newFocus);
	void TurnTo(const vec3& newFocus);
	void TurnToward(UActor* newTarget);
	void WaitForLanding();
	void SetMoveDuration(const vec3& deltaMove);
	float GetSpeed();

	bool TickRotateTo(const vec3& target);
	bool TickMoveTo(const vec3& target);

	// Returns true if any of the several points of other is visible (origin, top, bottom)
	// ignoreDistance is a Deus Ex only parameter, it is always false on Unreal.
	bool LineOfSightTo(UActor* other, bool ignoreDistance);
	// Similar to LineOfSightTo() but takes the Pawn's peripheral vision into account (SightRadius and PeripheralVision)
	bool CanSee(UActor* other);
	bool CanHearNoise(UActor* source, float loudness);
	bool ActorReachable(UActor* anActor, bool checkNavpoint = false);
	bool PointReachable(vec3 aPoint);

	bool ReachableFlying(UActor* anActor);
	bool ReachableSpider(UActor* anActor);
	bool ReachableSwimming(UActor* anActor);
	bool ReachableWalking(UActor* anActor);

	void ClientHearSound(UActor* actor, int id, USound* sound, const vec3& soundLocation, const vec3& parameters);

	// If the obstruction is jumpable, start jumping and keep the destination
	// Otherwise try rotating destination 90 degrees to left and right
	bool PickWallAdjust();

	vec3 EAdjustJump();

	UActor* PickAnyTarget(float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart);
	UActor* PickTarget(float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart);
	bool CheckIfBestTarget(UActor* actor, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart);

	UActor* PathSpecialHandling(const Array<UNavigationPoint*>& points);
	UNavigationPoint* SetRouteCache(const Array<UNavigationPoint*>& points);
	std::pair<Array<UNavigationPoint*>, int32_t> FindPathToEndPoint(UNavigationPoint* start, int maxNodes);

	void ClearPaths();
	UObject* FindRandomDest();
	UObject* FindPathTo(const vec3& aPoint, bool bSinglePath);
	UObject* FindPathToward(UObject* anActor, bool singlePath);
	UObject* FindBestInventoryPath(bool predictRespawns, float& outBestWeight);
	UNavigationPoint* FindClosestNavPoint(vec3 location);
	bool MarkReachableNavEndPoints();

	// Deus Ex AI functions
	float AICanHear(UActor* other, std::optional<float> volume, std::optional<float> radius);
	float AICanSee(UActor* other, std::optional<float> visibility, std::optional<bool> bCheckVisibility, std::optional<bool> bCheckDir, std::optional<bool> bCheckCylinder, std::optional<bool> bCheckLOS);
	float AICanSmell(UActor* other, std::optional<float> smell);

	float& AccelRate() { return Value<float>(PropOffsets_Pawn.AccelRate); }
	float& AirControl() { return Value<float>(PropOffsets_Pawn.AirControl); }
	float& AirSpeed() { return Value<float>(PropOffsets_Pawn.AirSpeed); }
	NameString& AlarmTag() { return Value<NameString>(PropOffsets_Pawn.AlarmTag); }
	float& Alertness() { return Value<float>(PropOffsets_Pawn.Alertness); }
	uint8_t& AttitudeToPlayer() { return Value<uint8_t>(PropOffsets_Pawn.AttitudeToPlayer); }
	float& AvgPhysicsTime() { return Value<float>(PropOffsets_Pawn.AvgPhysicsTime); }
	float& BaseEyeHeight() { return Value<float>(PropOffsets_Pawn.BaseEyeHeight); }
	float& CombatStyle() { return Value<float>(PropOffsets_Pawn.CombatStyle); }
	float& DamageScaling() { return Value<float>(PropOffsets_Pawn.DamageScaling); }
	float& DesiredSpeed() { return Value<float>(PropOffsets_Pawn.DesiredSpeed); }
	vec3& Destination() { return Value<vec3>(PropOffsets_Pawn.Destination); }
	USound*& Die() { return Value<USound*>(PropOffsets_Pawn.Die); }
	int& DieCount() { return Value<int>(PropOffsets_Pawn.DieCount); }
	UClass*& DropWhenKilled() { return Value<UClass*>(PropOffsets_Pawn.DropWhenKilled); }
	UPawn*& Enemy() { return Value<UPawn*>(PropOffsets_Pawn.Enemy); }
	float& EyeHeight() { return Value<float>(PropOffsets_Pawn.EyeHeight); }
	UActor*& FaceTarget() { return Value<UActor*>(PropOffsets_Pawn.FaceTarget); }
	vec3& Floor() { return Value<vec3>(PropOffsets_Pawn.Floor); }
	vec3& Focus() { return Value<vec3>(PropOffsets_Pawn.Focus); }
	PointRegion& FootRegion() { return Value<PointRegion>(PropOffsets_Pawn.FootRegion); }
	float& FovAngle() { return Value<float>(PropOffsets_Pawn.FovAngle); }
	float& GroundSpeed() { return Value<float>(PropOffsets_Pawn.GroundSpeed); }
	PointRegion& HeadRegion() { return Value<PointRegion>(PropOffsets_Pawn.HeadRegion); }
	int& Health() { return Value<int>(PropOffsets_Pawn.Health); }
	float& HearingThreshold() { return Value<float>(PropOffsets_Pawn.HearingThreshold); }
	USound*& HitSound1() { return Value<USound*>(PropOffsets_Pawn.HitSound1); }
	USound*& HitSound2() { return Value<USound*>(PropOffsets_Pawn.HitSound2); }
	uint8_t& Intelligence() { return Value<uint8_t>(PropOffsets_Pawn.Intelligence); }
	int& ItemCount() { return Value<int>(PropOffsets_Pawn.ItemCount); }
	float& JumpZ() { return Value<float>(PropOffsets_Pawn.JumpZ); }
	int& KillCount() { return Value<int>(PropOffsets_Pawn.KillCount); }
	USound*& Land() { return Value<USound*>(PropOffsets_Pawn.Land); }
	float& LastPainSound() { return Value<float>(PropOffsets_Pawn.LastPainSound); }
	vec3& LastSeeingPos() { return Value<vec3>(PropOffsets_Pawn.LastSeeingPos); }
	vec3& LastSeenPos() { return Value<vec3>(PropOffsets_Pawn.LastSeenPos); }
	float& LastSeenTime() { return Value<float>(PropOffsets_Pawn.LastSeenTime); }
	float& MaxDesiredSpeed() { return Value<float>(PropOffsets_Pawn.MaxDesiredSpeed); }
	float& MaxStepHeight() { return Value<float>(PropOffsets_Pawn.MaxStepHeight); }
	float& MeleeRange() { return Value<float>(PropOffsets_Pawn.MeleeRange); }
	std::string& MenuName() { return Value<std::string>(PropOffsets_Pawn.MenuName); }
	float& MinHitWall() { return Value<float>(PropOffsets_Pawn.MinHitWall); }
	UActor*& MoveTarget() { return Value<UActor*>(PropOffsets_Pawn.MoveTarget); }
	float& MoveTimer() { return Value<float>(PropOffsets_Pawn.MoveTimer); }
	std::string& NameArticle() { return Value<std::string>(PropOffsets_Pawn.NameArticle); }
	NameString& NextLabel() { return Value<NameString>(PropOffsets_Pawn.NextLabel); }
	NameString& NextState() { return Value<NameString>(PropOffsets_Pawn.NextState); }
	float& OldMessageTime() { return Value<float>(PropOffsets_Pawn.OldMessageTime); }
	float& OrthoZoom() { return Value<float>(PropOffsets_Pawn.OrthoZoom); }
	float& PainTime() { return Value<float>(PropOffsets_Pawn.PainTime); }
	UWeapon*& PendingWeapon() { return Value<UWeapon*>(PropOffsets_Pawn.PendingWeapon); }
	float& PeripheralVision() { return Value<float>(PropOffsets_Pawn.PeripheralVision); }
	NameString& PlayerReStartState() { return Value<NameString>(PropOffsets_Pawn.PlayerReStartState); }
	UPlayerReplicationInfo*& PlayerReplicationInfo() { return Value<UPlayerReplicationInfo*>(PropOffsets_Pawn.PlayerReplicationInfo); }
	UClass*& PlayerReplicationInfoClass() { return Value<UClass*>(PropOffsets_Pawn.PlayerReplicationInfoClass); }
	float& ReducedDamagePct() { return Value<float>(PropOffsets_Pawn.ReducedDamagePct); }
	NameString& ReducedDamageType() { return Value<NameString>(PropOffsets_Pawn.ReducedDamageType); }
	FixedArrayView<UNavigationPoint*, 16> RouteCache() { return FixedArray<UNavigationPoint*, 16>(PropOffsets_Pawn.RouteCache); }
	int& SecretCount() { return Value<int>(PropOffsets_Pawn.SecretCount); }
	UInventory*& SelectedItem() { return Value<UInventory*>(PropOffsets_Pawn.SelectedItem); }
	std::string& SelectionMesh() { return Value<std::string>(PropOffsets_Pawn.SelectionMesh); }
	UDecal*& Shadow() { return Value<UDecal*>(PropOffsets_Pawn.Shadow); }
	NameString& SharedAlarmTag() { return Value<NameString>(PropOffsets_Pawn.SharedAlarmTag); }
	float& SightCounter() { return Value<float>(PropOffsets_Pawn.SightCounter); }
	float& SightRadius() { return Value<float>(PropOffsets_Pawn.SightRadius); }
	float& Skill() { return Value<float>(PropOffsets_Pawn.Skill); }
	float& SoundDampening() { return Value<float>(PropOffsets_Pawn.SoundDampening); }
	UActor*& SpecialGoal() { return Value<UActor*>(PropOffsets_Pawn.SpecialGoal); }
	std::string& SpecialMesh() { return Value<std::string>(PropOffsets_Pawn.SpecialMesh); }
	float& SpecialPause() { return Value<float>(PropOffsets_Pawn.SpecialPause); }
	float& SpeechTime() { return Value<float>(PropOffsets_Pawn.SpeechTime); }
	float& SplashTime() { return Value<float>(PropOffsets_Pawn.SplashTime); }
	int& Spree() { return Value<int>(PropOffsets_Pawn.Spree); }
	float& Stimulus() { return Value<float>(PropOffsets_Pawn.Stimulus); }
	float& UnderWaterTime() { return Value<float>(PropOffsets_Pawn.UnderWaterTime); }
	Rotator& ViewRotation() { return Value<Rotator>(PropOffsets_Pawn.ViewRotation); }
	uint8_t& Visibility() { return Value<uint8_t>(PropOffsets_Pawn.Visibility); }
	uint8_t& VoicePitch() { return Value<uint8_t>(PropOffsets_Pawn.VoicePitch); }
	std::string& VoiceType() { return Value<std::string>(PropOffsets_Pawn.VoiceType); }
	vec3& WalkBob() { return Value<vec3>(PropOffsets_Pawn.WalkBob); }
	float& WaterSpeed() { return Value<float>(PropOffsets_Pawn.WaterSpeed); }
	USound*& WaterStep() { return Value<USound*>(PropOffsets_Pawn.WaterStep); }
	UWeapon*& Weapon() { return Value<UWeapon*>(PropOffsets_Pawn.Weapon); }
	BitfieldBool bAdvancedTactics() { return BoolValue(PropOffsets_Pawn.bAdvancedTactics); }
	uint8_t& bAltFire() { return Value<uint8_t>(PropOffsets_Pawn.bAltFire); }
	BitfieldBool bAutoActivate() { return BoolValue(PropOffsets_Pawn.bAutoActivate); }
	BitfieldBool bAvoidLedges() { return BoolValue(PropOffsets_Pawn.bAvoidLedges); }
	BitfieldBool bBehindView() { return BoolValue(PropOffsets_Pawn.bBehindView); }
	BitfieldBool bCanDoSpecial() { return BoolValue(PropOffsets_Pawn.bCanDoSpecial); }
	BitfieldBool bCanFly() { return BoolValue(PropOffsets_Pawn.bCanFly); }
	BitfieldBool bCanJump() { return BoolValue(PropOffsets_Pawn.bCanJump); }
	BitfieldBool bCanOpenDoors() { return BoolValue(PropOffsets_Pawn.bCanOpenDoors); }
	BitfieldBool bCanStrafe() { return BoolValue(PropOffsets_Pawn.bCanStrafe); }
	BitfieldBool bCanSwim() { return BoolValue(PropOffsets_Pawn.bCanSwim); }
	BitfieldBool bCanWalk() { return BoolValue(PropOffsets_Pawn.bCanWalk); }
	BitfieldBool bCountJumps() { return BoolValue(PropOffsets_Pawn.bCountJumps); }
	BitfieldBool bDrowning() { return BoolValue(PropOffsets_Pawn.bDrowning); }
	uint8_t& bDuck() { return Value<uint8_t>(PropOffsets_Pawn.bDuck); }
	uint8_t& bExtra0() { return Value<uint8_t>(PropOffsets_Pawn.bExtra0); }
	uint8_t& bExtra1() { return Value<uint8_t>(PropOffsets_Pawn.bExtra1); }
	uint8_t& bExtra2() { return Value<uint8_t>(PropOffsets_Pawn.bExtra2); }
	uint8_t& bExtra3() { return Value<uint8_t>(PropOffsets_Pawn.bExtra3); }
	uint8_t& bFire() { return Value<uint8_t>(PropOffsets_Pawn.bFire); }
	BitfieldBool bFixedStart() { return BoolValue(PropOffsets_Pawn.bFixedStart); }
	uint8_t& bFreeLook() { return Value<uint8_t>(PropOffsets_Pawn.bFreeLook); }
	BitfieldBool bFromWall() { return BoolValue(PropOffsets_Pawn.bFromWall); }
	BitfieldBool bHitSlopedWall() { return BoolValue(PropOffsets_Pawn.bHitSlopedWall); }
	BitfieldBool bHunting() { return BoolValue(PropOffsets_Pawn.bHunting); }
	BitfieldBool bIsFemale() { return BoolValue(PropOffsets_Pawn.bIsFemale); }
	BitfieldBool bIsHuman() { return BoolValue(PropOffsets_Pawn.bIsHuman); }
	BitfieldBool bIsMultiSkinned() { return BoolValue(PropOffsets_Pawn.bIsMultiSkinned); }
	BitfieldBool bIsPlayer() { return BoolValue(PropOffsets_Pawn.bIsPlayer); }
	BitfieldBool bIsWalking() { return BoolValue(PropOffsets_Pawn.bIsWalking); }
	BitfieldBool bJumpOffPawn() { return BoolValue(PropOffsets_Pawn.bJumpOffPawn); }
	BitfieldBool bJustLanded() { return BoolValue(PropOffsets_Pawn.bJustLanded); }
	BitfieldBool bLOSflag() { return BoolValue(PropOffsets_Pawn.bLOSflag); }
	uint8_t& bLook() { return Value<uint8_t>(PropOffsets_Pawn.bLook); }
	BitfieldBool bNeverSwitchOnPickup() { return BoolValue(PropOffsets_Pawn.bNeverSwitchOnPickup); }
	BitfieldBool bReducedSpeed() { return BoolValue(PropOffsets_Pawn.bReducedSpeed); }
	uint8_t& bRun() { return Value<uint8_t>(PropOffsets_Pawn.bRun); }
	BitfieldBool bShootSpecial() { return BoolValue(PropOffsets_Pawn.bShootSpecial); }
	uint8_t& bSnapLevel() { return Value<uint8_t>(PropOffsets_Pawn.bSnapLevel); }
	BitfieldBool bStopAtLedges() { return BoolValue(PropOffsets_Pawn.bStopAtLedges); }
	uint8_t& bStrafe() { return Value<uint8_t>(PropOffsets_Pawn.bStrafe); }
	BitfieldBool bUpAndOut() { return BoolValue(PropOffsets_Pawn.bUpAndOut); }
	BitfieldBool bUpdatingDisplay() { return BoolValue(PropOffsets_Pawn.bUpdatingDisplay); }
	BitfieldBool bViewTarget() { return BoolValue(PropOffsets_Pawn.bViewTarget); }
	BitfieldBool bWarping() { return BoolValue(PropOffsets_Pawn.bWarping); }
	uint8_t& bZoom() { return Value<uint8_t>(PropOffsets_Pawn.bZoom); }
	UDecoration*& carriedDecoration() { return Value<UDecoration*>(PropOffsets_Pawn.carriedDecoration); }
	UNavigationPoint*& home() { return Value<UNavigationPoint*>(PropOffsets_Pawn.home); }
	UPawn*& nextPawn() { return Value<UPawn*>(PropOffsets_Pawn.nextPawn); }
	float& noise1loudness() { return Value<float>(PropOffsets_Pawn.noise1loudness); }
	UPawn*& noise1other() { return Value<UPawn*>(PropOffsets_Pawn.noise1other); }
	vec3& noise1spot() { return Value<vec3>(PropOffsets_Pawn.noise1spot); }
	float& noise1time() { return Value<float>(PropOffsets_Pawn.noise1time); }
	float& noise2loudness() { return Value<float>(PropOffsets_Pawn.noise2loudness); }
	UPawn*& noise2other() { return Value<UPawn*>(PropOffsets_Pawn.noise2other); }
	vec3& noise2spot() { return Value<vec3>(PropOffsets_Pawn.noise2spot); }
	float& noise2time() { return Value<float>(PropOffsets_Pawn.noise2time); }

	// Deus Ex exclusive properties
	BitfieldBool bCanGlide() { return BoolValue(PropOffsets_Pawn.bCanGlide); }
	int& HealthHead() { return Value<int>(PropOffsets_Pawn.HealthHead); }
	int& HealthTorso() { return Value<int>(PropOffsets_Pawn.HealthTorso); }
	int& HealthLegLeft() { return Value<int>(PropOffsets_Pawn.HealthLegLeft); }
	int& HealthLegRight() { return Value<int>(PropOffsets_Pawn.HealthLegRight); }
	int& HealthArmLeft() { return Value<int>(PropOffsets_Pawn.HealthArmLeft); }
	int& HealthArmRight() { return Value<int>(PropOffsets_Pawn.HealthArmRight); }
	BitfieldBool bIsSpeaking() { return BoolValue(PropOffsets_Pawn.bIsSpeaking); }
	BitfieldBool bWasSpeaking() { return BoolValue(PropOffsets_Pawn.bWasSpeaking); }
	std::string& lastPhoneme() { return Value<std::string>(PropOffsets_Pawn.lastPhoneme); }
	std::string& nextPhoneme() { return Value<std::string>(PropOffsets_Pawn.nextPhoneme); }
	FixedArrayView<float, 4> animTimer() { return FixedArray<float, 4>(PropOffsets_Pawn.animTimer); }
	BitfieldBool bOnFire() { return BoolValue(PropOffsets_Pawn.bOnFire); }
	float& burnTimer() { return Value<float>(PropOffsets_Pawn.burnTimer); }
	float& AIHorizontalFov() { return Value<float>(PropOffsets_Pawn.AIHorizontalFov); }
	float& AspectRatio() { return Value<float>(PropOffsets_Pawn.AspectRatio); }
	float& AngularResolution() { return Value<float>(PropOffsets_Pawn.AngularResolution); }
	float& MinAngularSize() { return Value<float>(PropOffsets_Pawn.MinAngularSize); }
	float& VisibilityThreshold() { return Value<float>(PropOffsets_Pawn.VisibilityThreshold); }
	float& SmellThreshold() { return Value<float>(PropOffsets_Pawn.SmellThreshold); }
	NameString Alliance() { return Value<NameString>(PropOffsets_Pawn.Alliance); }
	Rotator& AIAddViewRotation() { return Value<Rotator>(PropOffsets_Pawn.AIAddViewRotation); }

	//Unreal 227 exclusive properties
	UActor*& BleedingActor() { return Value<UActor*>(PropOffsets_Pawn.BleedingActor); }
	BitfieldBool bIsBleeding() { return BoolValue(PropOffsets_Pawn.bIsBleeding); }
	float& SightDistanceMulti() { return Value<float>(PropOffsets_Pawn.SightDistanceMulti); }
	EPawnSightCheck SightCheckType() { return static_cast<EPawnSightCheck>(Value<uint8_t>(PropOffsets_Pawn.SightCheckType)); }
	float& SerpentineDist() { return Value<float>(PropOffsets_Pawn.SerpentineDist); }
	float& SerpentineTime() { return Value<float>(PropOffsets_Pawn.SerpentineTime); }
	vec3& MovementStart() { return Value<vec3>(PropOffsets_Pawn.MovementStart); }
	BitfieldBool bIsCrawler() { return BoolValue(PropOffsets_Pawn.bIsCrawler); }
	float& HuntOffDistance() { return Value<float>(PropOffsets_Pawn.HuntOffDistance); }
	BitfieldBool bDoAutoSerpentine() { return BoolValue(PropOffsets_Pawn.bDoAutoSerpentine); }

private:
	bool IsInPathSpecialHandling = false;
};
