#pragma once

#include "UMesh.h"

class ULodMesh : public UMesh
{
public:
	using UMesh::UMesh;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<uint16_t> CollapsePointThus;
	Array<uint16_t> FaceLevel;
	Array<MeshFace> Faces;
	Array<uint16_t> CollapseWedgeThus;
	Array<MeshWedge> Wedges;
	Array<MeshMaterial> Materials;
	Array<MeshFace> SpecialFaces;
	uint32_t ModelVerts = 0;
	uint32_t SpecialVerts = 0;
	float MeshScaleMax = 0.0f;
	float LODHysteresis = 0.0f;
	float LODStrength = 0.0f;
	uint32_t LODMinVerts = 0;
	float LODMorph = 0.0f;
	float LODZDisplace = 0.0f;
	Array<uint16_t> ReMapAnimVerts;
	uint32_t OldFrameVerts = 0;
};
