#pragma once

#include "Packages/Engine/Resources/UPrimitive.h"

class UTexture;
class UAnimation;
class UStaticMesh;

struct MeshTri
{
	uint16_t Indices[3];
	uvec2b UV[3];
	uint32_t PolyFlags;
	int32_t TextureIndex;
};

struct MeshAnimNotify
{
	float Time;
	NameString Function;
};

struct MeshAnimSeq
{
	NameString Name;
	NameString Group;
	int StartFrame;
	int NumFrames;
	float Rate;
	Array<MeshAnimNotify> Notifys;
};

struct MeshVertConnect
{
	int NumVertTriangles;
	uint32_t TriangleListOffset;
};

struct MeshFace
{
	uint16_t Indices[3];
	uint16_t MaterialIndex;
};

struct MeshWedge
{
	uint16_t Vertex;
	uint8_t U;
	uint8_t V;
};

struct MeshMaterial
{
	uint32_t PolyFlags = 0;
	int TextureIndex = 0;
};

class UMesh : public UPrimitive
{
public:
	using UPrimitive::UPrimitive;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	MeshAnimSeq* GetSequence(const NameString& name)
	{
		for (MeshAnimSeq& seq : AnimSeqs)
			if (seq.Name == name)
				return &seq;
		return !AnimSeqs.empty() ? AnimSeqs.data() : nullptr;
	}

	Array<vec3> Verts;
	Array<MeshTri> Tris;
	Array<MeshAnimSeq> AnimSeqs;
	Array<MeshVertConnect> Connects;
	Array<BBox> BoundingBoxes;
	Array<vec4> BoundingSpheres;
	BBox MeshBoundingBox;
	vec4 MeshBoundingSphere;
	Array<int> VertLinks;
	Array<UTexture*> Textures;
	Array<float> TextureLOD;
	int FrameVerts = 0;
	int AnimFrames = 0;
	uint32_t AndFlags = 0;
	uint32_t OrFlags = 0;
	vec3 Scale = { 0.0f };
	vec3 Origin = { 0.0f };
	Rotator RotOrigin;
	uint32_t CurPoly = 0;
	uint32_t CurVertex = 0;

	Array<vec3> Normals;
	mat4 meshToObject;
};
