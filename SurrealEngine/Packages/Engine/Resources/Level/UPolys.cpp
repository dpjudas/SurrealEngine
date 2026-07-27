
#include "Precomp.h"
#include "UPolys.h"
#include "Packages/Engine/Actors/Brush/UBrush.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

void UPolys::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		int numVertices = stream->ReadIndex();
		Poly poly;
		poly.Base.x = stream->ReadFloat();
		poly.Base.y = stream->ReadFloat();
		poly.Base.z = stream->ReadFloat();
		poly.Normal.x = stream->ReadFloat();
		poly.Normal.y = stream->ReadFloat();
		poly.Normal.z = stream->ReadFloat();
		poly.TextureU.x = stream->ReadFloat();
		poly.TextureU.y = stream->ReadFloat();
		poly.TextureU.z = stream->ReadFloat();
		poly.TextureV.x = stream->ReadFloat();
		poly.TextureV.y = stream->ReadFloat();
		poly.TextureV.z = stream->ReadFloat();
		for (int i = 0; i < numVertices; i++)
		{
			vec3 v;
			v.x = stream->ReadFloat();
			v.y = stream->ReadFloat();
			v.z = stream->ReadFloat();
			poly.Vertices.push_back(v);
		}
		poly.PolyFlags = stream->ReadUInt32();
		poly.Actor = stream->ReadObject<UBrush>();
		poly.Texture = stream->ReadObject<UTexture>();
		poly.ItemName = stream->ReadName();
		poly.LinkIndex = stream->ReadIndex();
		poly.BrushPolyIndex = stream->ReadIndex();
		poly.PanU = stream->ReadInt16();
		poly.PanV = stream->ReadInt16();
		Polys.push_back(poly);
	}
}

void UPolys::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	int count = (int)Polys.size();
	int maxcount = count;
	stream->WriteInt32(count);
	stream->WriteInt32(maxcount);
	for (const Poly& poly : Polys)
	{
		stream->WriteIndex((int)poly.Vertices.size());
		stream->WriteFloat(poly.Base.x);
		stream->WriteFloat(poly.Base.y);
		stream->WriteFloat(poly.Base.z);
		stream->WriteFloat(poly.Normal.x);
		stream->WriteFloat(poly.Normal.y);
		stream->WriteFloat(poly.Normal.z);
		stream->WriteFloat(poly.TextureU.x);
		stream->WriteFloat(poly.TextureU.y);
		stream->WriteFloat(poly.TextureU.z);
		stream->WriteFloat(poly.TextureV.x);
		stream->WriteFloat(poly.TextureV.y);
		stream->WriteFloat(poly.TextureV.z);
		for (const vec3& v : poly.Vertices)
		{
			stream->WriteFloat(v.x);
			stream->WriteFloat(v.y);
			stream->WriteFloat(v.z);
		}
		stream->WriteUInt32(poly.PolyFlags);
		stream->WriteObject(poly.Actor);
		stream->WriteObject(poly.Texture);
		stream->WriteName(poly.ItemName);
		stream->WriteIndex(poly.LinkIndex);
		stream->WriteIndex(poly.BrushPolyIndex);
		stream->WriteInt16(poly.PanU);
		stream->WriteInt16(poly.PanV);
	}
}
