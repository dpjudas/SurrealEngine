#pragma once

#include "UActor.h"
#include "Math/quaternion.h"

class USkeletalMesh;
class UAnimation;

struct Matrix
{
    vec4 XPlane;
    vec4 YPlane;
    vec4 ZPlane;
    vec4 WPlane;
};

struct MeshModifierType
{
    vec3 boneScale;
    Coords CoordsMod, OrgCoords;
    int RootB;
    uint8_t bInUse;
};

struct AnimationChannel
{
    int RootBoneIndex;
    int MChunkPtr, ASeqPtr;
    float Rate, Frame, FrameStart, TweenOut, Alpha;
    quaternion LastAnimQuat;
    vec3 LastAnimTrack;
    uint8_t bLoop, bTweenIn, bNotifies;
    NameString AnimName;
};

class U227SkeletalMeshInstance : public UObject
{
public:
    using UObject::UObject;

    TypedScriptArray<Coords> SpaceBases() { return DynamicArray<Coords>(PropOffsets_SkeletalMeshInstance.SpaceBases); }
    TypedScriptArray<int> CachedLinks() { return DynamicArray<int>(PropOffsets_SkeletalMeshInstance.CachedLinks); }
    uint8_t& bHasUpdated() { return Value<uint8_t>(PropOffsets_SkeletalMeshInstance.bHasUpdated); }
    USkeletalMesh*& LastDrawnMesh() { return Value<USkeletalMesh*>(PropOffsets_SkeletalMeshInstance.LastDrawnMesh); }
    UAnimation*& CachedAnim() { return Value<UAnimation*>(PropOffsets_SkeletalMeshInstance.CachedAnim); }
    TypedScriptArray<Coords> CachedOrientations() { return DynamicArray<Coords>(PropOffsets_SkeletalMeshInstance.CachedOrientations); }
    TypedScriptArray<vec3*> CachedPositions() { return DynamicArray<vec3*>(PropOffsets_SkeletalMeshInstance.CachedPositions); }
    float& TweenStartFrame() { return Value<float>(PropOffsets_SkeletalMeshInstance.TweenStartFrame); }
    Matrix*& Base() { return Value<Matrix*>(PropOffsets_SkeletalMeshInstance.Base); }
    uint8_t& bHasCachedFrame() { return Value<uint8_t>(PropOffsets_SkeletalMeshInstance.bHasCachedFrame); }
    uint8_t& bWasTweening() { return Value<uint8_t>(PropOffsets_SkeletalMeshInstance.bWasTweening); }
    NameString& CachedTweenSeq() { return Value<NameString>(PropOffsets_SkeletalMeshInstance.CachedTweenSeq); }
    TypedScriptArray<MeshModifierType*> Modifiers() { return DynamicArray<MeshModifierType*>(PropOffsets_SkeletalMeshInstance.Modifiers); }
    TypedScriptArray<AnimationChannel*> Channels() { return DynamicArray<AnimationChannel*>(PropOffsets_SkeletalMeshInstance.Channels); }
    int& TChannelPtr() { return Value<int>(PropOffsets_SkeletalMeshInstance.TChannelPtr); }
    TypedScriptArray<UActor*> AttachedActors() { return DynamicArray<UActor*>(PropOffsets_SkeletalMeshInstance.AttachedActors); }
    int& AttachedBoneIndex() { return Value<int>(PropOffsets_SkeletalMeshInstance.AttachedBoneIndex); }
    NameString& AttachedBoneName() { return Value<NameString>(PropOffsets_SkeletalMeshInstance.AttachedBoneName); }
    UActor*& MyAttachment() { return Value<UActor*>(PropOffsets_SkeletalMeshInstance.MyAttachment); }
    uint8_t& HardAttachFlags() { return Value<uint8_t>(PropOffsets_SkeletalMeshInstance.HardAttachFlags); }
};