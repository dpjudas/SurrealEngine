#pragma once

#include "UXParticleEmitter.h"

class UXParticleForces;

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
	TypedScriptArray<UXEmitter*> DestructCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.DestructCombiners); }
	uint8_t& DrawScaleCount() { return Value<uint8_t>(PropOffsets_XEmitter.DrawScaleCount); }
	float& FadeInMaxAmount() { return Value<float>(PropOffsets_XEmitter.FadeInMaxAmount); }
	float& FadeInTime() { return Value<float>(PropOffsets_XEmitter.FadeInTime); }
	float& FadeOutTime() { return Value<float>(PropOffsets_XEmitter.FadeOutTime); }
	UActor*& FinishedSpawningTrigger() { return Value<UActor*>(PropOffsets_XEmitter.FinishedSpawningTrigger); }
	TypedScriptArray<UXParticleForces*> ForcesList() { return DynamicArray<UXParticleForces*>(PropOffsets_XEmitter.ForcesList); }
	FixedArrayView<NameString, 4> ForcesTags() { return FixedArray<NameString, 4>(PropOffsets_XEmitter.ForcesTags); }
	float& HittingActorKickVelScale() { return Value<float>(PropOffsets_XEmitter.HittingActorKickVelScale); }
	ParticleSndType*& ImpactSound() { return Value<ParticleSndType*>(PropOffsets_XEmitter.ImpactSound); }
	TypedScriptArray<UXEmitter*> LifeTimeCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.LifeTimeCombiners); }
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
	TypedScriptArray<UClass*> ParticleKillCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleKillCClass); }
	NameString& ParticleKillTag() { return Value<NameString>(PropOffsets_XEmitter.ParticleKillTag); }
	TypedScriptArray<UClass*> ParticleLifeTimeCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleLifeTimeCClass); }
	FloatRange*& ParticleLifeTimeSDelay() { return Value<FloatRange*>(PropOffsets_XEmitter.ParticleLifeTimeSDelay); }
	TypedScriptArray<UClass*> ParticleSpawnCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleSpawnCClass); }
	NameString& ParticleSpawnTag() { return Value<NameString>(PropOffsets_XEmitter.ParticleSpawnTag); }
	float& ParticlesPerSec() { return Value<float>(PropOffsets_XEmitter.ParticlesPerSec); }
	ERenderStyle ParticleStyle() { return static_cast<ERenderStyle>(Value<uint8_t>(PropOffsets_XEmitter.ParticleStyle)); }
	uint8_t& ParticleTexCount() { return Value<uint8_t>(PropOffsets_XEmitter.ParticleTexCount); }
	FixedArrayView<UTexture*, 16> ParticleTextures() { return FixedArray<UTexture*, 16>(PropOffsets_XEmitter.ParticleTextures); }
	TypedScriptArray<UClass*> ParticleWallHitCClass() { return DynamicArray<UClass*>(PropOffsets_XEmitter.ParticleWallHitCClass); }
	NameString& ParticleWallHitTag() { return Value<NameString>(PropOffsets_XEmitter.ParticleWallHitTag); }
	float& PartSpriteForwardZ() { return Value<float>(PropOffsets_XEmitter.PartSpriteForwardZ); }
	Box*& RendBoundingBox() { return Value<Box*>(PropOffsets_XEmitter.RendBoundingBox); }
	float& ResetTimer() { return Value<float>(PropOffsets_XEmitter.ResetTimer); }
	RangeVector*& RevolutionOffset() { return Value<RangeVector*>(PropOffsets_XEmitter.RevolutionOffset); }
	vec3*& RevolutionOffsetUnAxis() { return Value<vec3*>(PropOffsets_XEmitter.RevolutionOffsetUnAxis); }
	RangeVector*& RevolutionsPerSec() { return Value<RangeVector*>(PropOffsets_XEmitter.RevolutionsPerSec); }
	TypedScriptArray<RevolveScaleType*> RevolutionTimeScale() { return DynamicArray<RevolveScaleType*>(PropOffsets_XEmitter.RevolutionTimeScale); }
	RangeVector*& Scale3DRange() { return Value<RangeVector*>(PropOffsets_XEmitter.Scale3DRange); }
	int& SingleIVert() { return Value<int>(PropOffsets_XEmitter.SingleIVert); }
	TypedScriptArray<UXEmitter*> SpawnCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.SpawnCombiners); }
	float& SpawnInterval() { return Value<float>(PropOffsets_XEmitter.SpawnInterval); }
	vec3*& SpawnOffsetMultiplier() { return Value<vec3*>(PropOffsets_XEmitter.SpawnOffsetMultiplier); }
	IntRange*& SpawnParts() { return Value<IntRange*>(PropOffsets_XEmitter.SpawnParts); }
	ESpawnPosType SpawnPosType() { return static_cast<ESpawnPosType>(Value<uint8_t>(PropOffsets_XEmitter.SpawnPosType)); }
	ParticleSndType*& SpawnSound() { return Value<ParticleSndType*>(PropOffsets_XEmitter.SpawnSound); }
	ESpawnPosType SpawnVelType() { return static_cast<ESpawnPosType>(Value<uint8_t>(PropOffsets_XEmitter.SpawnVelType)); }
	FixedArrayView<SpeedRangeType*, 5> SpeedScale() { return FixedArray<SpeedRangeType*, 5>(PropOffsets_XEmitter.SpeedScale); }
	uint8_t& SpeedScaleCount() { return Value<uint8_t>(PropOffsets_XEmitter.SpeedScaleCount); }
	TypedScriptArray<Speed3DType*> SpeedTimeScale3D() { return DynamicArray<Speed3DType*>(PropOffsets_XEmitter.SpeedTimeScale3D); }
	FloatRange*& SphereCylinderRange() { return Value<FloatRange*>(PropOffsets_XEmitter.SphereCylinderRange); }
	FloatRange*& SphereCylVelocity() { return Value<FloatRange*>(PropOffsets_XEmitter.SphereCylVelocity); }
	ESpriteAnimType SpriteAnimationType() { return static_cast<ESpriteAnimType>(Value<uint8_t>(PropOffsets_XEmitter.SpriteAnimationType)); }
	FloatRange*& StartingScale() { return Value<FloatRange*>(PropOffsets_XEmitter.StartingScale); }
	TypedScriptArray<UXEmitter*> TDestructC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TDestructC); }
	TypedScriptArray<Speed3DType*> TimeDrawScale3D() { return DynamicArray<Speed3DType*>(PropOffsets_XEmitter.TimeDrawScale3D); }
	FixedArrayView<ScaleRangeType*, 5> TimeScale() { return FixedArray<ScaleRangeType*, 5>(PropOffsets_XEmitter.TimeScale); }
	TypedScriptArray<UXEmitter*> TLifeTimeC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TLifeTimeC); }
	EEmitterTriggerType TriggerAction() { return static_cast<EEmitterTriggerType>(Value<uint8_t>(PropOffsets_XEmitter.TriggerAction)); }
	TypedScriptArray<UXEmitter*> TSpawnC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TSpawnC); }
	TypedScriptArray<UXEmitter*> TWallHitC() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.TWallHitC); }
	UActor*& UseActorCoords() { return Value<UActor*>(PropOffsets_XEmitter.UseActorCoords); }
	vec3*& VelocityLossRate() { return Value<vec3*>(PropOffsets_XEmitter.VelocityLossRate); }
	Box*& VertexLimitBBox() { return Value<Box*>(PropOffsets_XEmitter.VertexLimitBBox); }
	Box*& VisibilityBox() { return Value<Box*>(PropOffsets_XEmitter.VisibilityBox); }
	TypedScriptArray<UXEmitter*> WallHitCombiners() { return DynamicArray<UXEmitter*>(PropOffsets_XEmitter.WallHitCombiners); }
	EHitEventType WallImpactAction() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XEmitter.WallImpactAction)); }
	EHitEventType WaterImpactAction() { return static_cast<EHitEventType>(Value<uint8_t>(PropOffsets_XEmitter.WaterImpactAction)); }
};
