
#include "Precomp.h"
#include "UMesh.h"
#include "UTexture.h"

UPrimitive::UPrimitive(ObjectStream* stream) : UObject(stream)
{
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

/////////////////////////////////////////////////////////////////////////////

UMesh::UMesh(ObjectStream* stream) : UPrimitive(stream)
{
	uint32_t VertsSkipOffset = 0;
	if (stream->GetVersion() > 61) VertsSkipOffset = stream->ReadUInt32();
	int NumVerts = stream->ReadIndex();
	for (int i = 0; i < NumVerts; i++)
	{
		int32_t packedvertex = stream->ReadInt32();
		vec3 vertex = { (float)((packedvertex << 21) >> 21), (float)((packedvertex << 10) >> 21), (float)(packedvertex >> 22) };
		Verts.push_back(vertex);
	}
	if (stream->GetVersion() > 61 && stream->Tell() != VertsSkipOffset)
		throw std::runtime_error("Unexpected lazy array size");

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
		throw std::runtime_error("Unexpected lazy array size");

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
		throw std::runtime_error("Unexpected lazy array size");

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
		throw std::runtime_error("Unexpected lazy array size");

	int NumTextures = stream->ReadIndex();
	for (int i = 0; i < NumTextures; i++)
	{
		Textures.push_back(Cast<UTexture>(stream->ReadUObject()));
	}

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
}

/////////////////////////////////////////////////////////////////////////////

ULodMesh::ULodMesh(ObjectStream* stream) : UMesh(stream)
{
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
	{
		ReMapAnimVerts.push_back(stream->ReadUInt16());
	}

	OldFrameVerts = stream->ReadUInt32();
}
