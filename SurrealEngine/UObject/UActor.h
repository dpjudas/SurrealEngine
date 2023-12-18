#pragma once

#include "UObject.h"

class UTexture;
class UMesh;
class UModel;
class USound;
class UPawn;
class UInventory;
class ULevelInfo;
class UAnimation;
class UPlayer;
class UMusic;
class UGameReplicationInfo;
class UPlayerReplicationInfo;
class UMutator;
class UMenu;
class UPlayerPawn;
class UStatLog;
class USkyZoneInfo;
class ULevelSummary;
class ULevel;
class UInventorySpot;
class UMover;
class UTrigger;
class UWarpZoneInfo;
class UZoneInfo;
class PackageManager;
class CollisionHit;
class BspNode;
struct MeshAnimSeq;

struct PointRegion
{
	UZoneInfo* Zone;
	int BspLeaf;
	uint8_t ZoneNumber;
};

enum EPhysics
{
	PHYS_None,
	PHYS_Walking,
	PHYS_Falling,
	PHYS_Swimming,
	PHYS_Flying,
	PHYS_Rotating,
	PHYS_Projectile,
	PHYS_Rolling,
	PHYS_Interpolating,
	PHYS_MovingBrush,
	PHYS_Spider,
	PHYS_Trailer
};

enum ENetRole
{
	ROLE_None,
	ROLE_DumbProxy,
	ROLE_SimulatedProxy,
	ROLE_AutonomousProxy,
	ROLE_Authority,
};

enum ESoundSlot
{
	SLOT_None,
	SLOT_Misc,
	SLOT_Pain,
	SLOT_Interact,
	SLOT_Ambient,
	SLOT_Talk,
	SLOT_Interface
};

enum EMusicTransition
{
	MTRAN_None,
	MTRAN_Instant,
	MTRAN_Segue,
	MTRAN_Fade,
	MTRAN_FastFade,
	MTRAN_SlowFade
};

enum ENetMode
{
	NM_Standalone,
	NM_DedicatedServer,
	NM_ListenServer,
	NM_Client
};

enum ELightType
{
	LT_None,
	LT_Steady,
	LT_Pulse,
	LT_Blink,
	LT_Flicker,
	LT_Strobe,
	LT_BackdropLight,
	LT_SubtlePulse,
	LT_TexturePaletteOnce,
	LT_TexturePaletteLoop
};

enum ELightEffect
{
	LE_None,
	LE_TorchWaver,
	LE_FireWaver,
	LE_WateryShimmer,
	LE_Searchlight,
	LE_SlowWave,
	LE_FastWave,
	LE_CloudCast,
	LE_StaticSpot,
	LE_Shock,
	LE_Disco,
	LE_Warp,
	LE_Spotlight,
	LE_NonIncidence,
	LE_Shell,
	LE_OmniBumpMap,
	LE_Interference,
	LE_Cylinder,
	LE_Rotor,
	LE_Unused
};

enum EDrawType
{
	DT_None,
	DT_Sprite,
	DT_Mesh,
	DT_Brush,
	DT_RopeSprite,
	DT_VerticalSprite,
	DT_Terraform,
	DT_SpriteAnimOnce
};

class UActor : public UObject
{
public:
	using UObject::UObject;

	UActor* Spawn(UClass* SpawnClass, UActor* SpawnOwner, NameString SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation);
	bool Destroy();
	void InitBase();

	void SetBase(UActor* newBase, bool sendBaseChangeEvent);
	void SetOwner(UActor* newOwner);
	virtual void InitActorZone();
	virtual void UpdateActorZone();
	PointRegion FindRegion(const vec3& offset = vec3(0.0f));

	virtual void Tick(float elapsed, bool tickedFlag);

	void TickAnimation(float elapsed);

	void TickPhysics(float elapsed);
	void TickWalking(float elapsed);
	void TickFalling(float elapsed);
	void TickSwimming(float elapsed);
	void TickFlying(float elapsed);
	void TickProjectile(float elapsed);
	void TickRolling(float elapsed);
	void TickInterpolating(float elapsed);
	void TickMovingBrush(float elapsed);
	void TickSpider(float elapsed);
	void TickTrailer(float elapsed);

	void PhysLanded(const vec3& hitNormal);

	virtual void TickRotating(float elapsed);

	void SetPhysics(uint8_t newPhysics);
	void SetCollision(bool newColActors, bool newBlockActors, bool newBlockPlayers);

	std::pair<bool, vec3> CheckLocation(vec3 location, float radius, float height, bool check);

	bool SetLocation(const vec3& newLocation);
	bool SetRotation(const Rotator& newRotation);
	bool SetCollisionSize(float newRadius, float newHeight);

	UObject* Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent);
	bool FastTrace(const vec3& traceEnd, const vec3& traceStart);

	CollisionHit TryMove(const vec3& delta);
	CollisionHit TryMoveSmooth(const vec3& delta);
	bool Move(const vec3& delta);
	bool MoveSmooth(const vec3& delta);

	bool IsBasedOn(UActor* other);
	bool IsOwnedBy(UActor* owner);
	bool IsOverlapping(UActor* other);

	void Touch(UActor* actor);
	void UnTouch(UActor* actor);
	static const int TouchingArraySize = 4;

	bool HasAnim(const NameString& sequence);
	bool IsAnimating();
	void FinishAnim();
	NameString GetAnimGroup(const NameString& sequence);
	void PlayAnim(const NameString& sequence, float rate, float tweenTime);
	void LoopAnim(const NameString& sequence, float rate, float tweenTime, float minRate);
	void TweenAnim(const NameString& sequence, float tweenTime);

	void MakeNoise(float loudness);

	void UpdateBspInfo();
	void AddToBspNode(BspNode* node);
	void RemoveFromBspNode();
	static int NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node);

	// The status of the actor in the collision hash
	struct
	{
		bool Inserted = false;
		vec3 Location = { 0.0f };
		float Height = 0.0f;
		float Radius = 0.0f;
	} CollisionHashInfo;

	// Lights touching this actor
	struct
	{
		bool NeedsUpdate = true;
		vec3 Location = vec3(0.0f);
		std::vector<UActor*> LightList;
	} LightInfo;

	// Fog between actor and camera
	struct
	{
		vec3 fogcolor = { 0.0f };
		float brightness = -1.0f;
		float fog = -1.0f;
		float radius = -1.0f;
	} FogInfo;

	// Location in the BSP tree
	struct
	{
		vec3 Location = vec3(0.0f);
		vec3 Extents = vec3(0.0f);
		BspNode* Node = nullptr;
		UActor* Prev = nullptr;
		UActor* Next = nullptr;
	} BspInfo;

	// Tweening animation state
	struct
	{
		int V0 = 0;
		int V1 = 0;
		float T = -1.0f;
	} TweenFromAnimFrame;

	int LastDrawFrame = -1;

	float SleepTimeLeft = 0.0f;
	vec3 gravityVector;

	void SetTweenFromAnimFrame();

	UTexture* GetMultiskin(int index)
	{
		if (index >= 0 && index < 8)
			return (&MultiSkins())[index];
		else
			return nullptr;
	}

	float WorldSoundRadius() { return ((int)SoundRadius() + 1) * 25.0f; }
	float WorldVolumetricRadius() { return ((int)VolumeRadius() + 1) * 25.0f; }
	float WorldLightRadius() { return ((int)LightRadius() + 1) * 25.0f; }

	vec3& Acceleration() { return Value<vec3>(PropOffsets_Actor.Acceleration); }
	uint8_t& AmbientGlow() { return Value<uint8_t>(PropOffsets_Actor.AmbientGlow); }
	USound*& AmbientSound() { return Value<USound*>(PropOffsets_Actor.AmbientSound); }
	float& AnimFrame() { return Value<float>(PropOffsets_Actor.AnimFrame); }
	float& AnimLast() { return Value<float>(PropOffsets_Actor.AnimLast); }
	float& AnimMinRate() { return Value<float>(PropOffsets_Actor.AnimMinRate); }
	float& AnimRate() { return Value<float>(PropOffsets_Actor.AnimRate); }
	NameString& AnimSequence() { return Value<NameString>(PropOffsets_Actor.AnimSequence); }
	NameString& AttachTag() { return Value<NameString>(PropOffsets_Actor.AttachTag); }
	UActor*& ActorBase() { return Value<UActor*>(PropOffsets_Actor.Base); }
	UModel*& Brush() { return Value<UModel*>(PropOffsets_Actor.Brush); }
	float& Buoyancy() { return Value<float>(PropOffsets_Actor.Buoyancy); }
	vec3& ColLocation() { return Value<vec3>(PropOffsets_Actor.ColLocation); }
	float& CollisionHeight() { return Value<float>(PropOffsets_Actor.CollisionHeight); }
	float& CollisionRadius() { return Value<float>(PropOffsets_Actor.CollisionRadius); }
	int& CollisionTag() { return Value<int>(PropOffsets_Actor.CollisionTag); }
	UActor*& Deleted() { return Value<UActor*>(PropOffsets_Actor.Deleted); }
	Rotator& DesiredRotation() { return Value<Rotator>(PropOffsets_Actor.DesiredRotation); }
	uint8_t& DodgeDir() { return Value<uint8_t>(PropOffsets_Actor.DodgeDir); }
	float& DrawScale() { return Value<float>(PropOffsets_Actor.DrawScale); }
	uint8_t& DrawType() { return Value<uint8_t>(PropOffsets_Actor.DrawType); }
	NameString& Event() { return Value<NameString>(PropOffsets_Actor.Event); }
	int& ExtraTag() { return Value<int>(PropOffsets_Actor.ExtraTag); }
	uint8_t& Fatness() { return Value<uint8_t>(PropOffsets_Actor.Fatness); }
	NameString& Group() { return Value<NameString>(PropOffsets_Actor.Group); }
	UActor*& HitActor() { return Value<UActor*>(PropOffsets_Actor.HitActor); }
	NameString& InitialState() { return Value<NameString>(PropOffsets_Actor.InitialState); }
	UPawn*& Instigator() { return Value<UPawn*>(PropOffsets_Actor.Instigator); }
	UInventory*& Inventory() { return Value<UInventory*>(PropOffsets_Actor.Inventory); }
	float& LODBias() { return Value<float>(PropOffsets_Actor.LODBias); }
	UActor*& LatentActor() { return Value<UActor*>(PropOffsets_Actor.LatentActor); }
	uint8_t& LatentByte() { return Value<uint8_t>(PropOffsets_Actor.LatentByte); }
	float& LatentFloat() { return Value<float>(PropOffsets_Actor.LatentFloat); }
	int& LatentInt() { return Value<int>(PropOffsets_Actor.LatentInt); }
	ULevelInfo*& Level() { return Value<ULevelInfo*>(PropOffsets_Actor.Level); }
	float& LifeSpan() { return Value<float>(PropOffsets_Actor.LifeSpan); }
	uint8_t& LightBrightness() { return Value<uint8_t>(PropOffsets_Actor.LightBrightness); }
	uint8_t& LightCone() { return Value<uint8_t>(PropOffsets_Actor.LightCone); }
	uint8_t& LightEffect() { return Value<uint8_t>(PropOffsets_Actor.LightEffect); }
	uint8_t& LightHue() { return Value<uint8_t>(PropOffsets_Actor.LightHue); }
	uint8_t& LightPeriod() { return Value<uint8_t>(PropOffsets_Actor.LightPeriod); }
	uint8_t& LightPhase() { return Value<uint8_t>(PropOffsets_Actor.LightPhase); }
	uint8_t& LightRadius() { return Value<uint8_t>(PropOffsets_Actor.LightRadius); }
	uint8_t& LightSaturation() { return Value<uint8_t>(PropOffsets_Actor.LightSaturation); }
	uint8_t& LightType() { return Value<uint8_t>(PropOffsets_Actor.LightType); }
	int& LightingTag() { return Value<int>(PropOffsets_Actor.LightingTag); }
	vec3& Location() { return Value<vec3>(PropOffsets_Actor.Location); }
	float& Mass() { return Value<float>(PropOffsets_Actor.Mass); }
	UMesh*& Mesh() { return Value<UMesh*>(PropOffsets_Actor.Mesh); }
	uint8_t& MiscNumber() { return Value<uint8_t>(PropOffsets_Actor.MiscNumber); }
	UTexture*& MultiSkins() { return Value<UTexture*>(PropOffsets_Actor.MultiSkins); }
	float& NetPriority() { return Value<float>(PropOffsets_Actor.NetPriority); }
	int& NetTag() { return Value<int>(PropOffsets_Actor.NetTag); }
	float& NetUpdateFrequency() { return Value<float>(PropOffsets_Actor.NetUpdateFrequency); }
	float& OddsOfAppearing() { return Value<float>(PropOffsets_Actor.OddsOfAppearing); }
	float& OldAnimRate() { return Value<float>(PropOffsets_Actor.OldAnimRate); }
	vec3& OldLocation() { return Value<vec3>(PropOffsets_Actor.OldLocation); }
	int& OtherTag() { return Value<int>(PropOffsets_Actor.OtherTag); }
	UActor*& Owner() { return Value<UActor*>(PropOffsets_Actor.Owner); }
	UActor*& PendingTouch() { return Value<UActor*>(PropOffsets_Actor.PendingTouch); }
	float& PhysAlpha() { return Value<float>(PropOffsets_Actor.PhysAlpha); }
	float& PhysRate() { return Value<float>(PropOffsets_Actor.PhysRate); }
	uint8_t& Physics() { return Value<uint8_t>(PropOffsets_Actor.Physics); }
	vec3& PrePivot() { return Value<vec3>(PropOffsets_Actor.PrePivot); }
	PointRegion& Region() { return Value<PointRegion>(PropOffsets_Actor.Region); }
	uint8_t& RemoteRole() { return Value<uint8_t>(PropOffsets_Actor.RemoteRole); }
	//URenderIterator*& RenderInterface() { return Value<URenderIterator*>(PropOffsets_Actor.RenderInterface); }
	UClass*& RenderIteratorClass() { return Value<UClass*>(PropOffsets_Actor.RenderIteratorClass); }
	uint8_t& Role() { return Value<uint8_t>(PropOffsets_Actor.Role); }
	Rotator& Rotation() { return Value<Rotator>(PropOffsets_Actor.Rotation); }
	Rotator& RotationRate() { return Value<Rotator>(PropOffsets_Actor.RotationRate); }
	float& ScaleGlow() { return Value<float>(PropOffsets_Actor.ScaleGlow); }
	//Plane& SimAnim() { return Value<Plane>(PropOffsets_Actor.SimAnim); }
	UAnimation*& SkelAnim() { return Value<UAnimation*>(PropOffsets_Actor.SkelAnim); }
	UTexture*& Skin() { return Value<UTexture*>(PropOffsets_Actor.Skin); }
	uint8_t& SoundPitch() { return Value<uint8_t>(PropOffsets_Actor.SoundPitch); }
	uint8_t& SoundRadius() { return Value<uint8_t>(PropOffsets_Actor.SoundRadius); }
	uint8_t& SoundVolume() { return Value<uint8_t>(PropOffsets_Actor.SoundVolume); }
	int& SpecialTag() { return Value<int>(PropOffsets_Actor.SpecialTag); }
	UTexture*& Sprite() { return Value<UTexture*>(PropOffsets_Actor.Sprite); }
	float& SpriteProjForward() { return Value<float>(PropOffsets_Actor.SpriteProjForward); }
	uint8_t& StandingCount() { return Value<uint8_t>(PropOffsets_Actor.StandingCount); }
	uint8_t& Style() { return Value<uint8_t>(PropOffsets_Actor.Style); }
	NameString& Tag() { return Value<NameString>(PropOffsets_Actor.Tag); }
	UActor*& Target() { return Value<UActor*>(PropOffsets_Actor.Target); }
	UTexture*& Texture() { return Value<UTexture*>(PropOffsets_Actor.Texture); }
	float& TimerCounter() { return Value<float>(PropOffsets_Actor.TimerCounter); }
	float& TimerRate() { return Value<float>(PropOffsets_Actor.TimerRate); }
	UActor** Touching() { return FixedArray<UActor*>(PropOffsets_Actor.Touching); }
	float& TransientSoundRadius() { return Value<float>(PropOffsets_Actor.TransientSoundRadius); }
	float& TransientSoundVolume() { return Value<float>(PropOffsets_Actor.TransientSoundVolume); }
	float& TweenRate() { return Value<float>(PropOffsets_Actor.TweenRate); }
	vec3& Velocity() { return Value<vec3>(PropOffsets_Actor.Velocity); }
	float& VisibilityHeight() { return Value<float>(PropOffsets_Actor.VisibilityHeight); }
	float& VisibilityRadius() { return Value<float>(PropOffsets_Actor.VisibilityRadius); }
	uint8_t& VolumeBrightness() { return Value<uint8_t>(PropOffsets_Actor.VolumeBrightness); }
	uint8_t& VolumeFog() { return Value<uint8_t>(PropOffsets_Actor.VolumeFog); }
	uint8_t& VolumeRadius() { return Value<uint8_t>(PropOffsets_Actor.VolumeRadius); }
	ULevel*& XLevel() { return Value<ULevel*>(PropOffsets_Actor.XLevel); }
	BitfieldBool bActorShadows() { return BoolValue(PropOffsets_Actor.bActorShadows); }
	BitfieldBool bAlwaysRelevant() { return BoolValue(PropOffsets_Actor.bAlwaysRelevant); }
	BitfieldBool bAlwaysTick() { return BoolValue(PropOffsets_Actor.bAlwaysTick); }
	BitfieldBool bAnimByOwner() { return BoolValue(PropOffsets_Actor.bAnimByOwner); }
	BitfieldBool bAnimFinished() { return BoolValue(PropOffsets_Actor.bAnimFinished); }
	BitfieldBool bAnimLoop() { return BoolValue(PropOffsets_Actor.bAnimLoop); }
	BitfieldBool bAnimNotify() { return BoolValue(PropOffsets_Actor.bAnimNotify); }
	BitfieldBool bAssimilated() { return BoolValue(PropOffsets_Actor.bAssimilated); }
	BitfieldBool bBlockActors() { return BoolValue(PropOffsets_Actor.bBlockActors); }
	BitfieldBool bBlockPlayers() { return BoolValue(PropOffsets_Actor.bBlockPlayers); }
	BitfieldBool bBounce() { return BoolValue(PropOffsets_Actor.bBounce); }
	BitfieldBool bCanTeleport() { return BoolValue(PropOffsets_Actor.bCanTeleport); }
	BitfieldBool bCarriedItem() { return BoolValue(PropOffsets_Actor.bCarriedItem); }
	BitfieldBool bClientAnim() { return BoolValue(PropOffsets_Actor.bClientAnim); }
	BitfieldBool bClientDemoNetFunc() { return BoolValue(PropOffsets_Actor.bClientDemoNetFunc); }
	BitfieldBool bClientDemoRecording() { return BoolValue(PropOffsets_Actor.bClientDemoRecording); }
	BitfieldBool bCollideActors() { return BoolValue(PropOffsets_Actor.bCollideActors); }
	BitfieldBool bCollideWhenPlacing() { return BoolValue(PropOffsets_Actor.bCollideWhenPlacing); }
	BitfieldBool bCollideWorld() { return BoolValue(PropOffsets_Actor.bCollideWorld); }
	BitfieldBool bCorona() { return BoolValue(PropOffsets_Actor.bCorona); }
	BitfieldBool bDeleteMe() { return BoolValue(PropOffsets_Actor.bDeleteMe); }
	BitfieldBool bDemoRecording() { return BoolValue(PropOffsets_Actor.bDemoRecording); }
	BitfieldBool bDifficulty0() { return BoolValue(PropOffsets_Actor.bDifficulty0); }
	BitfieldBool bDifficulty1() { return BoolValue(PropOffsets_Actor.bDifficulty1); }
	BitfieldBool bDifficulty2() { return BoolValue(PropOffsets_Actor.bDifficulty2); }
	BitfieldBool bDifficulty3() { return BoolValue(PropOffsets_Actor.bDifficulty3); }
	BitfieldBool bDirectional() { return BoolValue(PropOffsets_Actor.bDirectional); }
	BitfieldBool bDynamicLight() { return BoolValue(PropOffsets_Actor.bDynamicLight); }
	BitfieldBool bEdLocked() { return BoolValue(PropOffsets_Actor.bEdLocked); }
	BitfieldBool bEdShouldSnap() { return BoolValue(PropOffsets_Actor.bEdShouldSnap); }
	BitfieldBool bEdSnap() { return BoolValue(PropOffsets_Actor.bEdSnap); }
	BitfieldBool bFilterByVolume() { return BoolValue(PropOffsets_Actor.bFilterByVolume); }
	BitfieldBool bFixedRotationDir() { return BoolValue(PropOffsets_Actor.bFixedRotationDir); }
	BitfieldBool bForcePhysicsUpdate() { return BoolValue(PropOffsets_Actor.bForcePhysicsUpdate); }
	BitfieldBool bForceStasis() { return BoolValue(PropOffsets_Actor.bForceStasis); }
	BitfieldBool bGameRelevant() { return BoolValue(PropOffsets_Actor.bGameRelevant); }
	BitfieldBool bHidden() { return BoolValue(PropOffsets_Actor.bHidden); }
	BitfieldBool bHiddenEd() { return BoolValue(PropOffsets_Actor.bHiddenEd); }
	BitfieldBool bHighDetail() { return BoolValue(PropOffsets_Actor.bHighDetail); }
	BitfieldBool bHighlighted() { return BoolValue(PropOffsets_Actor.bHighlighted); }
	BitfieldBool bHurtEntry() { return BoolValue(PropOffsets_Actor.bHurtEntry); }
	BitfieldBool bInterpolating() { return BoolValue(PropOffsets_Actor.bInterpolating); }
	BitfieldBool bIsItemGoal() { return BoolValue(PropOffsets_Actor.bIsItemGoal); }
	BitfieldBool bIsKillGoal() { return BoolValue(PropOffsets_Actor.bIsKillGoal); }
	BitfieldBool bIsMover() { return BoolValue(PropOffsets_Actor.bIsMover); }
	BitfieldBool bIsPawn() { return BoolValue(PropOffsets_Actor.bIsPawn); }
	BitfieldBool bIsSecretGoal() { return BoolValue(PropOffsets_Actor.bIsSecretGoal); }
	BitfieldBool bJustTeleported() { return BoolValue(PropOffsets_Actor.bJustTeleported); }
	BitfieldBool bLensFlare() { return BoolValue(PropOffsets_Actor.bLensFlare); }
	BitfieldBool bLightChanged() { return BoolValue(PropOffsets_Actor.bLightChanged); }
	BitfieldBool bMemorized() { return BoolValue(PropOffsets_Actor.bMemorized); }
	BitfieldBool bMeshCurvy() { return BoolValue(PropOffsets_Actor.bMeshCurvy); }
	BitfieldBool bMeshEnviroMap() { return BoolValue(PropOffsets_Actor.bMeshEnviroMap); }
	BitfieldBool bMovable() { return BoolValue(PropOffsets_Actor.bMovable); }
	BitfieldBool bNet() { return BoolValue(PropOffsets_Actor.bNet); }
	BitfieldBool bNetFeel() { return BoolValue(PropOffsets_Actor.bNetFeel); }
	BitfieldBool bNetHear() { return BoolValue(PropOffsets_Actor.bNetHear); }
	BitfieldBool bNetInitial() { return BoolValue(PropOffsets_Actor.bNetInitial); }
	BitfieldBool bNetOptional() { return BoolValue(PropOffsets_Actor.bNetOptional); }
	BitfieldBool bNetOwner() { return BoolValue(PropOffsets_Actor.bNetOwner); }
	BitfieldBool bNetRelevant() { return BoolValue(PropOffsets_Actor.bNetRelevant); }
	BitfieldBool bNetSee() { return BoolValue(PropOffsets_Actor.bNetSee); }
	BitfieldBool bNetSpecial() { return BoolValue(PropOffsets_Actor.bNetSpecial); }
	BitfieldBool bNetTemporary() { return BoolValue(PropOffsets_Actor.bNetTemporary); }
	BitfieldBool bNoDelete() { return BoolValue(PropOffsets_Actor.bNoDelete); }
	BitfieldBool bNoSmooth() { return BoolValue(PropOffsets_Actor.bNoSmooth); }
	BitfieldBool bOnlyOwnerSee() { return BoolValue(PropOffsets_Actor.bOnlyOwnerSee); }
	BitfieldBool bOwnerNoSee() { return BoolValue(PropOffsets_Actor.bOwnerNoSee); }
	BitfieldBool bParticles() { return BoolValue(PropOffsets_Actor.bParticles); }
	BitfieldBool bProjTarget() { return BoolValue(PropOffsets_Actor.bProjTarget); }
	BitfieldBool bRandomFrame() { return BoolValue(PropOffsets_Actor.bRandomFrame); }
	BitfieldBool bReplicateInstigator() { return BoolValue(PropOffsets_Actor.bReplicateInstigator); }
	BitfieldBool bRotateToDesired() { return BoolValue(PropOffsets_Actor.bRotateToDesired); }
	BitfieldBool bScriptInitialized() { return BoolValue(PropOffsets_Actor.bScriptInitialized); }
	BitfieldBool bSelected() { return BoolValue(PropOffsets_Actor.bSelected); }
	BitfieldBool bShadowCast() { return BoolValue(PropOffsets_Actor.bShadowCast); }
	BitfieldBool bSimFall() { return BoolValue(PropOffsets_Actor.bSimFall); }
	BitfieldBool bSimulatedPawn() { return BoolValue(PropOffsets_Actor.bSimulatedPawn); }
	BitfieldBool bSinglePlayer() { return BoolValue(PropOffsets_Actor.bSinglePlayer); }
	BitfieldBool bSpecialLit() { return BoolValue(PropOffsets_Actor.bSpecialLit); }
	BitfieldBool bStasis() { return BoolValue(PropOffsets_Actor.bStasis); }
	BitfieldBool bStatic() { return BoolValue(PropOffsets_Actor.bStatic); }
	BitfieldBool bTempEditor() { return BoolValue(PropOffsets_Actor.bTempEditor); }
	BitfieldBool bTicked() { return BoolValue(PropOffsets_Actor.bTicked); }
	BitfieldBool bTimerLoop() { return BoolValue(PropOffsets_Actor.bTimerLoop); }
	BitfieldBool bTrailerPrePivot() { return BoolValue(PropOffsets_Actor.bTrailerPrePivot); }
	BitfieldBool bTrailerSameRotation() { return BoolValue(PropOffsets_Actor.bTrailerSameRotation); }
	BitfieldBool bTravel() { return BoolValue(PropOffsets_Actor.bTravel); }
	BitfieldBool bUnlit() { return BoolValue(PropOffsets_Actor.bUnlit); }
};

class ULight : public UActor
{
public:
	using UActor::UActor;
};

class UDecal : public UActor
{
public:
	using UActor::UActor;

	UObject* AttachDecal(float traceDistance, const vec3& decalDir);
	void DetachDecal();

	float& LastRenderedTime() { return Value<float>(PropOffsets_Decal.LastRenderedTime); }
	int& MultiDecalLevel() { return Value<int>(PropOffsets_Decal.MultiDecalLevel); }
	std::vector<void*>& SurfList() { return Value<std::vector<void*>>(PropOffsets_Decal.SurfList); }
};

class USpawnNotify : public UActor
{
public:
	using UActor::UActor;

	UClass*& ActorClass() { return Value<UClass*>(PropOffsets_SpawnNotify.ActorClass); }
	USpawnNotify*& Next() { return Value<USpawnNotify*>(PropOffsets_SpawnNotify.Next); }
};

class UInventory : public UActor
{
public:
	using UActor::UActor;

	int& AbsorptionPriority() { return Value<int>(PropOffsets_Inventory.AbsorptionPriority); }
	USound*& ActivateSound() { return Value<USound*>(PropOffsets_Inventory.ActivateSound); }
	int& ArmorAbsorption() { return Value<int>(PropOffsets_Inventory.ArmorAbsorption); }
	uint8_t& AutoSwitchPriority() { return Value<uint8_t>(PropOffsets_Inventory.AutoSwitchPriority); }
	float& BobDamping() { return Value<float>(PropOffsets_Inventory.BobDamping); }
	int& Charge() { return Value<int>(PropOffsets_Inventory.Charge); }
	USound*& DeActivateSound() { return Value<USound*>(PropOffsets_Inventory.DeActivateSound); }
	uint8_t& FlashCount() { return Value<uint8_t>(PropOffsets_Inventory.FlashCount); }
	UTexture*& Icon() { return Value<UTexture*>(PropOffsets_Inventory.Icon); }
	uint8_t& InventoryGroup() { return Value<uint8_t>(PropOffsets_Inventory.InventoryGroup); }
	std::string& ItemArticle() { return Value<std::string>(PropOffsets_Inventory.ItemArticle); }
	UClass*& ItemMessageClass() { return Value<UClass*>(PropOffsets_Inventory.ItemMessageClass); }
	std::string& ItemName() { return Value<std::string>(PropOffsets_Inventory.ItemName); }
	std::string& M_Activated() { return Value<std::string>(PropOffsets_Inventory.M_Activated); }
	std::string& M_Deactivated() { return Value<std::string>(PropOffsets_Inventory.M_Deactivated); }
	std::string& M_Selected() { return Value<std::string>(PropOffsets_Inventory.M_Selected); }
	float& MaxDesireability() { return Value<float>(PropOffsets_Inventory.MaxDesireability); }
	UMesh*& MuzzleFlashMesh() { return Value<UMesh*>(PropOffsets_Inventory.MuzzleFlashMesh); }
	float& MuzzleFlashScale() { return Value<float>(PropOffsets_Inventory.MuzzleFlashScale); }
	uint8_t& MuzzleFlashStyle() { return Value<uint8_t>(PropOffsets_Inventory.MuzzleFlashStyle); }
	UTexture*& MuzzleFlashTexture() { return Value<UTexture*>(PropOffsets_Inventory.MuzzleFlashTexture); }
	UInventory*& NextArmor() { return Value<UInventory*>(PropOffsets_Inventory.NextArmor); }
	uint8_t& OldFlashCount() { return Value<uint8_t>(PropOffsets_Inventory.OldFlashCount); }
	std::string& PickupMessage() { return Value<std::string>(PropOffsets_Inventory.PickupMessage); }
	UClass*& PickupMessageClass() { return Value<UClass*>(PropOffsets_Inventory.PickupMessageClass); }
	USound*& PickupSound() { return Value<USound*>(PropOffsets_Inventory.PickupSound); }
	UMesh*& PickupViewMesh() { return Value<UMesh*>(PropOffsets_Inventory.PickupViewMesh); }
	float& PickupViewScale() { return Value<float>(PropOffsets_Inventory.PickupViewScale); }
	NameString& PlayerLastTouched() { return Value<NameString>(PropOffsets_Inventory.PlayerLastTouched); }
	UMesh*& PlayerViewMesh() { return Value<UMesh*>(PropOffsets_Inventory.PlayerViewMesh); }
	vec3& PlayerViewOffset() { return Value<vec3>(PropOffsets_Inventory.PlayerViewOffset); }
	float& PlayerViewScale() { return Value<float>(PropOffsets_Inventory.PlayerViewScale); }
	NameString& ProtectionType1() { return Value<NameString>(PropOffsets_Inventory.ProtectionType1); }
	NameString& ProtectionType2() { return Value<NameString>(PropOffsets_Inventory.ProtectionType2); }
	USound*& RespawnSound() { return Value<USound*>(PropOffsets_Inventory.RespawnSound); }
	float& RespawnTime() { return Value<float>(PropOffsets_Inventory.RespawnTime); }
	UTexture*& StatusIcon() { return Value<UTexture*>(PropOffsets_Inventory.StatusIcon); }
	UMesh*& ThirdPersonMesh() { return Value<UMesh*>(PropOffsets_Inventory.ThirdPersonMesh); }
	float& ThirdPersonScale() { return Value<float>(PropOffsets_Inventory.ThirdPersonScale); }
	BitfieldBool bActivatable() { return BoolValue(PropOffsets_Inventory.bActivatable); }
	BitfieldBool bActive() { return BoolValue(PropOffsets_Inventory.bActive); }
	BitfieldBool bAmbientGlow() { return BoolValue(PropOffsets_Inventory.bAmbientGlow); }
	BitfieldBool bDisplayableInv() { return BoolValue(PropOffsets_Inventory.bDisplayableInv); }
	BitfieldBool bFirstFrame() { return BoolValue(PropOffsets_Inventory.bFirstFrame); }
	BitfieldBool bHeldItem() { return BoolValue(PropOffsets_Inventory.bHeldItem); }
	BitfieldBool bInstantRespawn() { return BoolValue(PropOffsets_Inventory.bInstantRespawn); }
	BitfieldBool bIsAnArmor() { return BoolValue(PropOffsets_Inventory.bIsAnArmor); }
	BitfieldBool bMuzzleFlashParticles() { return BoolValue(PropOffsets_Inventory.bMuzzleFlashParticles); }
	BitfieldBool bRotatingPickup() { return BoolValue(PropOffsets_Inventory.bRotatingPickup); }
	BitfieldBool bSleepTouch() { return BoolValue(PropOffsets_Inventory.bSleepTouch); }
	BitfieldBool bSteadyFlash3rd() { return BoolValue(PropOffsets_Inventory.bSteadyFlash3rd); }
	BitfieldBool bSteadyToggle() { return BoolValue(PropOffsets_Inventory.bSteadyToggle); }
	BitfieldBool bToggleSteadyFlash() { return BoolValue(PropOffsets_Inventory.bToggleSteadyFlash); }
	BitfieldBool bTossedOut() { return BoolValue(PropOffsets_Inventory.bTossedOut); }
	UInventorySpot*& myMarker() { return Value<UInventorySpot*>(PropOffsets_Inventory.myMarker); }
};

class UWeapon : public UInventory
{
public:
	using UInventory::UInventory;

	float& AIRating() { return Value<float>(PropOffsets_Weapon.AIRating); }
	Rotator& AdjustedAim() { return Value<Rotator>(PropOffsets_Weapon.AdjustedAim); }
	NameString& AltDamageType() { return Value<NameString>(PropOffsets_Weapon.AltDamageType); }
	USound*& AltFireSound() { return Value<USound*>(PropOffsets_Weapon.AltFireSound); }
	UClass*& AltProjectileClass() { return Value<UClass*>(PropOffsets_Weapon.AltProjectileClass); }
	float& AltProjectileSpeed() { return Value<float>(PropOffsets_Weapon.AltProjectileSpeed); }
	float& AltRefireRate() { return Value<float>(PropOffsets_Weapon.AltRefireRate); }
	UClass*& AmmoName() { return Value<UClass*>(PropOffsets_Weapon.AmmoName); }
	//UAmmo*& AmmoType() { return Value<UAmmo*>(PropOffsets_Weapon.AmmoType); }
	USound*& CockingSound() { return Value<USound*>(PropOffsets_Weapon.CockingSound); }
	std::string& DeathMessage() { return Value<std::string>(PropOffsets_Weapon.DeathMessage); }
	vec3& FireOffset() { return Value<vec3>(PropOffsets_Weapon.FireOffset); }
	USound*& FireSound() { return Value<USound*>(PropOffsets_Weapon.FireSound); }
	float& FiringSpeed() { return Value<float>(PropOffsets_Weapon.FiringSpeed); }
	float& FlareOffset() { return Value<float>(PropOffsets_Weapon.FlareOffset); }
	float& FlashC() { return Value<float>(PropOffsets_Weapon.FlashC); }
	float& FlashLength() { return Value<float>(PropOffsets_Weapon.FlashLength); }
	float& FlashO() { return Value<float>(PropOffsets_Weapon.FlashO); }
	int& FlashS() { return Value<int>(PropOffsets_Weapon.FlashS); }
	float& FlashTime() { return Value<float>(PropOffsets_Weapon.FlashTime); }
	float& FlashY() { return Value<float>(PropOffsets_Weapon.FlashY); }
	UTexture*& MFTexture() { return Value<UTexture*>(PropOffsets_Weapon.MFTexture); }
	float& MaxTargetRange() { return Value<float>(PropOffsets_Weapon.MaxTargetRange); }
	std::string& MessageNoAmmo() { return Value<std::string>(PropOffsets_Weapon.MessageNoAmmo); }
	USound*& Misc1Sound() { return Value<USound*>(PropOffsets_Weapon.Misc1Sound); }
	USound*& Misc2Sound() { return Value<USound*>(PropOffsets_Weapon.Misc2Sound); }
	USound*& Misc3Sound() { return Value<USound*>(PropOffsets_Weapon.Misc3Sound); }
	UTexture*& MuzzleFlare() { return Value<UTexture*>(PropOffsets_Weapon.MuzzleFlare); }
	float& MuzzleScale() { return Value<float>(PropOffsets_Weapon.MuzzleScale); }
	NameString& MyDamageType() { return Value<NameString>(PropOffsets_Weapon.MyDamageType); }
	Color& NameColor() { return Value<Color>(PropOffsets_Weapon.NameColor); }
	int& PickupAmmoCount() { return Value<int>(PropOffsets_Weapon.PickupAmmoCount); }
	UClass*& ProjectileClass() { return Value<UClass*>(PropOffsets_Weapon.ProjectileClass); }
	float& ProjectileSpeed() { return Value<float>(PropOffsets_Weapon.ProjectileSpeed); }
	float& RefireRate() { return Value<float>(PropOffsets_Weapon.RefireRate); }
	uint8_t& ReloadCount() { return Value<uint8_t>(PropOffsets_Weapon.ReloadCount); }
	USound*& SelectSound() { return Value<USound*>(PropOffsets_Weapon.SelectSound); }
	float& aimerror() { return Value<float>(PropOffsets_Weapon.aimerror); }
	BitfieldBool bAltInstantHit() { return BoolValue(PropOffsets_Weapon.bAltInstantHit); }
	BitfieldBool bAltWarnTarget() { return BoolValue(PropOffsets_Weapon.bAltWarnTarget); }
	BitfieldBool bCanThrow() { return BoolValue(PropOffsets_Weapon.bCanThrow); }
	BitfieldBool bChangeWeapon() { return BoolValue(PropOffsets_Weapon.bChangeWeapon); }
	BitfieldBool bDrawMuzzleFlash() { return BoolValue(PropOffsets_Weapon.bDrawMuzzleFlash); }
	BitfieldBool bHideWeapon() { return BoolValue(PropOffsets_Weapon.bHideWeapon); }
	BitfieldBool bInstantHit() { return BoolValue(PropOffsets_Weapon.bInstantHit); }
	BitfieldBool bLockedOn() { return BoolValue(PropOffsets_Weapon.bLockedOn); }
	BitfieldBool bMeleeWeapon() { return BoolValue(PropOffsets_Weapon.bMeleeWeapon); }
	uint8_t& bMuzzleFlash() { return Value<uint8_t>(PropOffsets_Weapon.bMuzzleFlash); }
	BitfieldBool bOwnsCrosshair() { return BoolValue(PropOffsets_Weapon.bOwnsCrosshair); }
	BitfieldBool bPointing() { return BoolValue(PropOffsets_Weapon.bPointing); }
	BitfieldBool bRapidFire() { return BoolValue(PropOffsets_Weapon.bRapidFire); }
	BitfieldBool bRecommendAltSplashDamage() { return BoolValue(PropOffsets_Weapon.bRecommendAltSplashDamage); }
	BitfieldBool bRecommendSplashDamage() { return BoolValue(PropOffsets_Weapon.bRecommendSplashDamage); }
	BitfieldBool bSetFlashTime() { return BoolValue(PropOffsets_Weapon.bSetFlashTime); }
	BitfieldBool bSpecialIcon() { return BoolValue(PropOffsets_Weapon.bSpecialIcon); }
	BitfieldBool bSplashDamage() { return BoolValue(PropOffsets_Weapon.bSplashDamage); }
	BitfieldBool bWarnTarget() { return BoolValue(PropOffsets_Weapon.bWarnTarget); }
	BitfieldBool bWeaponStay() { return BoolValue(PropOffsets_Weapon.bWeaponStay); }
	BitfieldBool bWeaponUp() { return BoolValue(PropOffsets_Weapon.bWeaponUp); }
	float& shakemag() { return Value<float>(PropOffsets_Weapon.shakemag); }
	float& shaketime() { return Value<float>(PropOffsets_Weapon.shaketime); }
	float& shakevert() { return Value<float>(PropOffsets_Weapon.shakevert); }
};

class UNavigationPoint : public UActor
{
public:
	using UActor::UActor;

	int& ExtraCost() { return Value<int>(PropOffsets_NavigationPoint.ExtraCost); }
	int& Paths() { return Value<int>(PropOffsets_NavigationPoint.Paths); }
	int& PrunedPaths() { return Value<int>(PropOffsets_NavigationPoint.PrunedPaths); }
	UActor*& RouteCache() { return Value<UActor*>(PropOffsets_NavigationPoint.RouteCache); }
	UNavigationPoint*& VisNoReachPaths() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.VisNoReachPaths); }
	BitfieldBool bAutoBuilt() { return BoolValue(PropOffsets_NavigationPoint.bAutoBuilt); }
	BitfieldBool bEndPoint() { return BoolValue(PropOffsets_NavigationPoint.bEndPoint); }
	BitfieldBool bEndPointOnly() { return BoolValue(PropOffsets_NavigationPoint.bEndPointOnly); }
	BitfieldBool bNeverUseStrafing() { return BoolValue(PropOffsets_NavigationPoint.bNeverUseStrafing); }
	BitfieldBool bOneWayPath() { return BoolValue(PropOffsets_NavigationPoint.bOneWayPath); }
	BitfieldBool bPlayerOnly() { return BoolValue(PropOffsets_NavigationPoint.bPlayerOnly); }
	BitfieldBool bSpecialCost() { return BoolValue(PropOffsets_NavigationPoint.bSpecialCost); }
	BitfieldBool bTwoWay() { return BoolValue(PropOffsets_NavigationPoint.bTwoWay); }
	int& bestPathWeight() { return Value<int>(PropOffsets_NavigationPoint.bestPathWeight); }
	int& cost() { return Value<int>(PropOffsets_NavigationPoint.cost); }
	UNavigationPoint*& nextNavigationPoint() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.nextNavigationPoint); }
	UNavigationPoint*& nextOrdered() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.nextOrdered); }
	NameString& ownerTeam() { return Value<NameString>(PropOffsets_NavigationPoint.ownerTeam); }
	UNavigationPoint*& prevOrdered() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.prevOrdered); }
	UNavigationPoint*& previousPath() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.previousPath); }
	UNavigationPoint*& startPath() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.startPath); }
	BitfieldBool taken() { return BoolValue(PropOffsets_NavigationPoint.taken); }
	int& upstreamPaths() { return Value<int>(PropOffsets_NavigationPoint.upstreamPaths); }
	int& visitedWeight() { return Value<int>(PropOffsets_NavigationPoint.visitedWeight); }
};

class ULiftExit : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	float& LastTriggerTime() { return Value<float>(PropOffsets_LiftExit.LastTriggerTime); }
	NameString& LiftTag() { return Value<NameString>(PropOffsets_LiftExit.LiftTag); }
	NameString& LiftTrigger() { return Value<NameString>(PropOffsets_LiftExit.LiftTrigger); }
	UMover*& MyLift() { return Value<UMover*>(PropOffsets_LiftExit.MyLift); }
	UTrigger*& RecommendedTrigger() { return Value<UTrigger*>(PropOffsets_LiftExit.RecommendedTrigger); }
};

class ULiftCenter : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	float& LastTriggerTime() { return Value<float>(PropOffsets_LiftCenter.LastTriggerTime); }
	vec3& LiftOffset() { return Value<vec3>(PropOffsets_LiftCenter.LiftOffset); }
	NameString& LiftTag() { return Value<NameString>(PropOffsets_LiftCenter.LiftTag); }
	NameString& LiftTrigger() { return Value<NameString>(PropOffsets_LiftCenter.LiftTrigger); }
	float& MaxDist2D() { return Value<float>(PropOffsets_LiftCenter.MaxDist2D); }
	float& MaxZDiffAdd() { return Value<float>(PropOffsets_LiftCenter.MaxZDiffAdd); }
	UMover*& MyLift() { return Value<UMover*>(PropOffsets_LiftCenter.MyLift); }
	UTrigger*& RecommendedTrigger() { return Value<UTrigger*>(PropOffsets_LiftCenter.RecommendedTrigger); }
};

class UWarpZoneMarker : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_WarpZoneMarker.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_WarpZoneMarker.TriggerActor2); }
	UWarpZoneInfo*& markedWarpZone() { return Value<UWarpZoneInfo*>(PropOffsets_WarpZoneMarker.markedWarpZone); }
};

class UInventorySpot : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	UInventory*& markedItem() { return Value<UInventory*>(PropOffsets_InventorySpot.markedItem); }
};

class UTriggerMarker : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;
};

class UButtonMarker : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;
};

class UPlayerStart : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	uint8_t& TeamNumber() { return Value<uint8_t>(PropOffsets_PlayerStart.TeamNumber); }
	BitfieldBool bCoopStart() { return BoolValue(PropOffsets_PlayerStart.bCoopStart); }
	BitfieldBool bEnabled() { return BoolValue(PropOffsets_PlayerStart.bEnabled); }
	BitfieldBool bSinglePlayerStart() { return BoolValue(PropOffsets_PlayerStart.bSinglePlayerStart); }
};

class UTeleporter : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	float& LastFired() { return Value<float>(PropOffsets_Teleporter.LastFired); }
	NameString& ProductRequired() { return Value<NameString>(PropOffsets_Teleporter.ProductRequired); }
	vec3& TargetVelocity() { return Value<vec3>(PropOffsets_Teleporter.TargetVelocity); }
	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_Teleporter.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_Teleporter.TriggerActor2); }
	std::string& URL() { return Value<std::string>(PropOffsets_Teleporter.URL); }
	BitfieldBool bChangesVelocity() { return BoolValue(PropOffsets_Teleporter.bChangesVelocity); }
	BitfieldBool bChangesYaw() { return BoolValue(PropOffsets_Teleporter.bChangesYaw); }
	BitfieldBool bEnabled() { return BoolValue(PropOffsets_Teleporter.bEnabled); }
	BitfieldBool bReversesX() { return BoolValue(PropOffsets_Teleporter.bReversesX); }
	BitfieldBool bReversesY() { return BoolValue(PropOffsets_Teleporter.bReversesY); }
	BitfieldBool bReversesZ() { return BoolValue(PropOffsets_Teleporter.bReversesZ); }
};

class UPathNode : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;
};

class UDecoration : public UActor
{
public:
	using UActor::UActor;

	UClass*& EffectWhenDestroyed() { return Value<UClass*>(PropOffsets_Decoration.EffectWhenDestroyed); }
	USound*& EndPushSound() { return Value<USound*>(PropOffsets_Decoration.EndPushSound); }
	USound*& PushSound() { return Value<USound*>(PropOffsets_Decoration.PushSound); }
	BitfieldBool bBobbing() { return BoolValue(PropOffsets_Decoration.bBobbing); }
	BitfieldBool bOnlyTriggerable() { return BoolValue(PropOffsets_Decoration.bOnlyTriggerable); }
	BitfieldBool bPushSoundPlaying() { return BoolValue(PropOffsets_Decoration.bPushSoundPlaying); }
	BitfieldBool bPushable() { return BoolValue(PropOffsets_Decoration.bPushable); }
	BitfieldBool bSplash() { return BoolValue(PropOffsets_Decoration.bSplash); }
	BitfieldBool bWasCarried() { return BoolValue(PropOffsets_Decoration.bWasCarried); }
	UClass*& content2() { return Value<UClass*>(PropOffsets_Decoration.content2); }
	UClass*& content3() { return Value<UClass*>(PropOffsets_Decoration.content3); }
	UClass*& contents() { return Value<UClass*>(PropOffsets_Decoration.contents); }
	int& numLandings() { return Value<int>(PropOffsets_Decoration.numLandings); }
};

class UCarcass : public UDecoration
{
public:
	using UDecoration::UDecoration;

	UPawn*& Bugs() { return Value<UPawn*>(PropOffsets_Carcass.Bugs); }
	int& CumulativeDamage() { return Value<int>(PropOffsets_Carcass.CumulativeDamage); }
	UPlayerReplicationInfo*& PlayerOwner() { return Value<UPlayerReplicationInfo*>(PropOffsets_Carcass.PlayerOwner); }
	BitfieldBool bDecorative() { return BoolValue(PropOffsets_Carcass.bDecorative); }
	BitfieldBool bPlayerCarcass() { return BoolValue(PropOffsets_Carcass.bPlayerCarcass); }
	BitfieldBool bReducedHeight() { return BoolValue(PropOffsets_Carcass.bReducedHeight); }
	BitfieldBool bSlidingCarcass() { return BoolValue(PropOffsets_Carcass.bSlidingCarcass); }
	uint8_t& flies() { return Value<uint8_t>(PropOffsets_Carcass.flies); }
	uint8_t& rats() { return Value<uint8_t>(PropOffsets_Carcass.rats); }
};

class UProjectile : public UActor
{
public:
	using UActor::UActor;

	float& Damage() { return Value<float>(PropOffsets_Projectile.Damage); }
	float& ExploWallOut() { return Value<float>(PropOffsets_Projectile.ExploWallOut); }
	UClass*& ExplosionDecal() { return Value<UClass*>(PropOffsets_Projectile.ExplosionDecal); }
	USound*& ImpactSound() { return Value<USound*>(PropOffsets_Projectile.ImpactSound); }
	float& MaxSpeed() { return Value<float>(PropOffsets_Projectile.MaxSpeed); }
	USound*& MiscSound() { return Value<USound*>(PropOffsets_Projectile.MiscSound); }
	int& MomentumTransfer() { return Value<int>(PropOffsets_Projectile.MomentumTransfer); }
	NameString& MyDamageType() { return Value<NameString>(PropOffsets_Projectile.MyDamageType); }
	USound*& SpawnSound() { return Value<USound*>(PropOffsets_Projectile.SpawnSound); }
	float& speed() { return Value<float>(PropOffsets_Projectile.speed); }
};

class UKeypoint : public UActor
{
public:
	using UActor::UActor;
};

class Ulocationid : public UKeypoint
{
public:
	using UKeypoint::UKeypoint;

	std::string& LocationName() { return Value<std::string>(PropOffsets_locationid.LocationName); }
	Ulocationid*& NextLocation() { return Value<Ulocationid*>(PropOffsets_locationid.NextLocation); }
	float& Radius() { return Value<float>(PropOffsets_locationid.Radius); }
};

class UInterpolationPoint : public UKeypoint
{
public:
	using UKeypoint::UKeypoint;

	float& FovModifier() { return Value<float>(PropOffsets_InterpolationPoint.FovModifier); }
	float& GameSpeedModifier() { return Value<float>(PropOffsets_InterpolationPoint.GameSpeedModifier); }
	UInterpolationPoint*& Next() { return Value<UInterpolationPoint*>(PropOffsets_InterpolationPoint.Next); }
	int& Position() { return Value<int>(PropOffsets_InterpolationPoint.Position); }
	UInterpolationPoint*& Prev() { return Value<UInterpolationPoint*>(PropOffsets_InterpolationPoint.Prev); }
	float& RateModifier() { return Value<float>(PropOffsets_InterpolationPoint.RateModifier); }
	vec3& ScreenFlashFog() { return Value<vec3>(PropOffsets_InterpolationPoint.ScreenFlashFog); }
	float& ScreenFlashScale() { return Value<float>(PropOffsets_InterpolationPoint.ScreenFlashScale); }
	BitfieldBool bEndOfPath() { return BoolValue(PropOffsets_InterpolationPoint.bEndOfPath); }
	BitfieldBool bSkipNextPath() { return BoolValue(PropOffsets_InterpolationPoint.bSkipNextPath); }
};

class UTriggers : public UActor
{
public:
	using UActor::UActor;
};

class UTrigger : public UTriggers
{
public:
	using UTriggers::UTriggers;

	UClass*& ClassProximityType() { return Value<UClass*>(PropOffsets_Trigger.ClassProximityType); }
	float& DamageThreshold() { return Value<float>(PropOffsets_Trigger.DamageThreshold); }
	std::string& Message() { return Value<std::string>(PropOffsets_Trigger.Message); }
	float& ReTriggerDelay() { return Value<float>(PropOffsets_Trigger.ReTriggerDelay); }
	float& RepeatTriggerTime() { return Value<float>(PropOffsets_Trigger.RepeatTriggerTime); }
	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_Trigger.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_Trigger.TriggerActor2); }
	float& TriggerTime() { return Value<float>(PropOffsets_Trigger.TriggerTime); }
	uint8_t& TriggerType() { return Value<uint8_t>(PropOffsets_Trigger.TriggerType); }
	BitfieldBool bInitiallyActive() { return BoolValue(PropOffsets_Trigger.bInitiallyActive); }
	BitfieldBool bTriggerOnceOnly() { return BoolValue(PropOffsets_Trigger.bTriggerOnceOnly); }
};

class UHUD : public UActor
{
public:
	using UActor::UActor;

	int& Crosshair() { return Value<int>(PropOffsets_HUD.Crosshair); }
	std::string& HUDConfigWindowType() { return Value<std::string>(PropOffsets_HUD.HUDConfigWindowType); }
	UMutator*& HUDMutator() { return Value<UMutator*>(PropOffsets_HUD.HUDMutator); }
	int& HudMode() { return Value<int>(PropOffsets_HUD.HudMode); }
	UMenu*& MainMenu() { return Value<UMenu*>(PropOffsets_HUD.MainMenu); }
	UClass*& MainMenuType() { return Value<UClass*>(PropOffsets_HUD.MainMenuType); }
	UPlayerPawn*& PlayerOwner() { return Value<UPlayerPawn*>(PropOffsets_HUD.PlayerOwner); }
	Color& WhiteColor() { return Value<Color>(PropOffsets_HUD.WhiteColor); }
};

class UMenu : public UActor
{
public:
	using UActor::UActor;

	std::string& CenterString() { return Value<std::string>(PropOffsets_Menu.CenterString); }
	std::string& DisabledString() { return Value<std::string>(PropOffsets_Menu.DisabledString); }
	std::string& EnabledString() { return Value<std::string>(PropOffsets_Menu.EnabledString); }
	std::string& HelpMessage() { return Value<std::string>(PropOffsets_Menu.HelpMessage); }
	std::string& LeftString() { return Value<std::string>(PropOffsets_Menu.LeftString); }
	int& MenuLength() { return Value<int>(PropOffsets_Menu.MenuLength); }
	std::string& MenuList() { return Value<std::string>(PropOffsets_Menu.MenuList); }
	std::string& MenuTitle() { return Value<std::string>(PropOffsets_Menu.MenuTitle); }
	std::string& NoString() { return Value<std::string>(PropOffsets_Menu.NoString); }
	UMenu*& ParentMenu() { return Value<UMenu*>(PropOffsets_Menu.ParentMenu); }
	UPlayerPawn*& PlayerOwner() { return Value<UPlayerPawn*>(PropOffsets_Menu.PlayerOwner); }
	std::string& RightString() { return Value<std::string>(PropOffsets_Menu.RightString); }
	int& Selection() { return Value<int>(PropOffsets_Menu.Selection); }
	std::string& YesString() { return Value<std::string>(PropOffsets_Menu.YesString); }
	BitfieldBool bConfigChanged() { return BoolValue(PropOffsets_Menu.bConfigChanged); }
	BitfieldBool bExitAllMenus() { return BoolValue(PropOffsets_Menu.bExitAllMenus); }
};

class UInfo : public UActor
{
public:
	using UActor::UActor;
};

class UMutator : public UInfo
{
public:
	using UInfo::UInfo;

	UClass*& DefaultWeapon() { return Value<UClass*>(PropOffsets_Mutator.DefaultWeapon); }
	UMutator*& NextDamageMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextDamageMutator); }
	UMutator*& NextHUDMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextHUDMutator); }
	UMutator*& NextMessageMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextMessageMutator); }
	UMutator*& NextMutator() { return Value<UMutator*>(PropOffsets_Mutator.NextMutator); }
	BitfieldBool bHUDMutator() { return BoolValue(PropOffsets_Mutator.bHUDMutator); }
};

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
};

class USavedMove : public UInfo
{
public:
	using UInfo::UInfo;

	float& Delta() { return Value<float>(PropOffsets_SavedMove.Delta); }
	uint8_t& DodgeMove() { return Value<uint8_t>(PropOffsets_SavedMove.DodgeMove); }
	USavedMove*& NextMove() { return Value<USavedMove*>(PropOffsets_SavedMove.NextMove); }
	float& TimeStamp() { return Value<float>(PropOffsets_SavedMove.TimeStamp); }
	BitfieldBool bAltFire() { return BoolValue(PropOffsets_SavedMove.bAltFire); }
	BitfieldBool bDuck() { return BoolValue(PropOffsets_SavedMove.bDuck); }
	BitfieldBool bFire() { return BoolValue(PropOffsets_SavedMove.bFire); }
	BitfieldBool bForceAltFire() { return BoolValue(PropOffsets_SavedMove.bForceAltFire); }
	BitfieldBool bForceFire() { return BoolValue(PropOffsets_SavedMove.bForceFire); }
	BitfieldBool bPressedJump() { return BoolValue(PropOffsets_SavedMove.bPressedJump); }
	BitfieldBool bRun() { return BoolValue(PropOffsets_SavedMove.bRun); }
};

class UInternetInfo : public UInfo
{
public:
	using UInfo::UInfo;
};

class UZoneInfo : public UInfo
{
public:
	using UInfo::UInfo;

	uint8_t& AmbientBrightness() { return Value<uint8_t>(PropOffsets_ZoneInfo.AmbientBrightness); }
	uint8_t& AmbientHue() { return Value<uint8_t>(PropOffsets_ZoneInfo.AmbientHue); }
	uint8_t& AmbientSaturation() { return Value<uint8_t>(PropOffsets_ZoneInfo.AmbientSaturation); }
	int& CutoffHz() { return Value<int>(PropOffsets_ZoneInfo.CutoffHz); }
	int& DamagePerSec() { return Value<int>(PropOffsets_ZoneInfo.DamagePerSec); }
	std::string& DamageString() { return Value<std::string>(PropOffsets_ZoneInfo.DamageString); }
	NameString& DamageType() { return Value<NameString>(PropOffsets_ZoneInfo.DamageType); }
	uint8_t& Delay() { return Value<uint8_t>(PropOffsets_ZoneInfo.Delay); }
	UClass*& EntryActor() { return Value<UClass*>(PropOffsets_ZoneInfo.EntryActor); }
	USound*& EntrySound() { return Value<USound*>(PropOffsets_ZoneInfo.EntrySound); }
	UTexture*& EnvironmentMap() { return Value<UTexture*>(PropOffsets_ZoneInfo.EnvironmentMap); }
	UClass*& ExitActor() { return Value<UClass*>(PropOffsets_ZoneInfo.ExitActor); }
	USound*& ExitSound() { return Value<USound*>(PropOffsets_ZoneInfo.ExitSound); }
	Color& FogColor() { return Value<Color>(PropOffsets_ZoneInfo.FogColor); }
	float& FogDistance() { return Value<float>(PropOffsets_ZoneInfo.FogDistance); }
	uint8_t& Gain() { return Value<uint8_t>(PropOffsets_ZoneInfo.Gain); }
	UTexture*& LensFlare() { return Value<UTexture*>(PropOffsets_ZoneInfo.LensFlare); }
	float& LensFlareOffset() { return Value<float>(PropOffsets_ZoneInfo.LensFlareOffset); }
	float& LensFlareScale() { return Value<float>(PropOffsets_ZoneInfo.LensFlareScale); }
	uint8_t& MasterGain() { return Value<uint8_t>(PropOffsets_ZoneInfo.MasterGain); }
	int& MaxCarcasses() { return Value<int>(PropOffsets_ZoneInfo.MaxCarcasses); }
	uint8_t& MaxLightCount() { return Value<uint8_t>(PropOffsets_ZoneInfo.MaxLightCount); }
	int& MaxLightingPolyCount() { return Value<int>(PropOffsets_ZoneInfo.MaxLightingPolyCount); }
	uint8_t& MinLightCount() { return Value<uint8_t>(PropOffsets_ZoneInfo.MinLightCount); }
	int& MinLightingPolyCount() { return Value<int>(PropOffsets_ZoneInfo.MinLightingPolyCount); }
	int& NumCarcasses() { return Value<int>(PropOffsets_ZoneInfo.NumCarcasses); }
	USkyZoneInfo*& SkyZone() { return Value<USkyZoneInfo*>(PropOffsets_ZoneInfo.SkyZone); }
	float& SpeedOfSound() { return Value<float>(PropOffsets_ZoneInfo.SpeedOfSound); }
	float& TexUPanSpeed() { return Value<float>(PropOffsets_ZoneInfo.TexUPanSpeed); }
	float& TexVPanSpeed() { return Value<float>(PropOffsets_ZoneInfo.TexVPanSpeed); }
	vec3& ViewFlash() { return Value<vec3>(PropOffsets_ZoneInfo.ViewFlash); }
	vec3& ViewFog() { return Value<vec3>(PropOffsets_ZoneInfo.ViewFog); }
	float& ZoneFluidFriction() { return Value<float>(PropOffsets_ZoneInfo.ZoneFluidFriction); }
	vec3& ZoneGravity() { return Value<vec3>(PropOffsets_ZoneInfo.ZoneGravity); }
	float& ZoneGroundFriction() { return Value<float>(PropOffsets_ZoneInfo.ZoneGroundFriction); }
	std::string& ZoneName() { return Value<std::string>(PropOffsets_ZoneInfo.ZoneName); }
	int& ZonePlayerCount() { return Value<int>(PropOffsets_ZoneInfo.ZonePlayerCount); }
	NameString& ZonePlayerEvent() { return Value<NameString>(PropOffsets_ZoneInfo.ZonePlayerEvent); }
	NameString& ZoneTag() { return Value<NameString>(PropOffsets_ZoneInfo.ZoneTag); }
	float& ZoneTerminalVelocity() { return Value<float>(PropOffsets_ZoneInfo.ZoneTerminalVelocity); }
	vec3& ZoneVelocity() { return Value<vec3>(PropOffsets_ZoneInfo.ZoneVelocity); }
	BitfieldBool bBounceVelocity() { return BoolValue(PropOffsets_ZoneInfo.bBounceVelocity); }
	BitfieldBool bDestructive() { return BoolValue(PropOffsets_ZoneInfo.bDestructive); }
	BitfieldBool bFogZone() { return BoolValue(PropOffsets_ZoneInfo.bFogZone); }
	BitfieldBool bGravityZone() { return BoolValue(PropOffsets_ZoneInfo.bGravityZone); }
	BitfieldBool bKillZone() { return BoolValue(PropOffsets_ZoneInfo.bKillZone); }
	BitfieldBool bMoveProjectiles() { return BoolValue(PropOffsets_ZoneInfo.bMoveProjectiles); }
	BitfieldBool bNeutralZone() { return BoolValue(PropOffsets_ZoneInfo.bNeutralZone); }
	BitfieldBool bNoInventory() { return BoolValue(PropOffsets_ZoneInfo.bNoInventory); }
	BitfieldBool bPainZone() { return BoolValue(PropOffsets_ZoneInfo.bPainZone); }
	BitfieldBool bRaytraceReverb() { return BoolValue(PropOffsets_ZoneInfo.bRaytraceReverb); }
	BitfieldBool bReverbZone() { return BoolValue(PropOffsets_ZoneInfo.bReverbZone); }
	BitfieldBool bWaterZone() { return BoolValue(PropOffsets_ZoneInfo.bWaterZone); }
	Ulocationid*& locationid() { return Value<Ulocationid*>(PropOffsets_ZoneInfo.locationid); }
};

class UnrealURL
{
public:
	UnrealURL() = default;
	UnrealURL(const UnrealURL& base, const std::string& url)
	{
		// To do: this also needs to be able to handle fully qualified URLs for network support

		*this = base;

		size_t pos = url.find('?');
		if (pos == std::string::npos)
		{
			Map = url;
		}
		else
		{
			Map = url.substr(0, pos);

			pos++;
			while (pos < url.size())
			{
				size_t endpos = url.find('?', pos);
				if (endpos == std::string::npos)
					endpos = url.size();
				AddOrReplaceOption(url.substr(pos, endpos - pos));
				pos = endpos + 1;
			}
		}
	}

	std::string Protocol = "unreal";
	std::string ProtocolDescription = "Unreal Protocol";
	std::string Name = "Player";
	std::string Map = "Index.unr";
	std::string Host;
	std::string Portal;
	std::string MapExt = "unr";
	std::string SaveExt = "usa";
	int Port = 7777;
	std::vector<std::string> Options;

	void AddOrReplaceOption(const std::string& newvalue)
	{
		size_t pos = newvalue.find('=');
		if (pos != std::string::npos)
		{
			std::string name = newvalue.substr(0, pos);
			for (char& c : name) c = std::tolower(c);
			for (std::string& option : Options)
			{
				if (option.size() >= name.size() + 1 && option[name.size()] == '=')
				{
					std::string key = option.substr(0, name.size());
					for (char& c : key) c = std::tolower(c);
					if (key == name)
					{
						option = newvalue;
						return;
					}
				}
			}
			Options.push_back(newvalue);
		}
		else
		{
			std::string name = newvalue;
			for (char& c : name) c = std::tolower(c);
			for (std::string& option : Options)
			{
				if (option.size() == name.size())
				{
					std::string key = option;
					for (char& c : key) c = std::tolower(c);
					if (key == name)
					{
						option = newvalue;
						return;
					}
				}
			}
			Options.push_back(newvalue);
		}
	}

	bool HasOption(const std::string& name) const
	{
		for (const std::string& option : Options)
		{
			if ((option.size() >= name.size() + 1 && option[name.size()] == '=') || option.size() == name.size())
			{
				std::string key = option.substr(0, name.size());
				for (char& c : key) c = std::tolower(c);
				if (key == name)
					return true;
			}
		}
		return false;
	}

	std::string GetOption(const std::string& name) const
	{
		for (const std::string& option : Options)
		{
			if (option.size() >= name.size() + 1 && option[name.size()] == '=')
			{
				std::string key = option.substr(0, name.size());
				for (char& c : key) c = std::tolower(c);
				if (key == name)
					return option.substr(name.size() + 1);
			}
		}
		return {};
	}

	std::string GetAddressURL() const
	{
		return Host + ":" + std::to_string(Port);
	}

	std::string GetOptions() const
	{
		std::string result;
		for (const std::string& option : Options)
		{
			result += "?";
			result += option;
		}
		return result;
	}

	std::string GetPortal() const
	{
		if (!Portal.empty())
			return "#" + Portal;
		else
			return std::string();
	}

	std::string ToString() const
	{
		std::string result;

		if (Protocol != "unreal")
		{
			result += Protocol;
			result += ":";
			if (!Host.empty())
				result += "//";
		}

		if (!Host.empty() || Port != 7777)
		{
			result += Host;
			result += ":";
			result += std::to_string(Port);
			result += "/";
		}

		result += Map;
		result += GetOptions();
		result += GetPortal();

		return result;
	}
};

struct ObjectTravelInfo
{
	ObjectTravelInfo(UInventory* inventory)
	{
		// To do: read properties
		// Note: only include properties with PropertyFlags::Travel
	}

	static std::vector<ObjectTravelInfo> Parse(const std::string& text)
	{
		return {};
	}

	static std::string ToString(const std::vector<ObjectTravelInfo>& objects)
	{
		return {};
	}

	std::string ClassName;
	std::map<std::string, std::string> Properties;
};

class ULevelInfo : public UZoneInfo
{
public:
	using UZoneInfo::UZoneInfo;

	void UpdateActorZone() override;

	UnrealURL URL;

	int& AIProfile() { return Value<int>(PropOffsets_LevelInfo.AIProfile); }
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
};

class UWarpZoneInfo : public UZoneInfo
{
public:
	using UZoneInfo::UZoneInfo;

	std::string& Destinations() { return Value<std::string>(PropOffsets_WarpZoneInfo.Destinations); }
	UWarpZoneInfo*& OtherSideActor() { return Value<UWarpZoneInfo*>(PropOffsets_WarpZoneInfo.OtherSideActor); }
	UObject*& OtherSideLevel() { return Value<UObject*>(PropOffsets_WarpZoneInfo.OtherSideLevel); }
	std::string& OtherSideURL() { return Value<std::string>(PropOffsets_WarpZoneInfo.OtherSideURL); }
	NameString& ThisTag() { return Value<NameString>(PropOffsets_WarpZoneInfo.ThisTag); }
	//Coords& WarpCoords() { return Value<Coords>(PropOffsets_WarpZoneInfo.WarpCoords); }
	BitfieldBool bNoTeleFrag() { return BoolValue(PropOffsets_WarpZoneInfo.bNoTeleFrag); }
	int& iWarpZone() { return Value<int>(PropOffsets_WarpZoneInfo.iWarpZone); }
	int& numDestinations() { return Value<int>(PropOffsets_WarpZoneInfo.numDestinations); }
};

class USkyZoneInfo : public UZoneInfo
{
public:
	using UZoneInfo::UZoneInfo;
};

class UReplicationInfo : public UInfo
{
public:
	using UInfo::UInfo;
};

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

class UStatLog : public UInfo
{
public:
	using UInfo::UInfo;

	int& Context() { return Value<int>(PropOffsets_StatLog.Context); }
	std::string& DecoderRingURL() { return Value<std::string>(PropOffsets_StatLog.DecoderRingURL); }
	std::string& GameCreator() { return Value<std::string>(PropOffsets_StatLog.GameCreator); }
	std::string& GameCreatorURL() { return Value<std::string>(PropOffsets_StatLog.GameCreatorURL); }
	std::string& GameName() { return Value<std::string>(PropOffsets_StatLog.GameName); }
	std::string& LocalBatcherParams() { return Value<std::string>(PropOffsets_StatLog.LocalBatcherParams); }
	std::string& LocalBatcherURL() { return Value<std::string>(PropOffsets_StatLog.LocalBatcherURL); }
	std::string& LocalLogDir() { return Value<std::string>(PropOffsets_StatLog.LocalLogDir); }
	std::string& LocalStandard() { return Value<std::string>(PropOffsets_StatLog.LocalStandard); }
	std::string& LocalStatsURL() { return Value<std::string>(PropOffsets_StatLog.LocalStatsURL); }
	std::string& LogInfoURL() { return Value<std::string>(PropOffsets_StatLog.LogInfoURL); }
	std::string& LogVersion() { return Value<std::string>(PropOffsets_StatLog.LogVersion); }
	float& TimeStamp() { return Value<float>(PropOffsets_StatLog.TimeStamp); }
	std::string& WorldBatcherParams() { return Value<std::string>(PropOffsets_StatLog.WorldBatcherParams); }
	std::string& WorldBatcherURL() { return Value<std::string>(PropOffsets_StatLog.WorldBatcherURL); }
	std::string& WorldLogDir() { return Value<std::string>(PropOffsets_StatLog.WorldLogDir); }
	std::string& WorldStandard() { return Value<std::string>(PropOffsets_StatLog.WorldStandard); }
	std::string& WorldStatsURL() { return Value<std::string>(PropOffsets_StatLog.WorldStatsURL); }
	BitfieldBool bWorld() { return BoolValue(PropOffsets_StatLog.bWorld); }
	BitfieldBool bWorldBatcherError() { return BoolValue(PropOffsets_StatLog.bWorldBatcherError); }
};

class UStatLogFile : public UStatLog
{
public:
	using UStatLog::UStatLog;

	int& LogAr() { return Value<int>(PropOffsets_StatLogFile.LogAr); }
	std::string& StatLogFile() { return Value<std::string>(PropOffsets_StatLogFile.StatLogFile); }
	std::string& StatLogFinal() { return Value<std::string>(PropOffsets_StatLogFile.StatLogFinal); }
	BitfieldBool bWatermark() { return BoolValue(PropOffsets_StatLogFile.bWatermark); }
};

class UBrush : public UActor
{
public:
	using UActor::UActor;

	Color& BrushColor() { return Value<Color>(PropOffsets_Brush.BrushColor); }
	uint8_t& CsgOper() { return Value<uint8_t>(PropOffsets_Brush.CsgOper); }
	// Scale& MainScale() { return Value<Scale>(PropOffsets_Brush.MainScale); }
	int& PolyFlags() { return Value<int>(PropOffsets_Brush.PolyFlags); }
	vec3& PostPivot() { return Value<vec3>(PropOffsets_Brush.PostPivot); }
	// Scale& PostScale() { return Value<Scale>(PropOffsets_Brush.PostScale); }
	// Scale& TempScale() { return Value<Scale>(PropOffsets_Brush.TempScale); }
	UObject*& UnusedLightMesh() { return Value<UObject*>(PropOffsets_Brush.UnusedLightMesh); }
	BitfieldBool bColored() { return BoolValue(PropOffsets_Brush.bColored); }
};

class UMover : public UBrush
{
public:
	using UBrush::UBrush;

	vec3& BasePos() { return Value<vec3>(PropOffsets_Mover.BasePos); }
	Rotator& BaseRot() { return Value<Rotator>(PropOffsets_Mover.BaseRot); }
	uint8_t& BrushRaytraceKey() { return Value<uint8_t>(PropOffsets_Mover.BrushRaytraceKey); }
	NameString& BumpEvent() { return Value<NameString>(PropOffsets_Mover.BumpEvent); }
	uint8_t& BumpType() { return Value<uint8_t>(PropOffsets_Mover.BumpType); }
	int& ClientUpdate() { return Value<int>(PropOffsets_Mover.ClientUpdate); }
	USound*& ClosedSound() { return Value<USound*>(PropOffsets_Mover.ClosedSound); }
	USound*& ClosingSound() { return Value<USound*>(PropOffsets_Mover.ClosingSound); }
	float& DamageThreshold() { return Value<float>(PropOffsets_Mover.DamageThreshold); }
	float& DelayTime() { return Value<float>(PropOffsets_Mover.DelayTime); }
	int& EncroachDamage() { return Value<int>(PropOffsets_Mover.EncroachDamage); }
	UMover*& Follower() { return Value<UMover*>(PropOffsets_Mover.Follower); }
	uint8_t& KeyNum() { return Value<uint8_t>(PropOffsets_Mover.KeyNum); }
	vec3* KeyPos() { return FixedArray<vec3>(PropOffsets_Mover.KeyPos); }
	Rotator* KeyRot() { return FixedArray<Rotator>(PropOffsets_Mover.KeyRot); }
	UMover*& Leader() { return Value<UMover*>(PropOffsets_Mover.Leader); }
	USound*& MoveAmbientSound() { return Value<USound*>(PropOffsets_Mover.MoveAmbientSound); }
	float& MoveTime() { return Value<float>(PropOffsets_Mover.MoveTime); }
	uint8_t& MoverEncroachType() { return Value<uint8_t>(PropOffsets_Mover.MoverEncroachType); }
	uint8_t& MoverGlideType() { return Value<uint8_t>(PropOffsets_Mover.MoverGlideType); }
	uint8_t& NumKeys() { return Value<uint8_t>(PropOffsets_Mover.NumKeys); }
	vec3& OldPos() { return Value<vec3>(PropOffsets_Mover.OldPos); }
	vec3& OldPrePivot() { return Value<vec3>(PropOffsets_Mover.OldPrePivot); }
	Rotator& OldRot() { return Value<Rotator>(PropOffsets_Mover.OldRot); }
	USound*& OpenedSound() { return Value<USound*>(PropOffsets_Mover.OpenedSound); }
	USound*& OpeningSound() { return Value<USound*>(PropOffsets_Mover.OpeningSound); }
	float& OtherTime() { return Value<float>(PropOffsets_Mover.OtherTime); }
	NameString& PlayerBumpEvent() { return Value<NameString>(PropOffsets_Mover.PlayerBumpEvent); }
	uint8_t& PrevKeyNum() { return Value<uint8_t>(PropOffsets_Mover.PrevKeyNum); }
	vec3& RealPosition() { return Value<vec3>(PropOffsets_Mover.RealPosition); }
	Rotator& RealRotation() { return Value<Rotator>(PropOffsets_Mover.RealRotation); }
	UTrigger*& RecommendedTrigger() { return Value<UTrigger*>(PropOffsets_Mover.RecommendedTrigger); }
	NameString& ReturnGroup() { return Value<NameString>(PropOffsets_Mover.ReturnGroup); }
	vec3& SavedPos() { return Value<vec3>(PropOffsets_Mover.SavedPos); }
	Rotator& SavedRot() { return Value<Rotator>(PropOffsets_Mover.SavedRot); }
	UActor*& SavedTrigger() { return Value<UActor*>(PropOffsets_Mover.SavedTrigger); }
	vec3& SimInterpolate() { return Value<vec3>(PropOffsets_Mover.SimInterpolate); }
	vec3& SimOldPos() { return Value<vec3>(PropOffsets_Mover.SimOldPos); }
	int& SimOldRotPitch() { return Value<int>(PropOffsets_Mover.SimOldRotPitch); }
	int& SimOldRotRoll() { return Value<int>(PropOffsets_Mover.SimOldRotRoll); }
	int& SimOldRotYaw() { return Value<int>(PropOffsets_Mover.SimOldRotYaw); }
	float& StayOpenTime() { return Value<float>(PropOffsets_Mover.StayOpenTime); }
	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_Mover.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_Mover.TriggerActor2); }
	UPawn*& WaitingPawn() { return Value<UPawn*>(PropOffsets_Mover.WaitingPawn); }
	uint8_t& WorldRaytraceKey() { return Value<uint8_t>(PropOffsets_Mover.WorldRaytraceKey); }
	BitfieldBool bClientPause() { return BoolValue(PropOffsets_Mover.bClientPause); }
	BitfieldBool bDamageTriggered() { return BoolValue(PropOffsets_Mover.bDamageTriggered); }
	BitfieldBool bDelaying() { return BoolValue(PropOffsets_Mover.bDelaying); }
	BitfieldBool bDynamicLightMover() { return BoolValue(PropOffsets_Mover.bDynamicLightMover); }
	BitfieldBool bOpening() { return BoolValue(PropOffsets_Mover.bOpening); }
	BitfieldBool bPlayerOnly() { return BoolValue(PropOffsets_Mover.bPlayerOnly); }
	BitfieldBool bSlave() { return BoolValue(PropOffsets_Mover.bSlave); }
	BitfieldBool bTriggerOnceOnly() { return BoolValue(PropOffsets_Mover.bTriggerOnceOnly); }
	BitfieldBool bUseTriggered() { return BoolValue(PropOffsets_Mover.bUseTriggered); }
	UNavigationPoint*& myMarker() { return Value<UNavigationPoint*>(PropOffsets_Mover.myMarker); }
	int& numTriggerEvents() { return Value<int>(PropOffsets_Mover.numTriggerEvents); }
};

class UPawn : public UActor
{
public:
	using UActor::UActor;

	void Tick(float elapsed, bool tickedFlag) override;
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

	bool CanHearNoise(UActor* source, float loudness);

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
	UNavigationPoint*& RouteCache() { return Value<UNavigationPoint*>(PropOffsets_Pawn.RouteCache); }
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
};

class UScout : public UPawn
{
public:
	using UPawn::UPawn;
};

class UPlayerPawn : public UPawn
{
public:
	using UPawn::UPawn;

	void Tick(float elapsed, bool tickedFlag) override;
	void TickRotating(float elapsed) override;

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
	Color& ProgressColor() { return Value<Color>(PropOffsets_PlayerPawn.ProgressColor); }
	std::string& ProgressMessage() { return Value<std::string>(PropOffsets_PlayerPawn.ProgressMessage); }
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
	NameString& WeaponPriority() { return Value<NameString>(PropOffsets_PlayerPawn.WeaponPriority); }
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
};

class UCamera : public UPlayerPawn
{
public:
	using UPlayerPawn::UPlayerPawn;
};
