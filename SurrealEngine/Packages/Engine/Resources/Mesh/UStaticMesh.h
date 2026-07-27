#pragma once

#include "UMesh.h"

struct StaticMeshLODLevel
{
	float LODBias = 0.0;
	UStaticMesh* StaticMesh = nullptr;
};

class UStaticMesh : public UMesh
{
public:
	using UMesh::UMesh;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	bool FlipNormals = false;
	Array<StaticMeshLODLevel> LODLevels;
	Array<MeshMaterial> Materials;
	bool SmoothCorners = false;
	bool UseSimpleBoxCollision = false;
	bool UseSimpleLineCollision = false;
};
