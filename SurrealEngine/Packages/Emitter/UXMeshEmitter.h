#pragma once

#include "UXEmitter.h"

enum class EEmPartRotType : uint8_t
{
	MEP_DesiredRot,
	MEP_FacingCamera,
	MEP_YawingToCamera
};

struct AnimationType
{
	NameString AnimSeq;
	float Frame, Rate;
	bool bAnimLoop;
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
	TypedScriptArray<AnimationType> RandAnims() { return DynamicArray<AnimationType>(PropOffsets_XMeshEmitter.RandAnims); }
	RangeVector*& RotationsPerSec() { return Value<RangeVector*>(PropOffsets_XMeshEmitter.RotationsPerSec); }
};
