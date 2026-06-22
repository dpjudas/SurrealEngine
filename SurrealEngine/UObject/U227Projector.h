#pragma once

#include "UActor.h"

class U227Projector : public UActor
{
public:
    using UActor::UActor;

    FixedArrayView<vec4*, 6> FrustumPlanes() { return FixedArray<vec4*, 6>(PropOffsets_Projector.FrustumPlanes); }
    //Array<UActor*> DecalActors() { return DynamicArray<UActor*>(PropOffsets_Projector.DecalActors); }
    //Array<int> DecalNodes() { return DynamicArray<int>(PropOffsets_Projector.DecalNodes); }
    BoundingBox& Box() { return Value<BoundingBox>(PropOffsets_Projector.Box); }
    FixedArrayView<vec3*, 8> VisBox() { return FixedArray<vec3*, 8>(PropOffsets_Projector.VisBox); }
    // Pointer TexData() { return Value<Pointer>(PropOffsets_Projector.TexData); }

    UTexture*& ProjectTexture() { return Value<UTexture*>(PropOffsets_Projector.ProjectTexture); }
    uint8_t& FOV() { return Value<uint8_t>(PropOffsets_Projector.FOV); }
    float& MaxDistance() { return Value<float>(PropOffsets_Projector.MaxDistance); }
    float& ProjectorScale() { return Value<float>(PropOffsets_Projector.ProjectorScale); }
    ERenderStyle ProjectStyle() { return static_cast<ERenderStyle>(Value<uint8_t>(PropOffsets_Projector.ProjectStyle)); }

    BitfieldBool bProjectActors() { return BoolValue(PropOffsets_Projector.bProjectActors); }
    BitfieldBool bProjectBSPBackfaces() { return BoolValue(PropOffsets_Projector.bProjectBSPBackfaces); }
    BitfieldBool bProjectMeshBackfaces() { return BoolValue(PropOffsets_Projector.bProjectMeshBackfaces); }
    BitfieldBool bProjectBSP() { return BoolValue(PropOffsets_Projector.bProjectBSP); }
    BitfieldBool bGradualFade() { return BoolValue(PropOffsets_Projector.bGradualFade); }
    BitfieldBool bUseBetterActorAttach() { return BoolValue(PropOffsets_Projector.bUseBetterActorAttach); }
    BitfieldBool bHasAttached() { return BoolValue(PropOffsets_Projector.bHasAttached); }
    BitfieldBool bProjecting() { return BoolValue(PropOffsets_Projector.bProjecting); }
};
