#pragma once

#include "Packages/Core/UObject.h"
#include "Math/bbox.h"

class UZoneInfo;
class UTexture;
class UPawn;
class UInventory;
class UPrimitive;
class UMesh;
class ULevel;
class ULevelInfo;
class UModel;
class UAnimation;
class UViewport;
class USound;
class U227SkeletalMeshInstance;
class U227Projector;
class U227AnimationNotify;
class CollisionHit;
class BspNode;

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

enum ERenderStyle
{
	STY_None,
	STY_Normal,
	STY_Masked,
	STY_Translucent,
	STY_Modulated,
	STY_AlphaBlend
};

enum class ETravelType : uint8_t
{
	TRAVEL_Absolute, // Absolute URL
	TRAVEL_Partial,  // Partial (Carry name, reset server)
	TRAVEL_Relative  // Relative URL
};

// Unreal 227 only
// 227's Pawn class has a SightCheckType variable that will affect LineOfSightTo() function
enum class EPawnSightCheck : uint8_t
{
	SEE_PlayersOnly, // See bIsPlayer pawns only
	SEE_All,		 // See all pawns
	SEE_None		 // Do not do sight checks
};

// Deus Ex
enum class EAIEventState : uint8_t
{
	EAISTATE_Begin,
	EAISTATE_End,
	EAISTATE_Pulse,
	EAISTATE_ChangeBest
};

// Deus Ex
enum class EAIEventType : uint8_t
{
	EAITYPE_Visual,
	EAITYPE_Audio,
	EAITYPE_Olifactory
};

// Deus Ex
enum class EAllianceType : uint8_t
{
	ALLIANCE_Friendly,
	ALLIANCE_Neutral,
	ALLIANCE_Hostile
};

// Deus Ex
enum class EBarkModes : uint8_t
{
	BM_Idle,
	BM_CriticalDamage,
	BM_AreaSecure,
	BM_TargetAcquired,
	BM_TargetLost,
	BM_GoingForAlarm,
	BM_OutOfAmmo,
	BM_Scanning,
	BM_Futz,
	BM_OnFire,
	BM_TearGas,
	BM_Gore,
	BM_Surprise,
	BM_PreAttackSearching,
	BM_PreAttackSighting,
	BM_PostAttackSearching,
	BM_SearchGiveUp,
	BM_AllianceHostile,
	BM_AllianceFriendly
};

// Deus Ex (?)
enum class EDodgeDir : uint8_t
{
	DODGE_None,
	DODGE_Left,
	DODGE_Right,
	DODGE_Forward,
	DODGE_Back,
	DODGE_Active,
	DODGE_Done
};

// Unreal 227
enum class EAmbients : uint8_t
{
	REVERB_PRESET_GENERIC,
	REVERB_PRESET_PADDEDCELL,
	REVERB_PRESET_ROOM,
	REVERB_PRESET_BATHROOM,
	REVERB_PRESET_LIVINGROOM,
	REVERB_PRESET_STONEROOM,
	REVERB_PRESET_AUDITORIUM,
	REVERB_PRESET_CONCERTHALL,
	REVERB_PRESET_CAVE,
	REVERB_PRESET_ARENA,
	REVERB_PRESET_HANGAR,
	REVERB_PRESET_CARPETTEDHALLWAY,
	REVERB_PRESET_HALLWAY,
	REVERB_PRESET_STONECORRIDOR,
	REVERB_PRESET_ALLEY,
	REVERB_PRESET_FOREST,
	REVERB_PRESET_CITY,
	REVERB_PRESET_MOUNTAINS,
	REVERB_PRESET_QUARRY,
	REVERB_PRESET_PLAIN,
	REVERB_PRESET_PARKINGLOT,
	REVERB_PRESET_SEWERPIPE,
	REVERB_PRESET_UNDERWATER,
	REVERB_PRESET_DRUGGED,
	REVERB_PRESET_DIZZY,
	REVERB_PRESET_PSYCHOTIC,
	REVERB_PRESET_CASTLE_SMALLROOM,
	REVERB_PRESET_CASTLE_SHORTPASSAGE,
	REVERB_PRESET_CASTLE_MEDIUMROOM,
	REVERB_PRESET_CASTLE_LONGPASSAGE,
	REVERB_PRESET_CASTLE_LARGEROOM,
	REVERB_PRESET_CASTLE_HALL,
	REVERB_PRESET_CASTLE_CUPBOARD,
	REVERB_PRESET_CASTLE_COURTYARD,
	REVERB_PRESET_CASTLE_ALCOVE,
	REVERB_PRESET_FACTORY_ALCOVE,
	REVERB_PRESET_FACTORY_SHORTPASSAGE,
	REVERB_PRESET_FACTORY_MEDIUMROOM,
	REVERB_PRESET_FACTORY_LONGPASSAGE,
	REVERB_PRESET_FACTORY_LARGEROOM,
	REVERB_PRESET_FACTORY_HALL,
	REVERB_PRESET_FACTORY_CUPBOARD,
	REVERB_PRESET_FACTORY_COURTYARD,
	REVERB_PRESET_FACTORY_SMALLROOM,
	REVERB_PRESET_ICEPALACE_ALCOVE,
	REVERB_PRESET_ICEPALACE_SHORTPASSAGE,
	REVERB_PRESET_ICEPALACE_MEDIUMROOM,
	REVERB_PRESET_ICEPALACE_LONGPASSAGE,
	REVERB_PRESET_ICEPALACE_LARGEROOM,
	REVERB_PRESET_ICEPALACE_HALL,
	REVERB_PRESET_ICEPALACE_CUPBOARD,
	REVERB_PRESET_ICEPALACE_COURTYARD,
	REVERB_PRESET_ICEPALACE_SMALLROOM,
	REVERB_PRESET_SPACESTATION_ALCOVE,
	REVERB_PRESET_SPACESTATION_MEDIUMROOM,
	REVERB_PRESET_SPACESTATION_SHORTPASSAGE,
	REVERB_PRESET_SPACESTATION_LONGPASSAGE,
	REVERB_PRESET_SPACESTATION_LARGEROOM,
	REVERB_PRESET_SPACESTATION_HALL,
	REVERB_PRESET_SPACESTATION_CUPBOARD,
	REVERB_PRESET_SPACESTATION_SMALLROOM,
	REVERB_PRESET_WOODEN_ALCOVE,
	REVERB_PRESET_WOODEN_SHORTPASSAGE,
	REVERB_PRESET_WOODEN_MEDIUMROOM,
	REVERB_PRESET_WOODEN_LONGPASSAGE,
	REVERB_PRESET_WOODEN_LARGEROOM,
	REVERB_PRESET_WOODEN_HALL,
	REVERB_PRESET_WOODEN_CUPBOARD,
	REVERB_PRESET_WOODEN_SMALLROOM,
	REVERB_PRESET_WOODEN_COURTYARD,
	REVERB_PRESET_SPORT_EMPTYSTADIUM,
	REVERB_PRESET_SPORT_SQUASHCOURT,
	REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL,
	REVERB_PRESET_SPORT_LARGESWIMMINGPOOL,
	REVERB_PRESET_SPORT_GYMNASIUM,
	REVERB_PRESET_SPORT_FULLSTADIUM,
	REVERB_PRESET_SPORT_STADIUMTANNOY,
	REVERB_PRESET_PREFAB_WORKSHOP,
	REVERB_PRESET_PREFAB_SCHOOLROOM,
	REVERB_PRESET_PREFAB_PRACTISEROOM,
	REVERB_PRESET_PREFAB_OUTHOUSE,
	REVERB_PRESET_PREFAB_CARAVAN,
	REVERB_PRESET_DOME_TOMB,
	REVERB_PRESET_PIPE_SMALL,
	REVERB_PRESET_DOME_SAINTPAULS,
	REVERB_PRESET_PIPE_LONGTHIN,
	REVERB_PRESET_PIPE_LARGE,
	REVERB_PRESET_PIPE_RESONANT,
	REVERB_PRESET_OUTDOORS_BACKYARD,
	REVERB_PRESET_OUTDOORS_ROLLINGPLAINS,
	REVERB_PRESET_OUTDOORS_DEEPCANYON,
	REVERB_PRESET_OUTDOORS_CREEK,
	REVERB_PRESET_OUTDOORS_VALLEY,
	REVERB_PRESET_MOOD_HEAVEN,
	REVERB_PRESET_MOOD_HELL,
	REVERB_PRESET_MOOD_MEMORY,
	REVERB_PRESET_DRIVING_COMMENTATOR,
	REVERB_PRESET_DRIVING_PITGARAGE,
	REVERB_PRESET_DRIVING_INCAR_RACER,
	REVERB_PRESET_DRIVING_INCAR_SPORTS,
	REVERB_PRESET_DRIVING_INCAR_LUXURY,
	REVERB_PRESET_DRIVING_FULLGRANDSTAND,
	REVERB_PRESET_DRIVING_EMPTYGRANDSTAND,
	REVERB_PRESET_DRIVING_TUNNEL,
	REVERB_PRESET_CITY_STREETS,
	REVERB_PRESET_CITY_SUBWAY,
	REVERB_PRESET_CITY_MUSEUM,
	REVERB_PRESET_CITY_LIBRARY,
	REVERB_PRESET_CITY_UNDERPASS,
	REVERB_PRESET_CITY_ABANDONED,
	REVERB_PRESET_DUSTYROOM,
	REVERB_PRESET_CHAPEL,
	REVERB_PRESET_SMALLWATERROOM,
	REVERB_PRESET_UNDERSLIME,
	REVERB_PRESET_NONE
};

// Unreal 227
enum class EDynZoneInfoType : uint8_t
{
	DZONE_Cube,
	DZONE_Sphere,
	DZONE_Cylinder,
	DZONE_Script
};

// UT 469
enum FontFamily
{
	FF_Arial,	// Arial on Windows, Helvetica on Linux/Mac
	FF_Times,	// Times New Roman on Windows, times on Linux/Mac
	FF_Courier, // Courier New on Windows, courier on Linux/Mac
	FF_Tahoma   // Standard UWindow font, Linux/Mac will use Verdana if requested.
};

enum class EAnimType : uint8_t
{
	AT_Replace,
	AT_Combine,
};

class UActor : public UObject
{
public:
	using UObject::UObject;

	UActor* Spawn(UClass* SpawnClass, std::optional<UActor*> SpawnOwner, std::optional<NameString> SpawnTag, std::optional<vec3> SpawnLocation, std::optional<Rotator> SpawnRotation);
	bool Destroy();
	void InitBase();

	void SetBase(UActor* newBase, bool sendBaseChangeEvent);
	// Re-registers this actor into ActorBase()->BasedActors without firing Attach/BaseChange
	// events. BasedActors is native runtime-only state (never serialized), unlike ActorBase()
	// itself, so a freshly loaded actor whose ActorBase() property already points at another
	// actor needs this to make that relationship work in both directions again.
	void RelinkBasedActor();
	void SetOwner(UActor* newOwner);
	virtual void InitActorZone();
	virtual void UpdateActorZone();
	PointRegion FindRegion(const vec3& offset = vec3(0.0f));

	virtual void Tick(float elapsed);

	void TickAnimation(float elapsed);
	void TickBlendAnimation(float elapsed);

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

	void PhysLanded(UActor* hitActor, const vec3& hitNormal);

	// True (and fires FellOutOfWorld) if the actor's location has no valid region at all, i.e. it fell outside the level's BSP geometry.
	bool HasLeftWorld();
	// Shared accel/friction/max-speed update used by TickWalking, TickSwimming and TickFlying.
	void ApplyMovementAcceleration(float elapsed, float accelRate, float friction, float maxSpeed);
	// True if the movement loop calling this should stop: the actor was destroyed, or Physics()
	// no longer matches expectedPhysics. Both can happen from inside TryMove
	bool ShouldAbortMovementTick(uint8_t expectedPhysics);
	// Common TickWalking/TickSwimming/TickFlying preamble: casts to UPawn (null return means "not a
	// pawn, caller should bail"), checks HasLeftWorld, and resets OldLocation/bJustTeleported for the tick.
	UPawn* PreparePawnMovementTick();
	// Fires the HitWall event with the actor/level that was hit - used by every movement tick function.
	void FireHitWall(const CollisionHit& hit);
	// Dry-runs stepDownDelta to see if the ground is still reachable; steps down onto it and sets Base
	// if so (returns true), or switches to PHYS_Falling and clears Base if not (returns false).
	bool TryStepToGround(vec3 stepDownDelta);
	// Recovers Velocity from actual displacement over the tick, unless bJustTeleported was set (which
	// makes the displacement meaningless as a velocity source).
	void RecomputeVelocityFromDisplacement(float elapsed);

	virtual void TickRotating(float elapsed);

	void SetPhysics(uint8_t newPhysics);
	void SetCollision(bool newColActors, bool newBlockActors, bool newBlockPlayers);

	std::pair<bool, vec3> CheckLocation(vec3 location, float radius, float height, bool check);

	bool SetLocation(const vec3& newLocation);
	bool SetRotation(const Rotator& newRotation);
	bool SetCollisionSize(float newRadius, float newHeight);

	UObject* Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent);
	// Unreal 227's version of Actor.Trace()
	UObject* Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent, bool bTraceBSP, uint8_t	BSPTraceFlags);
	bool FastTrace(const vec3& traceEnd, const vec3& traceStart);
	// Unreal 227 - Trace against world and return the wanted information (location, normal, texture and/or polyflags)
	bool TraceSurfHitInfo(vec3& Start, vec3& End, vec3* HitLocation, vec3* HitNormal, UTexture* HitTex, int* HitFlags);
	// Unreal 227 - Perform a single line check with this actor
	bool TraceThisActor(vec3& TraceEnd, vec3 TraceStart, vec3* HitLocation, vec3* HitNormal, std::optional<vec3> Extent);

	CollisionHit TryMove(const vec3& delta, bool dryRun = false, bool isOwnBaseBlocking = true);
	CollisionHit TryMoveSmooth(const vec3& delta);
	bool Move(const vec3& delta);
	bool MoveSmooth(const vec3& delta);

	bool IsBasedOn(UActor* other);
	bool IsOwnedBy(UActor* owner);
	bool IsOverlapping(UActor* other);

	void Touch(UActor* actor);
	void UnTouch(UActor* actor);
	void CheckPendingTouch();

	static const int TouchingArraySize = 4;
	bool TouchEventSent[TouchingArraySize] = {};

	bool HasAnim(const NameString& sequence);
	bool IsAnimating();
	bool IsAnimating_HP(std::optional<NameString> RootBone);
	void FinishAnim();
	void FinishAnim_HP(std::optional<NameString> RootBone);
	NameString GetAnimGroup(const NameString& sequence);
	void PlayAnim(const NameString& sequence, float rate, float tweenTime);
	void PlayBlendAnim(const NameString& sequenceName, float rate, float tweenTime, int blendSlot);
	void TweenBlendAnim(const NameString& sequenceName, float time, int blendSlot);
	void LoopAnim(const NameString& sequence, float rate, float tweenTime, float minRate);
	void TweenAnim(const NameString& sequence, float tweenTime);

	void MakeNoise(float loudness);
	bool PlayerCanSeeMe();

	// Harry Potter
	void PlayAnim_HP(const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<EAnimType> Type, std::optional<NameString> RootBone);
	void LoopAnim_HP(const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<float> MinRate, std::optional<EAnimType> Type, std::optional<NameString> RootBone);
	BoundingBox GetWorldCollisionBox(bool bVisual);
	vec3 GetRenderExtent();
	UActor* CreateAnimChannel(UClass* NewClass, EAnimType Type, const NameString& RootBone, bool bTransient);
	int BoneNumber(const NameString& Bone);
	NameString BoneName(int Bone);
	vec3 BonePos(const NameString& Bone);
	UTexture* CreateTextureFromScreenShot(UViewport* vport);
	UTexture* CreateTextureFromBMP(const std::string& name, const std::string& filename);
	bool SaveObjectAsFile(const std::string& dir, UObject* object);
	bool LoadObjectAsFile(const std::string& dir, UObject* object);
	bool SaveGameSaveInfo(const std::string& dir, UObject* object);
	bool LoadGameSaveInfo(const std::string& dir, UObject* object);
	bool IsOSVer2kOrXP();

	void UpdateBspInfo();
	void AddToBspNode(BspNode* node);
	void RemoveFromBspNode();
	static int NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node);

	// The status of the actor in the collision hash
	struct
	{
		bool Inserted = false;
		vec3 Location = { 0.0f };
		vec3 Extents = { 0.0f };
		int CheckCounter = -1;
	} Collision;

	// The status of the actor in the light hash
	struct
	{
		bool Inserted = false;
		vec3 Location = { 0.0f };
		float Radius = 0.0f;
		int CheckCounter = -1;
	} Light;

	// Lights touching this actor
	struct
	{
		bool NeedsUpdate = true;
		vec3 Location = vec3(0.0f);
		Array<UActor*> LightList;
	} LightInfo;

	// Fog between actor and camera
	struct
	{
		vec3 fogcolor = { 0.0f };
		float brightness = -1.0f;
		float fog = -1.0f;
		float radius = -1.0f;
		vec3 location = { 0.0f };
	} FogInfo;

	// Location in the BSP tree
	struct
	{
		BBox BoundingBox;
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

	// Tweening blend animation state (per slot)
	struct
	{
		int V0 = 0;
		int V1 = 0;
		float T = -1.0f;
	} TweenFromBlendAnimFrame[4];

	int LastDrawFrame = -1;

	float SleepTimeLeft = 0.0f;
	vec3 gravityVector;

	// Index in level Actors array
	int Index = -1;

	// Child actor tracking
	Array<UActor*> ChildActors;
	// Based actor tracking
	Array<UActor*> BasedActors;

	void AddChildActor(UActor* actor);
	void RemoveChildActor(UActor* actor);

	void SetTweenFromAnimFrame();
	void SetTweenFromBlendAnimFrame(int slot);

	UTexture* GetMultiskin(int index);

	void DeusExConBindEvents();

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
	FixedArrayView<UTexture*, 8> MultiSkins() { return FixedArray<UTexture*, 8>(PropOffsets_Actor.MultiSkins); }
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
	float& LastRenderTime() { return Value<float>(PropOffsets_Actor.LastRenderTime); }
	float& DistanceFromPlayer() { return Value<float>(PropOffsets_Actor.DistanceFromPlayer); }
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
	ERenderStyle Style() { return static_cast<ERenderStyle>(Value<uint8_t>(PropOffsets_Actor.Style)); }
	NameString& Tag() { return Value<NameString>(PropOffsets_Actor.Tag); }
	UActor*& Target() { return Value<UActor*>(PropOffsets_Actor.Target); }
	UTexture*& Texture() { return Value<UTexture*>(PropOffsets_Actor.Texture); }
	float& TimerCounter() { return Value<float>(PropOffsets_Actor.TimerCounter); }
	float& TimerRate() { return Value<float>(PropOffsets_Actor.TimerRate); }
	FixedArrayView<UActor*, 4> Touching() { return FixedArray<UActor*, 4>(PropOffsets_Actor.Touching); }
	TypedScriptArray<UActor*> Touching_UT469() { return DynamicArray<UActor*>(PropOffsets_Actor.Touching); }
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
	BitfieldBool bBlockSight() { return BoolValue(PropOffsets_Actor.bBlockSight); }
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

	// Deus Ex exclusive properties
	std::string& BindName() { return Value<std::string>(PropOffsets_Actor.BindName); }
	std::string& BarkBindName() { return Value<std::string>(PropOffsets_Actor.BarkBindName); }

	FixedArrayView<float, 4> BlendAnimLast() { return FixedArray<float, 4>(PropOffsets_Actor.BlendAnimLast); }
	FixedArrayView<float, 4> BlendAnimMinRate() { return FixedArray<float, 4>(PropOffsets_Actor.BlendAnimMinRate); }
	FixedArrayView<float, 4> OldBlendAnimRate() { return FixedArray<float, 4>(PropOffsets_Actor.OldBlendAnimRate); }
	FixedArrayView<vec4, 4> SimBlendAnim() { return FixedArray<vec4, 4>(PropOffsets_Actor.SimBlendAnim); }

	std::string& FamiliarName() { return Value<std::string>(PropOffsets_Actor.FamiliarName); }
	std::string& UnfamiliarName() { return Value<std::string>(PropOffsets_Actor.FamiliarName); }
	UObject*& ConListItems() { return Value<UObject*>(PropOffsets_Actor.ConListItems); }
	float& LastConEndTime() { return Value<float>(PropOffsets_Actor.LastConEndTime); }
	float& ConStartInterval() { return Value<float>(PropOffsets_Actor.ConStartInterval); }

	float& VisUpdateTime() { return Value<float>(PropOffsets_Actor.VisUpdateTime); }
	float& CurrentVisibility() { return Value<float>(PropOffsets_Actor.CurrentVisibility); }
	float& LastVisibility() { return Value<float>(PropOffsets_Actor.LastVisibility); }

	UClass*& SmellClass() { return Value<UClass*>(PropOffsets_Actor.SmellClass); }
	// SmellNode*& LastSmellNode() { return Value<SmellNode*>(PropOffsets_Actor.LastSmellNode); } // SmellNode is not a native class

	BitfieldBool bOwned() { return BoolValue(PropOffsets_Actor.bOwned); }

	FixedArrayView<NameString, 4> BlendAnimSequence() { return FixedArray<NameString, 4>(PropOffsets_Actor.BlendAnimSequence); }
	FixedArrayView<float, 4> BlendAnimFrame() { return FixedArray<float, 4>(PropOffsets_Actor.BlendAnimFrame); }
	FixedArrayView<float, 4> BlendAnimRate() { return FixedArray<float, 4>(PropOffsets_Actor.BlendAnimRate); }
	FixedArrayView<float, 4> BlendTweenRate() { return FixedArray<float, 4>(PropOffsets_Actor.BlendTweenRate); }

	// Unreal 227 exclusive Properties
	BitfieldBool bNetNotify() { return BoolValue(PropOffsets_Actor.bNetNotify); }
	BitfieldBool bHandleOwnCorona() { return BoolValue(PropOffsets_Actor.bHandleOwnCorona); }
	BitfieldBool bRenderMultiEnviroMaps() { return BoolValue(PropOffsets_Actor.bRenderMultiEnviroMaps); }
	BitfieldBool bWorldGeometry() { return BoolValue(PropOffsets_Actor.bWorldGeometry); }
	BitfieldBool bUseMeshCollision() { return BoolValue(PropOffsets_Actor.bUseMeshCollision); }
	BitfieldBool bEditorSelectRender() { return BoolValue(PropOffsets_Actor.bEditorSelectRender); }
	BitfieldBool bNoDynamicShadowCast() { return BoolValue(PropOffsets_Actor.bNoDynamicShadowCast); }
	BitfieldBool bIsInOctree() { return BoolValue(PropOffsets_Actor.bIsInOctree); }
	BitfieldBool bProjectorDecal() { return BoolValue(PropOffsets_Actor.bProjectorDecal); }
	BitfieldBool bUseLitSprite() { return BoolValue(PropOffsets_Actor.bUseLitSprite); }
	BitfieldBool bAlwaysRender() { return BoolValue(PropOffsets_Actor.bAlwaysRender); }

	float& LastRenderedTime() { return Value<float>(PropOffsets_Actor.LastRenderedTime); }
	Color& ActorRenderColor() { return Value<Color>(PropOffsets_Actor.ActorRenderColor); }
	Color& ActorGUnlitColor() { return Value<Color>(PropOffsets_Actor.ActorGUnlitColor); }
	UPrimitive*& CollisionOverride() { return Value<UPrimitive*>(PropOffsets_Actor.CollisionOverride); }
	U227SkeletalMeshInstance*& MeshInstance() { return Value<U227SkeletalMeshInstance*>(PropOffsets_Actor.MeshInstance); }
	vec3*& RelativeLocation() { return Value<vec3*>(PropOffsets_Actor.RelativeLocation); }
	Rotator*& RelativeRotation() { return Value<Rotator*>(PropOffsets_Actor.RelativeRotation); }
	// Pointer type LightDataPtr()
	// Pointer type MeshDataPtr()
	TypedScriptArray<U227Projector*> ProjectorList() { return DynamicArray<U227Projector*>(PropOffsets_Actor.ProjectorList); }
	// Pointer type NetInitialProperties()
	TypedScriptArray<UActor*> RealTouching() { return DynamicArray<UActor*>(PropOffsets_Actor.RealTouching); }

	UClass*& DefaultAnimationNotify() { return Value<UClass*>(PropOffsets_Actor.DefaultAnimationNotify); }
	U227AnimationNotify*& AnimationNotify() { return Value<U227AnimationNotify*>(PropOffsets_Actor.AnimationNotify); }

	BitfieldBool bSkipActorReplication() { return BoolValue(PropOffsets_Actor.bSkipActorReplication); }
	BitfieldBool bRepAnimations() { return BoolValue(PropOffsets_Actor.bRepAnimations); }
	BitfieldBool bRepAmbientSound() { return BoolValue(PropOffsets_Actor.bRepAmbientSound); }
	BitfieldBool bSimulatedPawnRep() { return BoolValue(PropOffsets_Actor.bSimulatedPawnRep); }
	BitfieldBool bRepMesh() { return BoolValue(PropOffsets_Actor.bRepMesh); }

protected:
	static constexpr float stepDownDeltaFactor = 1.3f;

private:
	void AddBasedActor(UActor* actor);
	void RemoveBasedActor(UActor* actor);
	void TurnBasedActors(const Rotator& deltaRotation);

	// TODO: Compare behavior more closely with original engine. Might differ depending on game.
};
