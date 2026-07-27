#pragma once

#include "UModel.h"

class UVectors : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<vec3> Vectors;
};
