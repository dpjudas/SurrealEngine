#pragma once

#include "UObject.h"
#include "FrustumPlanes.h"

class UTexture;

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
	std::string Function;
};

struct MeshAnimSeq
{
	std::string Name;
	std::string Group;
	int StartFrame;
	int NumFrames;
	float Rate;
	std::vector<MeshAnimNotify> Notifys;
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

class UPrimitive : public UObject
{
public:
	UPrimitive(ObjectStream* stream);

	BBox BoundingBox;
	vec4 BoundingSphere = { 0.0f };
};

class UMesh : public UPrimitive
{
public:
	UMesh(ObjectStream* stream);

	std::vector<vec3> Verts;
	std::vector<MeshTri> Tris;
	std::vector<MeshAnimSeq> AnimSeqs;
	std::vector<MeshVertConnect> Connects;
	std::vector<BBox> BoundingBoxes;
	std::vector<vec4> BoundingSpheres;
	std::vector<int> VertLinks;
	std::vector<UTexture*> Textures;
	std::vector<float> TextureLOD;
	int FrameVerts = 0;
	int AnimFrames = 0;
	uint32_t AndFlags = 0;
	uint32_t OrFlags = 0;
	vec3 Scale = { 0.0f };
	vec3 Origin = { 0.0f };
	Rotator RotOrigin;
	uint32_t CurPoly = 0;
	uint32_t CurVertex = 0;
};

class ULodMesh : public UMesh
{
public:
	ULodMesh(ObjectStream* stream);

	std::vector<uint16_t> CollapsePointThus;
	std::vector<uint16_t> FaceLevel;
	std::vector<MeshFace> Faces;
	std::vector<uint16_t> CollapseWedgeThus;
	std::vector<MeshWedge> Wedges;
	std::vector<MeshMaterial> Materials;
	std::vector<MeshFace> SpecialFaces;
	uint32_t ModelVerts = 0;
	uint32_t SpecialVerts = 0;
	float MeshScaleMax = 0.0f;
	float LODHysteresis = 0.0f;
	float LODStrength = 0.0f;
	uint32_t LODMinVerts = 0;
	float LODMorph = 0.0f;
	float LODZDisplace = 0.0f;
	std::vector<uint16_t> ReMapAnimVerts;
	uint32_t OldFrameVerts = 0;
};
