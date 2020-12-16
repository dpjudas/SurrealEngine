#pragma once

#include "UObject.h"
#include "Math/bbox.h"
#include "Math/vec.h"
#include "Math/quaternion.h"

class UTexture;
class UAnimation;

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
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	BBox BoundingBox;
	vec4 BoundingSphere = { 0.0f };
};

class UMesh : public UPrimitive
{
public:
	using UPrimitive::UPrimitive;
	void Load(ObjectStream* stream) override;

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
	using UMesh::UMesh;
	void Load(ObjectStream* stream) override;

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

struct ExtMeshWedge
{
	uint16_t Vertex;
	uint16_t Flags;
	float U;
	float V;
};

struct RefSkeletonBone
{
	std::string Name;
	uint32_t Flags;
	quaternion Orientation;
	vec3 Position;
	float Length;
	vec3 Size;
	uint32_t NumChildren;
	uint32_t ParentIndex;
};

struct BoneWeightIndex
{
	uint16_t WeightIndex;
	uint16_t Number;
	uint16_t DetailA;
	uint16_t DetailB;
};

struct BoneWeight
{
	uint16_t PointIndex;
	uint16_t BoneWeight;
};

class USkeletalMesh : public ULodMesh
{
public:
	using ULodMesh::ULodMesh;
	void Load(ObjectStream* stream) override;

	std::vector<ExtMeshWedge> ExtWedges;
	std::vector<vec3> Points;
	std::vector<RefSkeletonBone> RefSkeleton;
	std::vector<BoneWeightIndex> BoneWeightIndices;
	std::vector<BoneWeight> BoneWeights;
	std::vector<vec3> LocalPoints;

	uint32_t SkeletalDepth = 0;
	UAnimation* DefaultAnimation = nullptr;
	uint32_t WeaponBoneIndex = 0;

	struct
	{
		vec3 Origin;
		vec3 XAxis;
		vec3 YAxis;
		vec3 ZAxis;
	} WeaponAdjust;
};

struct RefBone
{
	std::string Name;
	uint32_t Flags = 0;
	uint32_t ParentIndex = 0;
};

struct AnimTrack
{
	uint32_t Flags = 0;
	std::vector<quaternion> KeyQuat;
	std::vector<vec3> KeyPos;
	std::vector<float> KeyTime;
	struct
	{
		uint32_t Flags = 0;
		std::vector<quaternion> KeyQuat;
		std::vector<vec3> KeyPos;
		std::vector<float> KeyTime;
	} RootTrack;
};

struct AnimMove
{
	vec3 RootSpeed3D;
	float TrackTime = 0.0f;
	uint32_t StartBone = 0;
	std::vector<uint32_t> BoneIndices;
	std::vector<AnimTrack> AnimTracks;
};

class UAnimation : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<RefBone> RefBones;
	std::vector<AnimMove> Moves;
};
