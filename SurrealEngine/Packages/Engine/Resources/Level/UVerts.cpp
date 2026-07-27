
#include "Precomp.h"
#include "UVerts.h"

void UVerts::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		BspVert vert;
		vert.Vertex = stream->ReadIndex();
		vert.Side = stream->ReadIndex();
		Vertices.push_back(vert);
	}

	NumSharedSides = stream->ReadIndex();
}

void UVerts::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	int count = (int)Vertices.size();
	int maxcount = count;
	stream->WriteInt32(count);
	stream->WriteInt32(maxcount);
	for (const BspVert& vert : Vertices)
	{
		stream->WriteIndex(vert.Vertex);
		stream->WriteIndex(vert.Side);
	}
	stream->WriteIndex(NumSharedSides);
}
