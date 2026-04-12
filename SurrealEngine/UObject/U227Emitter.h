#pragma once

#include "U227Emitter.h"
#include "UActor.h"

class UXEmitter;
class UXParticleForces;
class UXRainRestrictionVolume;

// Structs found in XParticleEmitter
// These are used in subclasses

struct IntRange
{
    int Min, Max;
};

struct ByteRange
{
    uint8_t Min, Max;
};

struct FloatRange
{
    float Min, Max;
};

struct RangeVector
{
    FloatRange X, Y, Z;
};

struct ParticleSndType
{
    USound* Sounds[8];
    FloatRange SndPitch, SndRadius, SndVolume;
    uint8_t SndCount;
};

// Enums found in XParticleEmitter

enum class EHitEventType : uint8_t
{
    HIT_DoNothing,
    HIT_Destroy,
    HIT_StopMovement,
    HIT_Bounce,
    HIT_Script
};

// Particle Emitter base class
class UXParticleEmitter : public UActor
{
public:
    using UActor::UActor;

    int& ActiveCount() { return Value<int>(PropOffsets_XParticleEmitter.ActiveCount); }
    BitfieldBool bHasInitilized() { return BoolValue(PropOffsets_XParticleEmitter.bHasInitilized); }
    BitfieldBool bKillNextTick() { return BoolValue(PropOffsets_XParticleEmitter.bKillNextTick); }
    BitfieldBool bHasSpecialParts() { return BoolValue(PropOffsets_XParticleEmitter.bHasSpecialParts); }
    BitfieldBool bWasPostDestroyed() { return BoolValue(PropOffsets_XParticleEmitter.bWasPostDestroyed); }
    BitfieldBool bHasInitView() { return BoolValue(PropOffsets_XParticleEmitter.bHasInitView); }
    Array<UXEmitter*> PartCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XParticleEmitter.PartCombiners); }
    BitfieldBool bUSNotifyParticles() { return BoolValue(PropOffsets_XParticleEmitter.bUSNotifyParticles); }
    BitfieldBool bNotifyNetReceive() { return BoolValue(PropOffsets_XParticleEmitter.bNotifyNetReceive); }
    BitfieldBool bUSModifyParticles() { return BoolValue(PropOffsets_XParticleEmitter.bUSModifyParticles); }
    BitfieldBool bNotOnPortals() { return BoolValue(PropOffsets_XParticleEmitter.bNotOnPortals); }

    void SetParticlesProps(std::optional<float> Speed, std::optional<float> Scale);
};

// Enums found in XEmitter

enum class ESpawnPosType : uint8_t
{
    SP_Box,
    SP_Sphere,
    SP_Cylinder,
    SP_BoxSphere,
    SP_BoxCylinder
};

enum class EEmitterTriggerType : uint8_t
{
    ETR_ToggleDisabled,
    ETR_ResetEmitter,
    ETR_SpawnParticles
};

enum class EEmitterPartCol : uint8_t
{
    ECT_HitNothing,
    ECT_HitWalls,
    ECT_HitActors,
    ECT_HitProjTargets
};

enum class ESpriteAnimType : uint8_t
{
    SAN_None,
    SAN_PlayOnce,
    SAN_PlayOnceInverted,
    SAN_LoopAnim
};

// Structs found in XEmitter

struct SpeedRangeType
{
    float VelocityScale, Time;
};

struct Speed3DType
{
    vec3 VelocityScale;
    float Time;
};

struct RevolveScaleType
{
    vec3 RevolutionScale;
    float Time;
};

struct ScaleRangeType
{
    float DrawScaling, Type;
};

struct ColorScaleRangeType
{
    float Time;
    vec3 ColorScaling;
};

struct Box
{
    vec3 Min, Max;
    uint8_t IsValid; // No idea why this is a byte instead of a bool
};

// Enums found in XBeamEmitter

enum class EBeamTargetType : uint8_t
{
    BEAM_Velocity,
    BEAM_BeamActor,
    BEAM_Offset,
    BEAM_OffsetAsAbsolute
};

// Structs found in XBeamEmitter

struct FBeamTargetPoint
{
    UActor* TargetActor;
    vec3* Offset;
};

// Enums found in XMeshEmitter

enum class EEmPartRotType : uint8_t
{
    MEP_DesiredRot,
    MEP_FacingCamera,
    MEP_YawingToCamera
};

// Structs found in XMeshEmitter

struct AnimationType
{
    NameString AnimSeq;
    float Frame, Rate;
    bool bAnimLoop;
};

// Enums found in XSpriteEmitter

enum class ESprPartRotType : uint8_t
{
    SPR_DesiredRot,
    SPR_RelFacingVelocity,
    SPR_AbsFacingVelocity,
    SPR_RelFacingNormal,
    SPR_AbsFacingNormal
};

// Enums found in XWeatherEmitter

enum class EWeatherAreaType : uint8_t
{
    EWA_Box, // Use Location + AppearArea for rain appearance area
    EWA_Zone // Use current zone as the appearance area
};

enum class EFallingType : uint8_t
{
    EWF_Rain,
    EWF_Snow,
    EWF_Dust,
    EWF_Neither
};

// Enums found in XTrailEmitter

enum class ETrailType : uint8_t
{
    TRAIL_Sheet,      // Single sheet trailer
    TRAIL_DoubleSheet // Cross shaped trailer
};

// Structs found in XTrailEmitter

struct TrailOffsetPart
{
    vec3 Location, Velocity, Color, Accel;
    float LifeSpan[3], Scale, X;
};

/////////////////////////////////////////////////////////////////

// Normal Particle Emitter
class UXEmitter : public UXParticleEmitter
{
public:
    using UXParticleEmitter::UXParticleEmitter;

    void SpawnParticles(int Count);
    void SetMaxParticles(int MaxParts);
    void Kill(); // Stop spawning particles and destroy self when all the remaining particles are gone
    void EmTrigger(); // Trigger this emitter

    FloatRange*& AutoResetTime() { return Value<FloatRange*>(PropOffsets_XEmitter.AutoResetTime); }
    BitfieldBool bAccelRelativeToRot() { return BoolValue(PropOffsets_XEmitter.bAccelRelativeToRot); }
    BitfieldBool BACKUP_Disabled() { return BoolValue(PropOffsets_XEmitter.BACKUP_Disabled); }
    BitfieldBool bActorsBlockSight() { return BoolValue(PropOffsets_XEmitter.bActorsBlockSight); }
    BitfieldBool bAutoDestroy() { return BoolValue(PropOffsets_XEmitter.bAutoDestroy); }
    BitfieldBool bAutoReset() { return BoolValue(PropOffsets_XEmitter.bAutoReset); }
    BitfieldBool bAutoVisibilityBox() { return BoolValue(PropOffsets_XEmitter.bAutoVisibilityBox); }
    BitfieldBool bBoxVisibility() { return BoolValue(PropOffsets_XEmitter.bBoxVisibility); }
    BitfieldBool bCheckLineOfSight() { return BoolValue(PropOffsets_XEmitter.bCheckLineOfSight); }
    BitfieldBool bCOffsetRelativeToRot() { return BoolValue(PropOffsets_XEmitter.bCOffsetRelativeToRot); }
    BitfieldBool bCylRangeBasedOnPos() { return BoolValue(PropOffsets_XEmitter.bCylRangeBasedOnPos); }
    BitfieldBool bDestruction() { return BoolValue(PropOffsets_XEmitter.bDestruction); }
    BitfieldBool bDisabled() { return BoolValue(PropOffsets_XEmitter.bDisabled); }
    BitfieldBool bDisableRender() { return BoolValue(PropOffsets_XEmitter.bDisableRender); }
    BitfieldBool bDistanceCulling() { return BoolValue(PropOffsets_XEmitter.bDistanceCulling); }
    BitfieldBool bEffectsVelocity() { return BoolValue(PropOffsets_XEmitter.bEffectsVelocity); }
    BitfieldBool bGradualSpawnCoords() { return BoolValue(PropOffsets_XEmitter.bGradualSpawnCoords); }
    BitfieldBool bHasAliveParticles() { return BoolValue(PropOffsets_XEmitter.bHasAliveParticles); }
    BitfieldBool bHasLossVel() { return BoolValue(PropOffsets_XEmitter.bHasLossVel); }
    BitfieldBool bNoUpdateOnInvis() { return BoolValue(PropOffsets_XEmitter.bNoUpdateOnInvis); }
    BitfieldBool bParticleCoronaEnabled() { return BoolValue(PropOffsets_XEmitter.bParticleCoronaEnabled); }
    BitfieldBool bRelativeToRotation() { return BoolValue(PropOffsets_XEmitter.bRelativeToRotation); }
    BitfieldBool bRespawnParticles() { return BoolValue(PropOffsets_XEmitter.bRespawnParticles); }
    BitfieldBool bRevolutionEnabled() { return BoolValue(PropOffsets_XEmitter.bRevolutionEnabled); }
    BitfieldBool bRotationRequest() { return BoolValue(PropOffsets_XEmitter.bRotationRequest); }
    BitfieldBool bSpawnInitParticles() { return BoolValue(PropOffsets_XEmitter.bSpawnInitParticles); }
    BitfieldBool bStatisEmitter() { return BoolValue(PropOffsets_XEmitter.bStatisEmitter); }
    BitfieldBool bUseMeshAnim() { return BoolValue(PropOffsets_XEmitter.bUseMeshAnim); }
    BitfieldBool bUseRandomTex() { return BoolValue(PropOffsets_XEmitter.bUseRandomTex); }
    BitfieldBool bUseRelativeLocation() { return BoolValue(PropOffsets_XEmitter.bUseRelativeLocation); }
    BitfieldBool bVelRelativeToRotation() { return BoolValue(PropOffsets_XEmitter.bVelRelativeToRotation); }
    RangeVector*& BoxLocation() { return Value<RangeVector*>(PropOffsets_XEmitter.BoxLocation); }
    RangeVector*& BoxVelocity() { return Value<RangeVector*>(PropOffsets_XEmitter.BoxVelocity); }
    Coords& CacheRot() { return Value<Coords>(PropOffsets_XEmitter.CacheRot); }
    uint8_t& ColorScaleCount() { return Value<uint8_t>(PropOffsets_XEmitter.ColorScaleCount); }
    IntRange*& CombinedParticleCount() { return Value<IntRange*>(PropOffsets_XEmitter.CombinedParticleCount); }
    RangeVector*& CoronaColor() { return Value<RangeVector*>(PropOffsets_XEmitter.CoronaColor); }
    float& CoronaFadeTimeScale() { return Value<float>(PropOffsets_XEmitter.CoronaFadeTimeScale); }
    float& CoronaMaxScale() { return Value<float>(PropOffsets_XEmitter.CoronaMaxScale); }
    vec3*& CoronaOffset() { return Value<vec3*>(PropOffsets_XEmitter.CoronaOffset); }
    float& CoronaScaling() { return Value<float>(PropOffsets_XEmitter.CoronaScaling); }
    UTexture*& CoronaTexture() { return Value<UTexture*>(PropOffsets_XEmitter.CoronaTexture); }
    float& CullDistance() { return Value<float>(PropOffsets_XEmitter.CullDistance); }
    float& CullDistanceFadeDist() { return Value<float>(PropOffsets_XEmitter.CullDistanceFadeDist); }
    ParticleSndType*& DestroySound() { return Value<ParticleSndType*>(PropOffsets_XEmitter.DestroySound); }
    Array<UXEmitter*> DestructCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.DestructCombiners); }
    uint8_t& DrawScaleCount() { return Value<uint8_t>(PropOffsets_XEmitter.DrawScaleCount); }
    float& FadeInMaxAmount() { return Value<float>(PropOffsets_XEmitter.FadeInMaxAmount); }
    float& FadeInTime() { return Value<float>(PropOffsets_XEmitter.FadeInTime); }
    float& FadeOutTime() { return Value<float>(PropOffsets_XEmitter.FadeOutTime); }
    UActor*& FinishedSpawningTrigger() { return Value<UActor*>(PropOffsets_XEmitter.FinishedSpawningTrigger); }
    Array<UXParticleForces*> ForcesList() { return DynamicArray<UXParticleForces*>(PropOffsets_XEmitter.ForcesList); }
    FixedArrayView<NameString, 4> ForcesTags() { return FixedArray<NameString, 4>(PropOffsets_XEmitter.ForcesTags); }
    float& HittingActorKickVelScale() { return Value<float>(PropOffsets_XEmitter.HittingActorKickVelScale); }
    ParticleSndType*& ImpactSound() { return Value<ParticleSndType*>(PropOffsets_XEmitter.ImpactSound); }
    Array<UXEmitter*> LifeTimeCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.LifeTimeCombiners); }
    FloatRange*& LifetimeRange() { return Value<FloatRange*>(PropOffsets_XEmitter.LifetimeRange); }
    float& LODFactor() { return Value<float>(PropOffsets_XEmitter.LODFactor); }
    float& MaxCoronaDistance() { return Value<float>(PropOffsets_XEmitter.MaxCoronaDistance); }
    int& MaxParticles() { return Value<int>(PropOffsets_XEmitter.MaxParticles); }
    float& MinBounceVelocity() { return Value<float>(PropOffsets_XEmitter.MinBounceVelocity); }
    float& MinImpactVelForSnd() { return Value<float>(PropOffsets_XEmitter.MinImpactVelForSnd); }
    float& NextParticleTime() { return Value<float>(PropOffsets_XEmitter.NextParticleTime); }
    vec3*& OldSpawnPosition() { return Value<vec3*>(PropOffsets_XEmitter.OldSpawnPosition); }
    RangeVector*& ParticleAcceleration() { return Value<RangeVector*>(PropOffsets_XEmitter.ParticleAcceleration); }
    vec3*& ParticleBounchyness() { return Value<vec3*>(PropOffsets_XEmitter.ParticleBounchyness); }
    EEmitterPartCol ParticleCollision() { return static_cast<EEmitterPartCol>(Value<uint8_t>(PropOffsets_XEmitter.ParticleCollision)); }
    RangeVector*& ParticleColor() { return Value<RangeVector*>(PropOffsets_XEmitter.ParticleColor); }
    FixedArrayView<ColorScaleRangeType*, 5> ParticleColorScale() { return FixedArray<ColorScaleRangeType*, 5>(PropOffsets_XEmitter.ParticleColorScale); }
    vec3*& ParticleExtent() { return Value<vec3*>(PropOffsets_XEmitter.ParticleExtent); }
    Array<UClass*> ParticleKillCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleKillCClass); }
    NameString& ParticleKillTag() { return Value<NameString>(PropOffsets_XEmitter.ParticleKillTag); }
    Array<UClass*> ParticleLifeTimeCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleLifeTimeCClass); }
    FloatRange*& ParticleLifeTimeSDelay() { return Value<FloatRange*>(PropOffsets_XEmitter.ParticleLifeTimeSDelay); }
    Array<UClass*> ParticleSpawnCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleSpawnCClass); }
    NameString& ParticleSpawnTag() { return Value<NameString>(PropOffsets_XEmitter.ParticleSpawnTag); }
    float& ParticlesPerSec() { return Value<float>(PropOffsets_XEmitter.ParticlesPerSec); }
    ERenderStyle ParticleStyle() { return static_cast<ERenderStyle>(Value<uint8_t>(PropOffsets_XEmitter.ParticleStyle)); }
    uint8_t& ParticleTexCount() { return Value<uint8_t>(PropOffsets_XEmitter.ParticleTexCount); }
    FixedArrayView<UTexture*, 16> ParticleTextures() { return FixedArray<UTexture*, 16>(PropOffsets_XEmitter.ParticleTextures); }
    Array<UClass*> ParticleWallHitCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleWallHitCClass); }
    NameString& ParticleWallHitTag() { return Value<NameString>(PropOffsets_XEmitter.ParticleWallHitTag); }
    float& PartSpriteForwardZ() { return Value<float>(PropOffsets_XEmitter.PartSpriteForwardZ); }
    Box*& RendBoundingBox() { return Value<Box*>(PropOffsets_XEmitter.RendBoundingBox); }
    float& ResetTimer() { return Value<float>(PropOffsets_XEmitter.ResetTimer); }
    RangeVector*& RevolutionOffset() { return Value<RangeVector*>(PropOffsets_XEmitter.RevolutionOffset); }
    vec3*& RevolutionOffsetUnAxis() { return Value<vec3*>(PropOffsets_XEmitter.RevolutionOffsetUnAxis); }
    RangeVector*& RevolutionsPerSec() { return Value<RangeVector*>(PropOffsets_XEmitter.RevolutionsPerSec); }
    Array<RevolveScaleType*> RevolutionTimeScale() { return DynamicArray<RevolveScaleType*>(PropOffsets_XEmitter.RevolutionTimeScale); }
    RangeVector*& Scale3DRange() { return Value<RangeVector*>(PropOffsets_XEmitter.Scale3DRange); }
    int& SingleIVert() { return Value<int>(PropOffsets_XEmitter.SingleIVert); }
    Array<UXEmitter*> SpawnCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.SpawnCombiners); }
    float& SpawnInterval() { return Value<float>(PropOffsets_XEmitter.SpawnInterval); }
    vec3*& SpawnOffsetMultiplier() { return Value<vec3*>(PropOffsets_XEmitter.SpawnOffsetMultiplier); }
    IntRange*& SpawnParts() { return Value<IntRange*>(PropOffsets_XEmitter.SpawnParts); }
    ESpawnPosType SpawnPosType() { return static_cast<ESpawnPosType>(Value<uint8_t>(PropOffsets_XEmitter.SpawnPosType)); }
    ParticleSndType*& SpawnSound() { return Value<ParticleSndType*>(PropOffsets_XEmitter.SpawnSound); }
    ESpawnPosType SpawnVelType() { return static_cast<ESpawnPosType>(Value<uint8_t>(PropOffsets_XEmitter.SpawnVelType)); }
    FixedArrayView<SpeedRangeType*, 5> SpeedScale() { return FixedArray<SpeedRangeType*, 5>(PropOffsets_XEmitter.SpeedScale); }
    uint8_t& SpeedScaleCount() { return Value<uint8_t>(PropOffsets_XEmitter.SpeedScaleCount); }
    Array<Speed3DType*> SpeedTimeScale3D() { return DynamicArray<Speed3DType*>(PropOffsets_XEmitter.SpeedTimeScale3D); }
    FloatRange*& SphereCylinderRange() { return Value<FloatRange*>(PropOffsets_XEmitter.SphereCylinderRange); }
    FloatRange*& SphereCylVelocity() { return Value<FloatRange*>(PropOffsets_XEmitter.SphereCylVelocity); }
    ESpriteAnimType SpriteAnimationType() { return static_cast<ESpriteAnimType>(Value<uint8_t>(PropOffsets_XEmitter.SpriteAnimationType)); }
    FloatRange*& StartingScale() { return Value<FloatRange*>(PropOffsets_XEmitter.StartingScale); }
    Array<UXEmitter*> TDestructC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TDestructC); }
    Array<Speed3DType*> TimeDrawScale3D() { return DynamicArray<Speed3DType*>(PropOffsets_XEmitter.TimeDrawScale3D); }
    FixedArrayView<ScaleRangeType*, 5> TimeScale() { return FixedArray<ScaleRangeType*, 5>(PropOffsets_XEmitter.TimeScale); }
    Array<UXEmitter*> TLifeTimeC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TLifeTimeC); }
    EEmitterTriggerType TriggerAction() { return static_cast<EEmitterTriggerType>(Value<uint8_t>(PropOffsets_XEmitter.TriggerAction)); }
    Array<UXEmitter*> TSpawnC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TSpawnC); }
    Array<UXEmitter*> TWallHitC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TWallHitC); }
    UActor*& UseActorCoords() { return Value<UActor*>(PropOffsets_XEmitter.UseActorCoords); }
    vec3*& VelocityLossRate() { return Value<vec3*>(PropOffsets_XEmitter.VelocityLossRate); }
    Box*& VertexLimitBBox() { return Value<Box*>(PropOffsets_XEmitter.VertexLimitBBox); }
    Box*& VisibilityBox() { return Value<Box*>(PropOffsets_XEmitter.VisibilityBox); }
    Array<UXEmitter*> WallHitCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.WallHitCombiners); }
    EHitEventType WallImpactAction() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XEmitter.WallImpactAction)); }
    EHitEventType WaterImpactAction() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XEmitter.WaterImpactAction)); }
};

class UXParticleForces : public UActor
{
public:
    using UActor::UActor;

    BitfieldBool bEnabled() { return BoolValue(PropOffsets_XParticleForces.bEnabled); }
    BitfieldBool bUseBoxForcePosition() { return BoolValue(PropOffsets_XParticleForces.bUseBoxForcePosition); }
    Box*& EffectingBox() { return Value<Box*>(PropOffsets_XParticleForces.EffectingBox); }
    float& EffectingRadius() { return Value<float>(PropOffsets_XParticleForces.EffectingRadius); }
    FloatRange*& EffectPartLifeTime() { return Value<FloatRange*>(PropOffsets_XParticleForces.EffectPartLifeTime); }
    NameString& OldTagName() { return Value<NameString>(PropOffsets_XParticleForces.OldTagName); }
};

class UVelocityForce : public UXParticleForces
{
public:
    using UXParticleForces::UXParticleForces;

    BitfieldBool bChangeAcceleration() { return BoolValue(PropOffsets_VelocityForce.bChangeAcceleration); }
    BitfieldBool bInstantChange() { return BoolValue(PropOffsets_VelocityForce.bInstantChange); }
    vec3*& VelocityToAdd() { return Value<vec3*>(PropOffsets_VelocityForce.VelocityToAdd); }
};

class UKillParticleForce : public UXParticleForces
{
public:
    using UXParticleForces::UXParticleForces;

    float& LifeTimeDrainAmount() { return Value<float>(PropOffsets_KillParticleForce.LifeTimeDrainAmount); }
};

class UParticleConcentrateForce : public UXParticleForces
{
public:
    using UXParticleForces::UXParticleForces;

    BitfieldBool bActorDistanceSuckIn() { return BoolValue(PropOffsets_ParticleConcentrateForce.bActorDistanceSuckIn); }
    BitfieldBool bSetsAcceleration() { return BoolValue(PropOffsets_ParticleConcentrateForce.bSetsAcceleration); }
    vec3*& CenterPointOffset() { return Value<vec3*>(PropOffsets_ParticleConcentrateForce.CenterPointOffset); }
    float& DrainSpeed() { return Value<float>(PropOffsets_ParticleConcentrateForce.DrainSpeed); }
    float& MaxDistance() { return Value<float>(PropOffsets_ParticleConcentrateForce.MaxDistance); }
};

// A resource class for particles to use
class UEmitterRC : public UActor
{
public:
    using UActor::UActor;
};

// A pretty much empty class that gets immediately destroyed on PreBeginPlay()
class UXTrailParticle : public UActor
{
public:
    using UActor::UActor;
};

class UXBeamEmitter : public UXEmitter
{
public:
    using UXEmitter::UXEmitter;

    BitfieldBool bDoBeamNoise() { return BoolValue(PropOffsets_XBeamEmitter.bDoBeamNoise); }
    BitfieldBool bDynamicNoise() { return BoolValue(PropOffsets_XBeamEmitter.bDynamicNoise); }
    Array<float> BeamPointScaling() { return DynamicArray<float>(PropOffsets_XBeamEmitter.BeamPointScaling); }
    Array<FBeamTargetPoint> BeamTarget() { return DynamicArray<FBeamTargetPoint>(PropOffsets_XBeamEmitter.BeamTarget); }
    UTexture*& EndTexture() { return Value<UTexture*>(PropOffsets_XBeamEmitter.EndTexture); }
    RangeVector*& NoiseRange() { return Value<RangeVector*>(PropOffsets_XBeamEmitter.NoiseRange); }
    float& NoiseSwapTime() { return Value<float>(PropOffsets_XBeamEmitter.NoiseSwapTime); }
    Array<ScaleRangeType> NoiseTimeScale() { return DynamicArray<ScaleRangeType>(PropOffsets_XBeamEmitter.NoiseTimeScale); }
    UMesh*& RenderDataModel() { return Value<UMesh*>(PropOffsets_XBeamEmitter.RenderDataModel); }
    uint8_t& Segments() { return Value<uint8_t>(PropOffsets_XBeamEmitter.Segments); }
    Array<float> SegmentScales() { return DynamicArray<float>(PropOffsets_XBeamEmitter.SegmentScales); }
    UTexture*& StartTexture() { return Value<UTexture*>(PropOffsets_XBeamEmitter.StartTexture); }
    FixedArrayView<float, 4> TextureUV() { return FixedArray<float, 4>(PropOffsets_XBeamEmitter.TextureUV); }
    float& TurnRate() { return Value<float>(PropOffsets_XBeamEmitter.TurnRate); }
};

class UXMeshEmitter : public UXEmitter
{
public:
    using UXEmitter::UXEmitter;

    UActor*& AnimateByActor() { return Value<UActor*>(PropOffsets_XMeshEmitter.AnimateByActor); }
    BitfieldBool bAnimateParticles() { return BoolValue(PropOffsets_XMeshEmitter.bAnimateParticles); }
    BitfieldBool bMeshEnvironmentMapping() { return BoolValue(PropOffsets_XMeshEmitter.bMeshEnvironmentMapping); }
    BitfieldBool bPartAnimLoop() { return BoolValue(PropOffsets_XMeshEmitter.bPartAnimLoop); }
    BitfieldBool bParticlesRandFrame() { return BoolValue(PropOffsets_XMeshEmitter.bParticlesRandFrame); }
    BitfieldBool bRelativeToMoveDir() { return BoolValue(PropOffsets_XMeshEmitter.bRelativeToMoveDir); }
    BitfieldBool bRenderParticles() { return BoolValue(PropOffsets_XMeshEmitter.bRenderParticles); }
    RangeVector*& InitialRot() { return Value<RangeVector*>(PropOffsets_XMeshEmitter.InitialRot); }
    float& PartAnimFrameStart() { return Value<float>(PropOffsets_XMeshEmitter.PartAnimFrameStart); }
    float& PartAnimRate() { return Value<float>(PropOffsets_XMeshEmitter.PartAnimRate); }
    NameString& ParticleAnim() { return Value<NameString>(PropOffsets_XMeshEmitter.ParticleAnim); }
    ByteRange*& ParticleFatness() { return Value<ByteRange*>(PropOffsets_XMeshEmitter.ParticleFatness); }
    UMesh*& ParticleMesh() { return Value<UMesh*>(PropOffsets_XMeshEmitter.ParticleMesh); }
    EEmPartRotType ParticleRotation() { return static_cast<EEmPartRotType>(Value<uint8_t>(PropOffsets_XMeshEmitter.ParticleRotation)); }
    Array<AnimationType> RandAnims() { return DynamicArray<AnimationType>(PropOffsets_XMeshEmitter.RandAnims); }
    RangeVector*& RotationsPerSec() { return Value<RangeVector*>(PropOffsets_XMeshEmitter.RotationsPerSec); }
};

class UXSpriteEmitter : public UXEmitter
{
public:
    using UXEmitter::UXEmitter;

    RangeVector*& InitialRot() { return Value<RangeVector*>(PropOffsets_XSpriteEmitter.InitialRot); }
    ESprPartRotType ParticleRotation() { return static_cast<ESprPartRotType>(Value<uint8_t>(PropOffsets_XSpriteEmitter.ParticleRotation)); }
    RangeVector*& RotationsPerSec() { return Value<RangeVector*>(PropOffsets_XSpriteEmitter.RotationsPerSec); }
    float& RotateByVelocityScale() { return Value<float>(PropOffsets_XSpriteEmitter.RotateByVelocityScale); }
    vec3*& RotNormal() { return Value<vec3*>(PropOffsets_XSpriteEmitter.RotNormal); }
};

class UXWeatherEmitter : public UXParticleEmitter
{
public:
    using UXParticleEmitter::UXParticleEmitter;

    RangeVector*& AppearArea() { return Value<RangeVector*>(PropOffsets_XWeatherEmitter.AppearArea); }
    EWeatherAreaType AppearAreaType() { return static_cast<EWeatherAreaType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.AppearAreaType)); }
    BitfieldBool bIsEnabled() { return BoolValue(PropOffsets_XWeatherEmitter.bIsEnabled); }
    BitfieldBool bParticleColorEnabled() { return BoolValue(PropOffsets_XWeatherEmitter.bParticleColorEnabled); }
    BitfieldBool bUseAreaSpawns() { return BoolValue(PropOffsets_XWeatherEmitter.bUseAreaSpawns); }
    Coords*& CachedCoords() { return Value<Coords*>(PropOffsets_XWeatherEmitter.CachedCoords); }
    FloatRange*& FadeOutDistance() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.FadeOutDistance); }
    vec3*& LastCamPosition() { return Value<vec3*>(PropOffsets_XWeatherEmitter.LastCamPosition); }
    FloatRange*& Lifetime() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.Lifetime); }
    float& NextParticleTime() { return Value<float>(PropOffsets_XWeatherEmitter.NextParticleTime); }
    Array<UXRainRestrictionVolume*> NoRainBounds() { return DynamicArray<UXRainRestrictionVolume*>(PropOffsets_XWeatherEmitter.NoRainBounds); }
    int& ParticleCount() { return Value<int>(PropOffsets_XWeatherEmitter.ParticleCount); }
    RangeVector*& ParticlesColor() { return Value<RangeVector*>(PropOffsets_XWeatherEmitter.ParticlesColor); }
    uint8_t& ParticleTexCount() { return Value<uint8_t>(PropOffsets_XWeatherEmitter.ParticleTexCount); }
    ERenderStyle PartStyle() { return static_cast<ERenderStyle>(Value<uint8_t>(PropOffsets_XWeatherEmitter.PartStyle)); }
    FixedArrayView<UTexture*, 8> PartTextures() { return FixedArray<UTexture*, 8>(PropOffsets_XWeatherEmitter.PartTextures); }
    RangeVector*& Position() { return Value<RangeVector*>(PropOffsets_XWeatherEmitter.Position); }
    UMesh*& SheetModel() { return Value<UMesh*>(PropOffsets_XWeatherEmitter.SheetModel); }
    FloatRange*& Size() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.Size); }
    float& SpawnInterval() { return Value<float>(PropOffsets_XWeatherEmitter.SpawnInterval); }
    FloatRange*& Speed() { return Value<FloatRange*>(PropOffsets_XWeatherEmitter.Speed); }
    Coords*& TransfrmCoords() { return Value<Coords*>(PropOffsets_XWeatherEmitter.TransfrmCoords); }
    FixedArrayView<vec3*, 2> VecArea() { return FixedArray<vec3*, 2>(PropOffsets_XWeatherEmitter.VecArea); }
    NameString& WallHitEmitter() { return Value<NameString>(PropOffsets_XWeatherEmitter.WallHitEmitter); }
    Array<UXEmitter*> WallHitEmitters() { return DynamicArray<UXEmitter*>(PropOffsets_XWeatherEmitter.WallHitEmitters); }
    EHitEventType WallHitEvent() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.WallHitEvent)); }
    float& WallHitMinZ() { return Value<float>(PropOffsets_XWeatherEmitter.WallHitMinZ); }
    NameString& WaterHitEmitter() { return Value<NameString>(PropOffsets_XWeatherEmitter.WaterHitEmitter); }
    Array<UXEmitter*> WaterHitEmitters() { return DynamicArray<UXEmitter*>(PropOffsets_XWeatherEmitter.WaterHitEmitters); }
    EHitEventType WaterHitEvent() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.WaterHitEvent)); }
    EFallingType WeatherType() { return static_cast<EFallingType>(Value<uint8_t>(PropOffsets_XWeatherEmitter.WeatherType)); }
};

class UXTrailEmitter : public UXEmitter
{
public:
    using UXEmitter::UXEmitter;

    BitfieldBool bDynamicParticleCount() { return BoolValue(PropOffsets_XTrailEmitter.bDynamicParticleCount); }
    BitfieldBool bSettingTrail() { return BoolValue(PropOffsets_XTrailEmitter.bSettingTrail); }
    BitfieldBool bSmoothEntryPoint() { return BoolValue(PropOffsets_XTrailEmitter.bSmoothEntryPoint); }
    BitfieldBool bTexContinous() { return BoolValue(PropOffsets_XTrailEmitter.bTexContinous); }
    float& MaxTrailLength() { return Value<float>(PropOffsets_XTrailEmitter.MaxTrailLength); }
    vec3*& OldTrailSport() { return Value<vec3*>(PropOffsets_XTrailEmitter.OldTrailSport); }
    UXTrailParticle*& ParticleData() { return Value<UXTrailParticle*>(PropOffsets_XTrailEmitter.ParticleData); }
    float& TexOffset() { return Value<float>(PropOffsets_XTrailEmitter.TexOffset); }
    FixedArrayView<float, 4> TextureUV() { return FixedArray<float, 4>(PropOffsets_XTrailEmitter.TextureUV); }
    Array<TrailOffsetPart> Trail() { return DynamicArray<TrailOffsetPart>(PropOffsets_XTrailEmitter.Trail); }
    float& TrailThreshold() { return Value<float>(PropOffsets_XTrailEmitter.TrailThreshold); }
    ETrailType TrailType() { return static_cast<ETrailType>(Value<uint8_t>(PropOffsets_XTrailEmitter.TrailType)); }
};

class UXRainRestrictionVolume : public UActor
{
public:
    using UActor::UActor;

    vec3*& BoundsMax() { return Value<vec3*>(PropOffsets_XRainRestrictionVolume.BoundsMax); }
    vec3*& BoundsMin() { return Value<vec3*>(PropOffsets_XRainRestrictionVolume.BoundsMin); }
};

class UEmitterGarbageCollector : public UInfo
{
public:
    using UInfo::UInfo;

    BitfieldBool bCleanUp() { return BoolValue(PropOffsets_EmitterGarbageCollector.bCleanUp); }
    float& CleanUpTime() { return Value<float>(PropOffsets_EmitterGarbageCollector.CleanUpTime); }
    void*& GarbagePtr() { return Value<void*>(PropOffsets_EmitterGarbageCollector.GarbagePtr); }
};

// This is in Emitter package for some reason.
// Supports unlimited lighting distance.
class UDistantLightActor : public ULight
{
public:
    using ULight::ULight;

    float& NewLightRadius() { return Value<float>(PropOffsets_DistantLightActor.NewLightRadius); }
};
