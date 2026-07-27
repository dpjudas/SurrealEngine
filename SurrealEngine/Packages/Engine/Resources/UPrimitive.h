#pragma once

#include "Packages/Core/UObject.h"
#include "Math/bbox.h"
#include "Math/vec.h"
#include "Math/quaternion.h"

class UPrimitive : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	BBox BoundingBox;
	vec4 BoundingSphere = { 0.0f };
};
