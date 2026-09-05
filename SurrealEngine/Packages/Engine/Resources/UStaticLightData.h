#pragma once

#include "Packages/Core/UObject.h"

// Unreal 227 class
// Attached to static meshes?
class UStaticLightData : public UObject
{
public:
    using UObject::UObject;

    BitfieldBool HasLightmap() { return BoolValue(PropOffsets_StaticLightData.HasLightmap); }
};
