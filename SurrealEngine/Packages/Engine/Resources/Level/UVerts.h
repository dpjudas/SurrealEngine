#pragma once

#include "UModel.h"

class UVerts : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<BspVert> Vertices;
	int32_t NumSharedSides;
};
