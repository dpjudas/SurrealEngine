
#include "Precomp.h"
#include "UBspNodes.h"
#include "Packages/Engine/Actors/UActor.h"

void UBspNodes::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		BspNode node;
		node.PlaneX = stream->ReadFloat();
		node.PlaneY = stream->ReadFloat();
		node.PlaneZ = stream->ReadFloat();
		node.PlaneW = stream->ReadFloat();
		node.ZoneMask = stream->ReadUInt64();
		node.NodeFlags = stream->ReadUInt8();
		node.VertPool = stream->ReadIndex();
		node.Surf = stream->ReadIndex();
		node.Back = stream->ReadIndex();
		node.Front = stream->ReadIndex();
		node.Plane = stream->ReadIndex();
		node.CollisionBound = stream->ReadIndex();
		node.RenderBound = stream->ReadIndex();
		node.Zone0 = stream->ReadIndex();
		node.Zone1 = stream->ReadIndex();
		node.NumVertices = stream->ReadUInt8();
		node.Leaf0 = stream->ReadInt32();
		node.Leaf1 = stream->ReadInt32();
		Nodes.push_back(node);
	}

	int32_t NumZones = stream->ReadIndex();
	for (int i = 0; i < NumZones; i++)
	{
		ZoneProperties zone;
		zone.ZoneActor = stream->ReadObject<UActor>();
		zone.Connectivity = stream->ReadUInt64();
		zone.Visibility = stream->ReadUInt64();
		Zones.push_back(zone);
	}
}

void UBspNodes::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	int count = (int)Nodes.size();
	int maxcount = count;
	stream->WriteInt32(count);
	stream->WriteInt32(maxcount);
	for (const BspNode& node : Nodes)
	{
		stream->WriteFloat(node.PlaneX);
		stream->WriteFloat(node.PlaneY);
		stream->WriteFloat(node.PlaneZ);
		stream->WriteFloat(node.PlaneW);
		stream->WriteUInt64(node.ZoneMask);
		stream->WriteUInt8(node.NodeFlags);
		stream->WriteIndex(node.VertPool);
		stream->WriteIndex(node.Surf);
		stream->WriteIndex(node.Back);
		stream->WriteIndex(node.Front);
		stream->WriteIndex(node.Plane);
		stream->WriteIndex(node.CollisionBound);
		stream->WriteIndex(node.RenderBound);
		stream->WriteIndex(node.Zone0);
		stream->WriteIndex(node.Zone1);
		stream->WriteUInt8(node.NumVertices);
		stream->WriteInt32(node.Leaf0);
		stream->WriteInt32(node.Leaf1);
	}

	stream->WriteIndex((int)Zones.size());
	for (const ZoneProperties& zone : Zones)
	{
		stream->WriteObject(zone.ZoneActor);
		stream->WriteUInt64(zone.Connectivity);
		stream->WriteUInt64(zone.Visibility);
	}
}
