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

class UPrimitive : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	BBox BoundingBox;
	vec4 BoundingSphere = { 0.0f };
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
		return AnimSeqs.data();
	}

	Array<vec3> Verts;
	Array<MeshTri> Tris;
	Array<MeshAnimSeq> AnimSeqs;
	Array<MeshVertConnect> Connects;
	Array<BBox> BoundingBoxes;
	Array<vec4> BoundingSpheres;
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

struct ExtMeshWedge
{
	uint16_t Vertex;
	uint16_t Flags;
	float U;
	float V;
};

struct RefSkeletonBone
{
	NameString Name;
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
	void Save(PackageStreamWriter* stream) override;

	Array<ExtMeshWedge> ExtWedges;
	Array<vec3> Points;
	Array<RefSkeletonBone> RefSkeleton;
	Array<BoneWeightIndex> BoneWeightIndices;
	Array<BoneWeight> BoneWeights;
	Array<vec3> LocalPoints;

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
	NameString Name;
	uint32_t Flags = 0;
	uint32_t ParentIndex = 0;
};

struct AnimTrack
{
	uint32_t Flags = 0;
	Array<quaternion> KeyQuat;
	Array<vec3> KeyPos;
	Array<float> KeyTime;
};

struct AnimMove
{
	vec3 RootSpeed3D;
	float TrackTime = 0.0f;
	uint32_t StartBone = 0;
	uint32_t Flags = 0;
	Array<uint32_t> BoneIndices;
	Array<AnimTrack> AnimTracks;
	AnimTrack RootTrack;
};

class UAnimation : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<RefBone> RefBones;
	Array<AnimMove> Moves;
};
