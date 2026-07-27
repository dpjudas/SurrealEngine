
#include "Precomp.h"
#include "UBspSurfs.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

void UBspSurfs::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		BspSurface surface;
		surface.Material = stream->ReadObject<UTexture>();
		surface.PolyFlags = stream->ReadUInt32();
		surface.pBase = stream->ReadIndex();
		surface.vNormal = stream->ReadIndex();
		surface.vTextureU = stream->ReadIndex();
		surface.vTextureV = stream->ReadIndex();
		surface.LightMap = stream->ReadIndex();
		surface.BrushPoly = stream->ReadIndex();
		surface.PanU = stream->ReadInt16();
		surface.PanV = stream->ReadInt16();
		surface.BrushActor = stream->ReadObject<UActor>();
		Surfaces.push_back(surface);
	}
}

void UBspSurfs::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	int count = (int)Surfaces.size();
	int maxcount = count;
	stream->WriteInt32(count);
	stream->WriteInt32(maxcount);
	for (const BspSurface& surface : Surfaces)
	{
		stream->WriteObject(surface.Material);
		stream->WriteUInt32(surface.PolyFlags);
		stream->WriteIndex(surface.pBase);
		stream->WriteIndex(surface.vNormal);
		stream->WriteIndex(surface.vTextureU);
		stream->WriteIndex(surface.vTextureV);
		stream->WriteIndex(surface.LightMap);
		stream->WriteIndex(surface.BrushPoly);
		stream->WriteInt16(surface.PanU);
		stream->WriteInt16(surface.PanV);
		stream->WriteObject(surface.BrushActor);
	}
}
