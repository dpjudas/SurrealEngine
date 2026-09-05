#pragma once

#include "UActor.h"
#include "Packages/Engine/Resources/UStaticLightData.h"

// Unreal 227 class
// UnrealEd adds this actor when one selects the "Add Mesh Here" option
class UStaticMeshActor : public UActor
{
public:
    using UActor::UActor;

    UStaticLightData*& StaticLightD() { return Value<UStaticLightData*>(PropOffsets_StaticMeshActor.StaticLightD); }
    BitfieldBool bBuildStaticLights() { return BoolValue(PropOffsets_StaticMeshActor.bBuildStaticLights); }
    BitfieldBool bComputeUnlitColor() { return BoolValue(PropOffsets_StaticMeshActor.bComputeUnlitColor); }
};
