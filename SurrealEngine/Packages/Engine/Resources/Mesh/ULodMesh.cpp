
#include "Precomp.h"
#include "ULodMesh.h"

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
