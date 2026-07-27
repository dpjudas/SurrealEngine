
#include "Precomp.h"
#include "USkeletalMesh.h"
#include "Packages/Engine/Resources/Mesh/UAnimation.h"

void USkeletalMesh::Load(ObjectStream* stream)
{
	ULodMesh::Load(stream);

	int NumExtWedges = stream->ReadIndex();
	for (int i = 0; i < NumExtWedges; i++)
	{
		ExtMeshWedge wedge;
		wedge.Vertex = stream->ReadUInt16();
		wedge.Flags = stream->ReadUInt16();
		wedge.U = stream->ReadFloat();
		wedge.V = stream->ReadFloat();
		ExtWedges.push_back(wedge);
	}

	int NumPoints = stream->ReadIndex();
	for (int i = 0; i < NumPoints; i++)
	{
		vec3 p;
		p.x = stream->ReadFloat();
		p.y = stream->ReadFloat();
		p.z = stream->ReadFloat();
		Points.push_back(p);
	}

	int NumRefSkeletonBones = stream->ReadIndex();
	for (int i = 0; i < NumRefSkeletonBones; i++)
	{
		RefSkeletonBone bone;
		bone.Name = stream->ReadName();
		bone.Flags = stream->ReadUInt32();
		bone.Orientation.x = stream->ReadFloat();
		bone.Orientation.y = stream->ReadFloat();
		bone.Orientation.z = stream->ReadFloat();
		bone.Orientation.w = stream->ReadFloat();
		bone.Position.x = stream->ReadFloat();
		bone.Position.y = stream->ReadFloat();
		bone.Position.z = stream->ReadFloat();
		bone.Length = stream->ReadFloat();
		bone.Size.x = stream->ReadFloat();
		bone.Size.y = stream->ReadFloat();
		bone.Size.z = stream->ReadFloat();
		bone.NumChildren = stream->ReadUInt32();
		bone.ParentIndex = stream->ReadUInt32();
		RefSkeleton.push_back(bone);
	}

	int NumBoneWeightIndices = stream->ReadIndex();
	for (int i = 0; i < NumBoneWeightIndices; i++)
	{
		BoneWeightIndex index;
		index.WeightIndex = stream->ReadUInt16();
		index.Number = stream->ReadUInt16();
		index.DetailA = stream->ReadUInt16();
		index.DetailB = stream->ReadUInt16();
		BoneWeightIndices.push_back(index);
	}

	int NumBoneWeights = stream->ReadIndex();
	for (int i = 0; i < NumBoneWeights; i++)
	{
		BoneWeight weight;
		weight.PointIndex = stream->ReadUInt16();
		weight.BoneWeight = stream->ReadUInt16();
		BoneWeights.push_back(weight);
	}

	int NumLocalPoints = stream->ReadIndex();
	for (int i = 0; i < NumLocalPoints; i++)
	{
		vec3 p;
		p.x = stream->ReadFloat();
		p.y = stream->ReadFloat();
		p.z = stream->ReadFloat();
		LocalPoints.push_back(p);
	}

	SkeletalDepth = stream->ReadUInt32();
	DefaultAnimation = stream->ReadObject<UAnimation>();
	WeaponBoneIndex = stream->ReadUInt32();

	WeaponAdjust.Origin.x = stream->ReadFloat();
	WeaponAdjust.Origin.y = stream->ReadFloat();
	WeaponAdjust.Origin.z = stream->ReadFloat();
	WeaponAdjust.XAxis.x = stream->ReadFloat();
	WeaponAdjust.XAxis.y = stream->ReadFloat();
	WeaponAdjust.XAxis.z = stream->ReadFloat();
	WeaponAdjust.YAxis.x = stream->ReadFloat();
	WeaponAdjust.YAxis.y = stream->ReadFloat();
	WeaponAdjust.YAxis.z = stream->ReadFloat();
	WeaponAdjust.ZAxis.x = stream->ReadFloat();
	WeaponAdjust.ZAxis.y = stream->ReadFloat();
	WeaponAdjust.ZAxis.z = stream->ReadFloat();
}

void USkeletalMesh::Save(PackageStreamWriter* stream)
{
	ULodMesh::Save(stream);

	stream->WriteIndex((int)ExtWedges.size());
	for (const ExtMeshWedge& wedge : ExtWedges)
	{
		stream->WriteUInt16(wedge.Vertex);
		stream->WriteUInt16(wedge.Flags);
		stream->WriteFloat(wedge.U);
		stream->WriteFloat(wedge.V);
	}

	stream->WriteIndex((int)Points.size());
	for (const vec3& p : Points)
	{
		stream->WriteFloat(p.x);
		stream->WriteFloat(p.y);
		stream->WriteFloat(p.z);
	}

	stream->WriteIndex((int)RefSkeleton.size());
	for (const RefSkeletonBone& bone : RefSkeleton)
	{
		stream->WriteName(bone.Name);
		stream->WriteUInt32(bone.Flags);
		stream->WriteFloat(bone.Orientation.x);
		stream->WriteFloat(bone.Orientation.y);
		stream->WriteFloat(bone.Orientation.z);
		stream->WriteFloat(bone.Orientation.w);
		stream->WriteFloat(bone.Position.x);
		stream->WriteFloat(bone.Position.y);
		stream->WriteFloat(bone.Position.z);
		stream->WriteFloat(bone.Length);
		stream->WriteFloat(bone.Size.x);
		stream->WriteFloat(bone.Size.y);
		stream->WriteFloat(bone.Size.z);
		stream->WriteUInt32(bone.NumChildren);
		stream->WriteUInt32(bone.ParentIndex);
	}

	stream->WriteIndex((int)BoneWeightIndices.size());
	for (const BoneWeightIndex& index : BoneWeightIndices)
	{
		stream->WriteUInt16(index.WeightIndex);
		stream->WriteUInt16(index.Number);
		stream->WriteUInt16(index.DetailA);
		stream->WriteUInt16(index.DetailB);
	}

	stream->WriteIndex((int)BoneWeights.size());
	for (const BoneWeight& weight : BoneWeights)
	{
		stream->WriteUInt16(weight.PointIndex);
		stream->WriteUInt16(weight.BoneWeight);
	}

	stream->WriteIndex((int)LocalPoints.size());
	for (const vec3& p : LocalPoints)
	{
		stream->WriteFloat(p.x);
		stream->WriteFloat(p.y);
		stream->WriteFloat(p.z);
	}

	stream->WriteUInt32(SkeletalDepth);
	stream->WriteObject(DefaultAnimation);
	stream->WriteUInt32(WeaponBoneIndex);

	stream->WriteFloat(WeaponAdjust.Origin.x);
	stream->WriteFloat(WeaponAdjust.Origin.y);
	stream->WriteFloat(WeaponAdjust.Origin.z);
	stream->WriteFloat(WeaponAdjust.XAxis.x);
	stream->WriteFloat(WeaponAdjust.XAxis.y);
	stream->WriteFloat(WeaponAdjust.XAxis.z);
	stream->WriteFloat(WeaponAdjust.YAxis.x);
	stream->WriteFloat(WeaponAdjust.YAxis.y);
	stream->WriteFloat(WeaponAdjust.YAxis.z);
	stream->WriteFloat(WeaponAdjust.ZAxis.x);
	stream->WriteFloat(WeaponAdjust.ZAxis.y);
	stream->WriteFloat(WeaponAdjust.ZAxis.z);
}
