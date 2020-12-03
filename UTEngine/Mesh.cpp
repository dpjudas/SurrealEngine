
#include "Precomp.h"
#include "Mesh.h"
#include "PackageManager.h"
#include "Package.h"
#include "TextureManager.h"
#include "BinaryStream.h"

Mesh::Mesh(std::string meshname, PackageManager* packages, TextureManager* textures)
{
	Package* package = packages->GetPackage("UnrealShare");
	auto entry = package->FindExportObject("LodMesh", meshname);
	auto obj = entry->Open();

	// UPrimitive:

	BoundingBox.min.x = obj->Stream->ReadFloat();
	BoundingBox.min.y = obj->Stream->ReadFloat();
	BoundingBox.min.z = obj->Stream->ReadFloat();
	BoundingBox.max.x = obj->Stream->ReadFloat();
	BoundingBox.max.y = obj->Stream->ReadFloat();
	BoundingBox.max.z = obj->Stream->ReadFloat();
	BoundingBox.IsValid = obj->Stream->ReadUInt8() != 0;

	BoundingSphere.x = obj->Stream->ReadFloat();
	BoundingSphere.y = obj->Stream->ReadFloat();
	BoundingSphere.z = obj->Stream->ReadFloat();
	BoundingSphere.w = (package->GetVersion() > 61) ? obj->Stream->ReadFloat() : 0.0f;

	// UMesh:

	uint32_t VertsSkipOffset = 0;
	if (package->GetVersion() > 61) VertsSkipOffset = obj->Stream->ReadUInt32();
	int NumVerts = obj->Stream->ReadIndex();
	for (int i = 0; i < NumVerts; i++)
	{
		int32_t packedvertex = obj->Stream->ReadInt32();
		vec3 vertex = { (float)((packedvertex << 21) >> 21), (float)((packedvertex << 10) >> 21), (float)(packedvertex >> 22) };
		Verts.push_back(vertex);
	}
	if (package->GetVersion() > 61 && obj->Stream->Tell() != VertsSkipOffset)
		throw std::runtime_error("Unexpected lazy array size");

	uint32_t TrisSkipOffset = 0;
	if (package->GetVersion() > 61) TrisSkipOffset = obj->Stream->ReadUInt32();
	int NumTris = obj->Stream->ReadIndex();
	for (int i = 0; i < NumTris; i++)
	{
		MeshTri tri;
		tri.Indices[0] = obj->Stream->ReadUInt16();
		tri.Indices[1] = obj->Stream->ReadUInt16();
		tri.Indices[2] = obj->Stream->ReadUInt16();
		tri.UV[0].x = obj->Stream->ReadUInt8();
		tri.UV[0].y = obj->Stream->ReadUInt8();
		tri.UV[1].x = obj->Stream->ReadUInt8();
		tri.UV[1].y = obj->Stream->ReadUInt8();
		tri.UV[2].x = obj->Stream->ReadUInt8();
		tri.UV[2].y = obj->Stream->ReadUInt8();
		tri.PolyFlags = obj->Stream->ReadUInt32();
		tri.TextureIndex = obj->Stream->ReadInt32();
		Tris.push_back(tri);
	}
	if (package->GetVersion() > 61 && obj->Stream->Tell() != TrisSkipOffset)
		throw std::runtime_error("Unexpected lazy array size");

	int NumAnimSeq = obj->Stream->ReadIndex();
	for (int i = 0; i < NumAnimSeq; i++)
	{
		MeshAnimSeq seq;
		seq.Name = package->GetName(obj->Stream->ReadIndex());
		seq.Group = package->GetName(obj->Stream->ReadIndex());
		seq.StartFrame = obj->Stream->ReadInt32();
		seq.NumFrames = obj->Stream->ReadInt32();
		int NumNotifys = obj->Stream->ReadIndex();
		for (int j = 0; j < NumNotifys; j++)
		{
			MeshAnimNotify notify;
			notify.Time = obj->Stream->ReadFloat();
			notify.Function = package->GetName(obj->Stream->ReadIndex());
			seq.Notifys.push_back(notify);
		}
		seq.Rate = obj->Stream->ReadFloat();
		AnimSeqs.push_back(seq);
	}

	uint32_t ConnectsSkipOffset = 0;
	if (package->GetVersion() > 61) ConnectsSkipOffset = obj->Stream->ReadUInt32();
	int NumConnects = obj->Stream->ReadIndex();
	for (int i = 0; i < NumConnects; i++)
	{
		MeshVertConnect connect;
		connect.NumVertTriangles = obj->Stream->ReadInt32();
		connect.TriangleListOffset = obj->Stream->ReadUInt32();
		Connects.push_back(connect);
	}
	if (package->GetVersion() > 61 && obj->Stream->Tell() != ConnectsSkipOffset)
		throw std::runtime_error("Unexpected lazy array size");

	// Unknown bbox and sphere?
	BBox bbox;
	bbox.min.x = obj->Stream->ReadFloat();
	bbox.min.y = obj->Stream->ReadFloat();
	bbox.min.z = obj->Stream->ReadFloat();
	bbox.max.x = obj->Stream->ReadFloat();
	bbox.max.y = obj->Stream->ReadFloat();
	bbox.max.z = obj->Stream->ReadFloat();
	bbox.IsValid = obj->Stream->ReadUInt8() != 0;
	float x = obj->Stream->ReadFloat();
	float y = obj->Stream->ReadFloat();
	float z = obj->Stream->ReadFloat();
	float w = (package->GetVersion() > 61) ? obj->Stream->ReadFloat() : 0.0f;

	uint32_t VertLinksSkipOffset = 0;
	if (package->GetVersion() > 61) VertLinksSkipOffset = obj->Stream->ReadUInt32();
	int NumVertLinks = obj->Stream->ReadIndex();
	for (int i = 0; i < NumVertLinks; i++)
	{
		VertLinks.push_back(obj->Stream->ReadInt32());
	}
	if (package->GetVersion() > 61 && obj->Stream->Tell() != VertLinksSkipOffset)
		throw std::runtime_error("Unexpected lazy array size");

	int NumTextures = obj->Stream->ReadIndex();
	for (int i = 0; i < NumTextures; i++)
	{
		Textures.push_back(textures->GetTexture(package->FindExportObject(obj->Stream->ReadIndex())));
	}

	int NumBoundingBoxes = obj->Stream->ReadIndex();
	for (int i = 0; i < NumBoundingBoxes; i++)
	{
		BBox bbox;
		bbox.min.x = obj->Stream->ReadFloat();
		bbox.min.y = obj->Stream->ReadFloat();
		bbox.min.z = obj->Stream->ReadFloat();
		bbox.max.x = obj->Stream->ReadFloat();
		bbox.max.y = obj->Stream->ReadFloat();
		bbox.max.z = obj->Stream->ReadFloat();
		bbox.IsValid = obj->Stream->ReadUInt8() == 1;
		BoundingBoxes.push_back(bbox);
	}

	int NumBoundingSpheres = obj->Stream->ReadIndex();
	for (int i = 0; i < NumBoundingSpheres; i++)
	{
		float x = obj->Stream->ReadFloat();
		float y = obj->Stream->ReadFloat();
		float z = obj->Stream->ReadFloat();
		float w = (package->GetVersion() > 61) ? obj->Stream->ReadFloat() : 0.0f;
		BoundingSpheres.push_back({ x, y, z, w });
	}

	FrameVerts = obj->Stream->ReadInt32();
	AnimFrames = obj->Stream->ReadInt32();

	AndFlags = obj->Stream->ReadUInt32();
	OrFlags = obj->Stream->ReadUInt32();

	Scale.x = obj->Stream->ReadFloat();
	Scale.y = obj->Stream->ReadFloat();
	Scale.z = obj->Stream->ReadFloat();

	Origin.x = obj->Stream->ReadFloat();
	Origin.y = obj->Stream->ReadFloat();
	Origin.z = obj->Stream->ReadFloat();

	RotOrigin.Pitch = obj->Stream->ReadInt32();
	RotOrigin.Yaw = obj->Stream->ReadInt32();
	RotOrigin.Roll = obj->Stream->ReadInt32();

	CurPoly = obj->Stream->ReadUInt32();
	CurVertex = obj->Stream->ReadUInt32();

	if (package->GetVersion() == 65)
	{
		TextureLOD.push_back(obj->Stream->ReadFloat());
	}
	else if (package->GetVersion() >= 66)
	{
		int NumTextureLOD = obj->Stream->ReadIndex();
		for (int i = 0; i < NumTextureLOD; i++)
		{
			TextureLOD.push_back(obj->Stream->ReadFloat());
		}
	}

	// ULodMesh

	int NumCollapsePointThus = obj->Stream->ReadIndex();
	for (int i = 0; i < NumCollapsePointThus; i++)
		CollapsePointThus.push_back(obj->Stream->ReadUInt16());

	int NumFaceLevel = obj->Stream->ReadIndex();
	for (int i = 0; i < NumFaceLevel; i++)
		FaceLevel.push_back(obj->Stream->ReadUInt16());

	int NumFaces = obj->Stream->ReadIndex();
	for (int i = 0; i < NumFaces; i++)
	{
		MeshFace face;
		face.Indices[0] = obj->Stream->ReadUInt16();
		face.Indices[1] = obj->Stream->ReadUInt16();
		face.Indices[2] = obj->Stream->ReadUInt16();
		face.MaterialIndex = obj->Stream->ReadUInt16();
		Faces.push_back(face);
	}

	int NumCollapseWedgeThus = obj->Stream->ReadIndex();
	for (int i = 0; i < NumCollapseWedgeThus; i++)
		CollapseWedgeThus.push_back(obj->Stream->ReadUInt16());

	int NumWedges = obj->Stream->ReadIndex();
	for (int i = 0; i < NumWedges; i++)
	{
		MeshWedge wedge;
		wedge.Vertex = obj->Stream->ReadUInt16();
		wedge.U = obj->Stream->ReadUInt8();
		wedge.V = obj->Stream->ReadUInt8();
		Wedges.push_back(wedge);
	}

	int NumMaterials = obj->Stream->ReadIndex();
	for (int i = 0; i < NumMaterials; i++)
	{
		MeshMaterial material;
		material.PolyFlags = obj->Stream->ReadUInt32();
		material.TextureIndex = obj->Stream->ReadInt32();
		Materials.push_back(material);
	}

	int NumSpecialFaces = obj->Stream->ReadIndex();
	for (int i = 0; i < NumSpecialFaces; i++)
	{
		MeshFace face;
		face.Indices[0] = obj->Stream->ReadUInt16();
		face.Indices[1] = obj->Stream->ReadUInt16();
		face.Indices[2] = obj->Stream->ReadUInt16();
		face.MaterialIndex = obj->Stream->ReadUInt16();
		SpecialFaces.push_back(face);
	}

	ModelVerts = obj->Stream->ReadUInt32();
	SpecialVerts = obj->Stream->ReadUInt32();
	MeshScaleMax = obj->Stream->ReadFloat();
	LODHysteresis = obj->Stream->ReadFloat();
	LODStrength = obj->Stream->ReadFloat();
	LODMinVerts = obj->Stream->ReadUInt32();
	LODMorph = obj->Stream->ReadFloat();
	LODZDisplace = obj->Stream->ReadFloat();

	int NumReMapAnimVerts = obj->Stream->ReadIndex();
	for (int i = 0; i < NumReMapAnimVerts; i++)
	{
		ReMapAnimVerts.push_back(obj->Stream->ReadUInt16());
	}

	OldFrameVerts = obj->Stream->ReadUInt32();
}
