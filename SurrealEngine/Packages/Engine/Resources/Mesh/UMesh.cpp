
#include "Precomp.h"
#include "UMesh.h"
#include "Package/PackageManager.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

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
	MeshBoundingBox.min.x = stream->ReadFloat();
	MeshBoundingBox.min.y = stream->ReadFloat();
	MeshBoundingBox.min.z = stream->ReadFloat();
	MeshBoundingBox.max.x = stream->ReadFloat();
	MeshBoundingBox.max.y = stream->ReadFloat();
	MeshBoundingBox.max.z = stream->ReadFloat();
	MeshBoundingBox.IsValid = stream->ReadUInt8() != 0;
	MeshBoundingSphere.x = stream->ReadFloat();
	MeshBoundingSphere.y = stream->ReadFloat();
	MeshBoundingSphere.z = stream->ReadFloat();
	MeshBoundingSphere.w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;

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
	Normals.clear();
	Normals.resize(Verts.size(), vec3(0.0f));
	for (int frame = 0; frame < AnimFrames; frame++)
	{
		for (const MeshTri& tri : Tris)
		{
			int v0 = tri.Indices[0] + frame * FrameVerts;
			int v1 = tri.Indices[1] + frame * FrameVerts;
			int v2 = tri.Indices[2] + frame * FrameVerts;
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

void UMesh::Save(PackageStreamWriter* stream)
{
	UPrimitive::Save(stream);

	if (stream->GetVersion() > 61)
		stream->BeginSkipOffset();

	stream->WriteIndex((int)Verts.size());

	if (false/*stream->GetPackage()->GetPackageManager()->IsDeusEx()*/)
	{
		for (const vec3& v : Verts)
		{
			struct DeusExVertex
			{
				int16_t x, y, z, padding;
			} packedvertex;
			packedvertex.x = (int)std::round(v.x);
			packedvertex.y = (int)std::round(v.y);
			packedvertex.z = (int)std::round(v.z);
			stream->WriteBytes(&packedvertex, sizeof(DeusExVertex));
		}
	}
	else
	{
		for (const vec3& v : Verts)
		{
			int32_t x = (int)std::round(v.x);
			int32_t y = (int)std::round(v.y);
			int32_t z = (int)std::round(v.z);
			int32_t packedvertex = (x & 0x7ff) | ((y & 0x7ff) << 11) | ((z & 0x3ff) << 22);
			stream->WriteInt32(packedvertex);
		}
	}

	if (stream->GetVersion() > 61)
	{
		stream->EndSkipOffset();
		stream->BeginSkipOffset();
	}

	stream->WriteIndex((int)Tris.size());
	for (const MeshTri& tri : Tris)
	{
		stream->WriteUInt16(tri.Indices[0]);
		stream->WriteUInt16(tri.Indices[1]);
		stream->WriteUInt16(tri.Indices[2]);
		stream->WriteUInt8(tri.UV[0].x);
		stream->WriteUInt8(tri.UV[0].y);
		stream->WriteUInt8(tri.UV[1].x);
		stream->WriteUInt8(tri.UV[1].y);
		stream->WriteUInt8(tri.UV[2].x);
		stream->WriteUInt8(tri.UV[2].y);
		stream->WriteUInt32(tri.PolyFlags);
		stream->WriteUInt32(tri.TextureIndex);
	}

	if (stream->GetVersion() > 61)
		stream->EndSkipOffset();

	stream->WriteIndex((int)AnimSeqs.size());
	for (const MeshAnimSeq& seq : AnimSeqs)
	{
		stream->WriteName(seq.Name);
		stream->WriteName(seq.Group);
		stream->WriteInt32(seq.StartFrame);
		stream->WriteInt32(seq.NumFrames);
		stream->WriteIndex((int)seq.Notifys.size());
		for (const MeshAnimNotify& notify : seq.Notifys)
		{
			stream->WriteFloat(notify.Time);
			stream->WriteName(notify.Function);
		}
		stream->WriteFloat(seq.Rate);
	}

	if (stream->GetVersion() > 61)
		stream->BeginSkipOffset();

	stream->WriteIndex((int)Connects.size());
	for (const MeshVertConnect& connect : Connects)
	{
		stream->WriteInt32(connect.NumVertTriangles);
		stream->WriteUInt32(connect.TriangleListOffset);
	}

	if (stream->GetVersion() > 61)
		stream->EndSkipOffset();

	stream->WriteFloat(MeshBoundingBox.min.x);
	stream->WriteFloat(MeshBoundingBox.min.y);
	stream->WriteFloat(MeshBoundingBox.min.z);
	stream->WriteFloat(MeshBoundingBox.max.x);
	stream->WriteFloat(MeshBoundingBox.max.y);
	stream->WriteFloat(MeshBoundingBox.max.z);
	stream->WriteInt8(MeshBoundingBox.IsValid ? 1 : 0);
	stream->WriteFloat(MeshBoundingSphere.x);
	stream->WriteFloat(MeshBoundingSphere.y);
	stream->WriteFloat(MeshBoundingSphere.z);
	if (stream->GetVersion() > 61)
		stream->WriteFloat(MeshBoundingSphere.w);

	if (stream->GetVersion() > 61)
		stream->BeginSkipOffset();

	stream->WriteIndex((int)VertLinks.size());
	for (int v : VertLinks)
		stream->WriteInt32(v);

	if (stream->GetVersion() > 61)
		stream->EndSkipOffset();

	stream->WriteIndex((int)Textures.size());
	for (UTexture* tex : Textures)
		stream->WriteObject(tex);

	stream->WriteIndex((int)BoundingBoxes.size());
	for (const BBox& bbox : BoundingBoxes)
	{
		stream->WriteFloat(bbox.min.x);
		stream->WriteFloat(bbox.min.y);
		stream->WriteFloat(bbox.min.z);
		stream->WriteFloat(bbox.max.x);
		stream->WriteFloat(bbox.max.y);
		stream->WriteFloat(bbox.max.z);
		stream->WriteInt8(bbox.IsValid ? 1 : 0);
	}

	stream->WriteIndex((int)BoundingSpheres.size());
	for (const vec4& v : BoundingSpheres)
	{
		stream->WriteFloat(v.x);
		stream->WriteFloat(v.y);
		stream->WriteFloat(v.z);
		if (stream->GetVersion() > 61)
			stream->WriteFloat(v.w);
	}

	stream->WriteInt32(FrameVerts);
	stream->WriteInt32(AnimFrames);

	stream->WriteUInt32(AndFlags);
	stream->WriteUInt32(OrFlags);

	stream->WriteFloat(Scale.x);
	stream->WriteFloat(Scale.y);
	stream->WriteFloat(Scale.z);

	stream->WriteFloat(Origin.x);
	stream->WriteFloat(Origin.y);
	stream->WriteFloat(Origin.z);

	stream->WriteInt32(RotOrigin.Pitch);
	stream->WriteInt32(RotOrigin.Yaw);
	stream->WriteInt32(RotOrigin.Roll);

	stream->WriteUInt32(CurPoly);
	stream->WriteUInt32(CurVertex);

	if (stream->GetVersion() == 65)
	{
		stream->WriteFloat(TextureLOD.front());
	}
	else if (stream->GetVersion() >= 66)
	{
		stream->WriteIndex((int)TextureLOD.size());
		for (float v : TextureLOD)
			stream->WriteFloat(v);
	}
}
