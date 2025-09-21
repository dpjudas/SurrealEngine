
#include "Precomp.h"
#include "UMesh.h"
#include "UTexture.h"
#include "Engine.h"
#include "Package/PackageManager.h"

void UPrimitive::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	BoundingBox.min.x = stream->ReadFloat();
	BoundingBox.min.y = stream->ReadFloat();
	BoundingBox.min.z = stream->ReadFloat();
	BoundingBox.max.x = stream->ReadFloat();
	BoundingBox.max.y = stream->ReadFloat();
	BoundingBox.max.z = stream->ReadFloat();
	BoundingBox.IsValid = stream->ReadUInt8() != 0;

	BoundingSphere.x = stream->ReadFloat();
	BoundingSphere.y = stream->ReadFloat();
	BoundingSphere.z = stream->ReadFloat();
	BoundingSphere.w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;
}

void UPrimitive::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);

	stream->WriteFloat(BoundingBox.min.x);
	stream->WriteFloat(BoundingBox.min.y);
	stream->WriteFloat(BoundingBox.min.z);
	stream->WriteFloat(BoundingBox.max.x);
	stream->WriteFloat(BoundingBox.max.y);
	stream->WriteFloat(BoundingBox.max.z);
	stream->WriteUInt8(BoundingBox.IsValid ? 1 : 0);

	stream->WriteFloat(BoundingSphere.x);
	stream->WriteFloat(BoundingSphere.y);
	stream->WriteFloat(BoundingSphere.z);
	if (stream->GetVersion() > 61)
		stream->WriteFloat(BoundingSphere.w);
}

/////////////////////////////////////////////////////////////////////////////

void UMesh::Load(ObjectStream* stream)
{
	UPrimitive::Load(stream);

	uint32_t VertsSkipOffset = 0;
	if (stream->GetVersion() > 61) VertsSkipOffset = stream->ReadUInt32();
	int NumVerts = stream->ReadIndex();

	// XXX: Something to consider if we decide to load DeusEx packages
	//      without necessarily running DeusEx, we will need some kind
	//      of indicator as to what mesh type we're looking at.
	//      Maybe just some optional metadata in the package that describes 
	//			which type of mesh this is?
	if (stream->GetPackage()->GetPackageManager()->IsDeusEx())
	{
		for (int i = 0; i < NumVerts; i++)
		{
			struct DeusExVertex
			{
				int16_t x, y, z, padding;
			} packedvertex;
			stream->ReadBytes(&packedvertex, sizeof(DeusExVertex));
			vec3 vertex = { (float)packedvertex.x, (float)packedvertex.y, (float)packedvertex.z };
			Verts.push_back(vertex);
		}
	}
	else
	{
		for (int i = 0; i < NumVerts; i++)
		{
			int32_t packedvertex = stream->ReadInt32();
			vec3 vertex = { (float)((packedvertex << 21) >> 21), (float)((packedvertex << 10) >> 21), (float)(packedvertex >> 22) };
			Verts.push_back(vertex);
		}
	}
	if (stream->GetVersion() > 61 && stream->Tell() != VertsSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	uint32_t TrisSkipOffset = 0;
	if (stream->GetVersion() > 61) TrisSkipOffset = stream->ReadUInt32();
	int NumTris = stream->ReadIndex();
	for (int i = 0; i < NumTris; i++)
	{
		MeshTri tri;
		tri.Indices[0] = stream->ReadUInt16();
		tri.Indices[1] = stream->ReadUInt16();
		tri.Indices[2] = stream->ReadUInt16();
		tri.UV[0].x = stream->ReadUInt8();
		tri.UV[0].y = stream->ReadUInt8();
		tri.UV[1].x = stream->ReadUInt8();
		tri.UV[1].y = stream->ReadUInt8();
		tri.UV[2].x = stream->ReadUInt8();
		tri.UV[2].y = stream->ReadUInt8();
		tri.PolyFlags = stream->ReadUInt32();
		tri.TextureIndex = stream->ReadInt32();
		Tris.push_back(tri);
	}
	if (stream->GetVersion() > 61 && stream->Tell() != TrisSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	int NumAnimSeq = stream->ReadIndex();
	for (int i = 0; i < NumAnimSeq; i++)
	{
		MeshAnimSeq seq;
		seq.Name = stream->ReadName();
		seq.Group = stream->ReadName();
		seq.StartFrame = stream->ReadInt32();
		seq.NumFrames = stream->ReadInt32();
		int NumNotifys = stream->ReadIndex();
		for (int j = 0; j < NumNotifys; j++)
		{
			MeshAnimNotify notify;
			notify.Time = stream->ReadFloat();
			notify.Function = stream->ReadName();
			seq.Notifys.push_back(notify);
		}
		seq.Rate = stream->ReadFloat();
		std::stable_sort(seq.Notifys.begin(), seq.Notifys.end(), [](auto& a, auto& b) { return a.Time < b.Time; });
		AnimSeqs.push_back(seq);
	}

	uint32_t ConnectsSkipOffset = 0;
	if (stream->GetVersion() > 61) ConnectsSkipOffset = stream->ReadUInt32();
	int NumConnects = stream->ReadIndex();
	for (int i = 0; i < NumConnects; i++)
	{
		MeshVertConnect connect;
		connect.NumVertTriangles = stream->ReadInt32();
		connect.TriangleListOffset = stream->ReadUInt32();
		Connects.push_back(connect);
	}
	if (stream->GetVersion() > 61 && stream->Tell() != ConnectsSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	// Unknown bbox and sphere?
	BBox bbox;
	bbox.min.x = stream->ReadFloat();
	bbox.min.y = stream->ReadFloat();
	bbox.min.z = stream->ReadFloat();
	bbox.max.x = stream->ReadFloat();
	bbox.max.y = stream->ReadFloat();
	bbox.max.z = stream->ReadFloat();
	bbox.IsValid = stream->ReadUInt8() != 0;
	float x = stream->ReadFloat();
	float y = stream->ReadFloat();
	float z = stream->ReadFloat();
	float w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;

	uint32_t VertLinksSkipOffset = 0;
	if (stream->GetVersion() > 61) VertLinksSkipOffset = stream->ReadUInt32();
	int NumVertLinks = stream->ReadIndex();
	for (int i = 0; i < NumVertLinks; i++)
	{
		VertLinks.push_back(stream->ReadInt32());
	}
	if (stream->GetVersion() > 61 && stream->Tell() != VertLinksSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	int NumTextures = stream->ReadIndex();
	for (int i = 0; i < NumTextures; i++)
		Textures.push_back(stream->ReadObject<UTexture>());

	int NumBoundingBoxes = stream->ReadIndex();
	for (int i = 0; i < NumBoundingBoxes; i++)
	{
		BBox bbox;
		bbox.min.x = stream->ReadFloat();
		bbox.min.y = stream->ReadFloat();
		bbox.min.z = stream->ReadFloat();
		bbox.max.x = stream->ReadFloat();
		bbox.max.y = stream->ReadFloat();
		bbox.max.z = stream->ReadFloat();
		bbox.IsValid = stream->ReadUInt8() == 1;
		BoundingBoxes.push_back(bbox);
	}

	int NumBoundingSpheres = stream->ReadIndex();
	for (int i = 0; i < NumBoundingSpheres; i++)
	{
		float x = stream->ReadFloat();
		float y = stream->ReadFloat();
		float z = stream->ReadFloat();
		float w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;
		BoundingSpheres.push_back({ x, y, z, w });
	}

	FrameVerts = stream->ReadInt32();
	AnimFrames = stream->ReadInt32();

	AndFlags = stream->ReadUInt32();
	OrFlags = stream->ReadUInt32();

	Scale.x = stream->ReadFloat();
	Scale.y = stream->ReadFloat();
	Scale.z = stream->ReadFloat();

	Origin.x = stream->ReadFloat();
	Origin.y = stream->ReadFloat();
	Origin.z = stream->ReadFloat();

	RotOrigin.Pitch = stream->ReadInt32();
	RotOrigin.Yaw = stream->ReadInt32();
	RotOrigin.Roll = stream->ReadInt32();

	CurPoly = stream->ReadUInt32();
	CurVertex = stream->ReadUInt32();

	if (stream->GetVersion() == 65)
	{
		TextureLOD.push_back(stream->ReadFloat());
	}
	else if (stream->GetVersion() >= 66)
	{
		int NumTextureLOD = stream->ReadIndex();
		for (int i = 0; i < NumTextureLOD; i++)
		{
			TextureLOD.push_back(stream->ReadFloat());
		}
	}

	meshToObject = Coords::Rotation(RotOrigin).ToMatrix() * mat4::scale(Scale) * mat4::translate(-Origin);

	// Build smoothed normals
	// 
	// To do: build normals from mesh faces (maybe using the Connects array?)
}

void UMesh::Save(PackageStreamWriter* stream)
{
	UPrimitive::Save(stream);
	Exception::Throw("UMesh::Save not implemented");
}

/////////////////////////////////////////////////////////////////////////////

void ULodMesh::Load(ObjectStream* stream)
{
	UMesh::Load(stream);

	int NumCollapsePointThus = stream->ReadIndex();
	for (int i = 0; i < NumCollapsePointThus; i++)
		CollapsePointThus.push_back(stream->ReadUInt16());

	int NumFaceLevel = stream->ReadIndex();
	for (int i = 0; i < NumFaceLevel; i++)
		FaceLevel.push_back(stream->ReadUInt16());

	int NumFaces = stream->ReadIndex();
	for (int i = 0; i < NumFaces; i++)
	{
		MeshFace face;
		face.Indices[0] = stream->ReadUInt16();
		face.Indices[1] = stream->ReadUInt16();
		face.Indices[2] = stream->ReadUInt16();
		face.MaterialIndex = stream->ReadUInt16();
		Faces.push_back(face);
	}

	int NumCollapseWedgeThus = stream->ReadIndex();
	for (int i = 0; i < NumCollapseWedgeThus; i++)
		CollapseWedgeThus.push_back(stream->ReadUInt16());

	int NumWedges = stream->ReadIndex();
	for (int i = 0; i < NumWedges; i++)
	{
		MeshWedge wedge;
		wedge.Vertex = stream->ReadUInt16();
		wedge.U = stream->ReadUInt8();
		wedge.V = stream->ReadUInt8();
		Wedges.push_back(wedge);
	}

	int NumMaterials = stream->ReadIndex();
	for (int i = 0; i < NumMaterials; i++)
	{
		MeshMaterial material;
		material.PolyFlags = stream->ReadUInt32();
		material.TextureIndex = stream->ReadInt32();
		Materials.push_back(material);
	}

	int NumSpecialFaces = stream->ReadIndex();
	for (int i = 0; i < NumSpecialFaces; i++)
	{
		MeshFace face;
		face.Indices[0] = stream->ReadUInt16();
		face.Indices[1] = stream->ReadUInt16();
		face.Indices[2] = stream->ReadUInt16();
		face.MaterialIndex = stream->ReadUInt16();
		SpecialFaces.push_back(face);
	}

	ModelVerts = stream->ReadUInt32();
	SpecialVerts = stream->ReadUInt32();
	MeshScaleMax = stream->ReadFloat();
	LODHysteresis = stream->ReadFloat();
	LODStrength = stream->ReadFloat();
	LODMinVerts = stream->ReadUInt32();
	LODMorph = stream->ReadFloat();
	LODZDisplace = stream->ReadFloat();

	int NumReMapAnimVerts = stream->ReadIndex();
	for (int i = 0; i < NumReMapAnimVerts; i++)
		ReMapAnimVerts.push_back(stream->ReadUInt16());

	OldFrameVerts = stream->ReadUInt32();

	// Build smoothed normals
	Normals.clear();
	Normals.resize(Verts.size(), vec3(0.0f));
	for (int frame = 0; frame < AnimFrames; frame++)
	{
		for (const MeshFace& face : SpecialFaces)
		{
			int v0 = Wedges[face.Indices[0]].Vertex + frame * FrameVerts;
			int v1 = Wedges[face.Indices[1]].Vertex + frame * FrameVerts;
			int v2 = Wedges[face.Indices[2]].Vertex + frame * FrameVerts;
			vec3 n = normalize(cross(Verts[v1] - Verts[v0], Verts[v2] - Verts[v0]));
			Normals[v0] += n;
			Normals[v1] += n;
			Normals[v2] += n;
		}
		for (const MeshFace& face : Faces)
		{
			int v0 = Wedges[face.Indices[0]].Vertex + SpecialVerts + frame * FrameVerts;
			int v1 = Wedges[face.Indices[1]].Vertex + SpecialVerts + frame * FrameVerts;
			int v2 = Wedges[face.Indices[2]].Vertex + SpecialVerts + frame * FrameVerts;
			vec3 n = normalize(cross(Verts[v1] - Verts[v0], Verts[v2] - Verts[v0]));
			Normals[v0] += n;
			Normals[v1] += n;
			Normals[v2] += n;
		}
	}
	for (vec3& n : Normals)
	{
		n = normalize(n);
	}
}

void ULodMesh::Save(PackageStreamWriter* stream)
{
	UMesh::Save(stream);

	stream->WriteIndex((int)CollapsePointThus.size());
	for (uint16_t v : CollapsePointThus)
		stream->WriteUInt16(v);

	stream->WriteIndex((int)FaceLevel.size());
	for (uint16_t v : FaceLevel)
		stream->WriteUInt16(v);

	stream->WriteIndex((int)Faces.size());
	for (const MeshFace& face : Faces)
	{
		stream->WriteUInt16(face.Indices[0]);
		stream->WriteUInt16(face.Indices[1]);
		stream->WriteUInt16(face.Indices[2]);
		stream->WriteUInt16(face.MaterialIndex);
	}

	stream->WriteIndex((int)CollapseWedgeThus.size());
	for (uint16_t v : CollapseWedgeThus)
		stream->WriteUInt16(v);

	stream->WriteIndex((int)Wedges.size());
	for (const MeshWedge& wedge : Wedges)
	{
		stream->WriteUInt16(wedge.Vertex);
		stream->WriteUInt8(wedge.U);
		stream->WriteUInt8(wedge.V);
	}

	stream->WriteIndex((int)Materials.size());
	for (const MeshMaterial& material : Materials)
	{
		stream->WriteUInt32(material.PolyFlags);
		stream->WriteInt32(material.TextureIndex);
	}

	stream->WriteIndex((int)SpecialFaces.size());
	for (const MeshFace& face : SpecialFaces)
	{
		stream->WriteUInt16(face.Indices[0]);
		stream->WriteUInt16(face.Indices[1]);
		stream->WriteUInt16(face.Indices[2]);
		stream->WriteUInt16(face.MaterialIndex);
	}

	stream->WriteUInt32(ModelVerts);
	stream->WriteUInt32(SpecialVerts);
	stream->WriteFloat(MeshScaleMax);
	stream->WriteFloat(LODHysteresis);
	stream->WriteFloat(LODStrength);
	stream->WriteUInt32(LODMinVerts);
	stream->WriteFloat(LODMorph);
	stream->WriteFloat(LODZDisplace);

	stream->WriteIndex((int)ReMapAnimVerts.size());
	for (uint16_t v : ReMapAnimVerts)
		stream->WriteUInt16(v);

	stream->WriteUInt32(OldFrameVerts);
}

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

void UAnimation::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	int NumRefBones = stream->ReadIndex();
	for (int i = 0; i < NumRefBones; i++)
	{
		RefBone refbone;
		refbone.Name = stream->ReadName();
		refbone.Flags = stream->ReadUInt32();
		refbone.ParentIndex = stream->ReadUInt32();
		RefBones.push_back(refbone);
	}

	int NumMoves = stream->ReadIndex();
	for (int i = 0; i < NumMoves; i++)
	{
		AnimMove move;
		move.RootSpeed3D.x = stream->ReadFloat();
		move.RootSpeed3D.y = stream->ReadFloat();
		move.RootSpeed3D.z = stream->ReadFloat();
		move.TrackTime = stream->ReadFloat();
		move.StartBone = stream->ReadUInt32();
		move.Flags = stream->ReadUInt32();

		int NumBoneIndices = stream->ReadIndex();
		for (int j = 0; j < NumBoneIndices; j++)
			move.BoneIndices.push_back(stream->ReadUInt32());

		int NumAnimTracks = stream->ReadIndex();
		for (int j = 0; j < NumAnimTracks; j++)
		{
			AnimTrack track;
			track.Flags = stream->ReadUInt32();

			int NumKeyQuat = stream->ReadIndex();
			for (int k = 0; k < NumKeyQuat; k++)
			{
				quaternion q;
				q.x = stream->ReadFloat();
				q.y = stream->ReadFloat();
				q.z = stream->ReadFloat();
				q.w = stream->ReadFloat();
				track.KeyQuat.push_back(q);
			}

			int NumKeyPos = stream->ReadIndex();
			for (int k = 0; k < NumKeyPos; k++)
			{
				vec3 pos;
				pos.x = stream->ReadFloat();
				pos.y = stream->ReadFloat();
				pos.z = stream->ReadFloat();
				track.KeyPos.push_back(pos);
			}

			int NumKeyTime = stream->ReadIndex();
			for (int k = 0; k < NumKeyTime; k++)
				track.KeyTime.push_back(stream->ReadFloat());

			move.AnimTracks.push_back(track);
		}

		move.RootTrack.Flags = stream->ReadUInt32();

		int NumKeyQuat = stream->ReadIndex();
		for (int k = 0; k < NumKeyQuat; k++)
		{
			quaternion q;
			q.x = stream->ReadFloat();
			q.y = stream->ReadFloat();
			q.z = stream->ReadFloat();
			q.w = stream->ReadFloat();
			move.RootTrack.KeyQuat.push_back(q);
		}

		int NumKeyPos = stream->ReadIndex();
		for (int k = 0; k < NumKeyPos; k++)
		{
			vec3 pos;
			pos.x = stream->ReadFloat();
			pos.y = stream->ReadFloat();
			pos.z = stream->ReadFloat();
			move.RootTrack.KeyPos.push_back(pos);
		}

		int NumKeyTime = stream->ReadIndex();
		for (int k = 0; k < NumKeyTime; k++)
			move.RootTrack.KeyTime.push_back(stream->ReadFloat());

		Moves.push_back(move);
	}
}

void UAnimation::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);

	stream->WriteIndex((int)RefBones.size());
	for (const RefBone& refbone : RefBones)
	{
		stream->WriteName(refbone.Name);
		stream->WriteUInt32(refbone.Flags);
		stream->WriteUInt32(refbone.ParentIndex);
	}

	stream->WriteIndex((int)Moves.size());
	for (const AnimMove& move : Moves)
	{
		stream->WriteFloat(move.RootSpeed3D.x);
		stream->WriteFloat(move.RootSpeed3D.y);
		stream->WriteFloat(move.RootSpeed3D.z);
		stream->WriteFloat(move.TrackTime);
		stream->WriteUInt32(move.StartBone);
		stream->WriteUInt32(move.Flags);

		stream->WriteIndex((int)move.BoneIndices.size());
		for (uint32_t index : move.BoneIndices)
			stream->WriteUInt32(index);

		stream->WriteIndex((int)move.AnimTracks.size());
		for (const AnimTrack& track : move.AnimTracks)
		{
			stream->WriteUInt32(track.Flags);

			stream->WriteIndex((int)track.KeyQuat.size());
			for (const quaternion& q : track.KeyQuat)
			{
				stream->WriteFloat(q.x);
				stream->WriteFloat(q.y);
				stream->WriteFloat(q.z);
				stream->WriteFloat(q.w);
			}

			stream->WriteIndex((int)track.KeyPos.size());
			for (const vec3& pos : track.KeyPos)
			{
				stream->WriteFloat(pos.x);
				stream->WriteFloat(pos.y);
				stream->WriteFloat(pos.z);
			}

			stream->WriteIndex((int)track.KeyTime.size());
			for (float v : track.KeyTime)
				stream->WriteFloat(v);
		}

		stream->WriteUInt32(move.RootTrack.Flags);

		stream->WriteIndex((int)move.RootTrack.KeyQuat.size());
		for (const quaternion& q : move.RootTrack.KeyQuat)
		{
			stream->WriteFloat(q.x);
			stream->WriteFloat(q.y);
			stream->WriteFloat(q.z);
			stream->WriteFloat(q.w);
		}

		stream->WriteIndex((int)move.RootTrack.KeyPos.size());
		for (const vec3& pos : move.RootTrack.KeyPos)
		{
			stream->WriteFloat(pos.x);
			stream->WriteFloat(pos.y);
			stream->WriteFloat(pos.z);
		}

		stream->WriteIndex((int)move.RootTrack.KeyTime.size());
		for (float v : move.RootTrack.KeyTime)
			stream->WriteFloat(v);

		Moves.push_back(move);
	}
}
