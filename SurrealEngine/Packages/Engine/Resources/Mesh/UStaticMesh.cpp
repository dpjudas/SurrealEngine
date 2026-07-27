
#include "Precomp.h"
#include "UStaticMesh.h"
#include "Engine.h"

void UStaticMesh::Load(ObjectStream* stream)
{
	LogUnimplemented("Static Mesh loading");

	// UMesh::Load() doesn't work for some reason
	// We use UPrimitive::Load() for now so that we at least get an "Outer".
	UPrimitive::Load(stream);

	/*
	FlipNormals = stream->ReadUInt8();

	const int NumLODLevels = stream->ReadIndex();
	for (int i = 0 ; i < NumLODLevels ; i++)
	{
		StaticMeshLODLevel LODLevel;
		LODLevel.LODBias = stream->ReadFloat();
		LODLevel.StaticMesh = stream->ReadObject<UStaticMesh>();
		LODLevels.push_back(LODLevel);
	}

	const int NumMaterials = stream->ReadIndex();
	for (int i = 0; i < NumMaterials; i++)
	{
		MeshMaterial material;
		material.PolyFlags = stream->ReadUInt32();
		material.TextureIndex = stream->ReadInt32();
		Materials.push_back(material);
	}

	SmoothCorners = stream->ReadUInt8();
	UseSimpleBoxCollision = stream->ReadUInt8();
	UseSimpleLineCollision = stream->ReadUInt8();
	*/
}

void UStaticMesh::Save(PackageStreamWriter* stream)
{
	LogUnimplemented("Static Mesh saving");
}
