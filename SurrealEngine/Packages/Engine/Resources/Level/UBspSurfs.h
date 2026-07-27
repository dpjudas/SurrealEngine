#pragma once

#include "UModel.h"

class UBspSurfs : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<BspSurface> Surfaces;
};
