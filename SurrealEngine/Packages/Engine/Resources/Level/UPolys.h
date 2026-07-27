#pragma once

#include "Packages/Core/UObject.h"

class UBrush;
class UTexture;

struct Poly
{
	vec3 Base;
	vec3 Normal;
	vec3 TextureU;
	vec3 TextureV;
	Array<vec3> Vertices;
	uint32_t PolyFlags;
	UBrush* Actor;
	UTexture* Texture;
	NameString ItemName;
	int LinkIndex;
	int BrushPolyIndex;
	int16_t PanU;
	int16_t PanV;
};

class UPolys : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<Poly> Polys;
};
