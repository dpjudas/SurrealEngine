
#include "Precomp.h"
#include "ULevel.h"

#include "Engine.h"
#include "UActor.h"
#include "UTexture.h"
#include "UClass.h"
#include "VM/ScriptCall.h"

BBox BspNode::GetCollisionBox(UModel* model) const
{
	int32_t* hullIndexList = &model->LeafHulls[CollisionBound];
	int hullPlanesCount = 0;
	while (hullIndexList[hullPlanesCount] >= 0)
		hullPlanesCount++;

	vec3* bboxStart = (vec3*)(&hullIndexList[hullPlanesCount + 1]);

	BBox bbox;
	bbox.min = bboxStart[0];
	bbox.max = bboxStart[1];

	return bbox;
}

void ULevelBase::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	int32_t dbnum = stream->ReadInt32();
	int32_t dbmax = stream->ReadInt32();
	for (int32_t i = 0; i < dbnum; i++)
	{
		auto actor = stream->ReadObject<UActor>();
		if (actor)
			actor->Index = (int)Actors.size();
		Actors.push_back(actor);
	}

	Protocol = stream->ReadString();
	Host = stream->ReadString();
	if (!Host.empty())
		Port = stream->ReadInt32();
	Map = stream->ReadString();

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		Options.push_back(stream->ReadString());
	}

	Portal = stream->ReadString();

	stream->Skip(7);
}

/////////////////////////////////////////////////////////////////////////////

ULevel::ULevel(NameString name, UClass* base, ObjectFlags flags) : ULevelBase(name, base, flags)
{
	Collision.SetLevel(this);
}

void ULevel::Load(ObjectStream* stream)
{
	ULevelBase::Load(stream);

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LevelReachSpec spec;
		spec.distance = stream->ReadInt32();
		spec.startActor = stream->ReadIndex();
		spec.endActor = stream->ReadIndex();
		spec.collisionRadius = stream->ReadInt32();
		spec.collisionHeight = stream->ReadInt32();
		spec.reachFlags = stream->ReadInt32();
		spec.bPruned = stream->ReadInt8();
		ReachSpecs.push_back(spec);
	}

	Model = stream->ReadObject<UModel>();

	if (Model)
	{
		Model->LoadNow();
	}
}

void ULevel::TickActor(float elapsed, UActor* actor)
{
	if (!actor)
		return;

	// If we have an owner, tick it first
	if (!actor->bDeleteMe() && actor->Owner())
	{
		TickActor(elapsed, actor->Owner());
	}

	// Do we have an actor? is it deleted? did it already tick?
	if (actor->bDeleteMe() || actor->bTicked() == ticked)
		return;

	// Mark actor as ticked
	actor->bTicked() = ticked;

	// Tick the actor for this turn
	actor->Tick(elapsed);

	// Destroy the actor if its time
	if (actor->Role() >= ROLE_SimulatedProxy && actor->LifeSpan() != 0.0f)
	{
		actor->LifeSpan() = std::max(actor->LifeSpan() - elapsed, 0.0f);
		if (actor->LifeSpan() == 0.0f)
		{
			CallEvent(actor, EventName::Expired);
			actor->Destroy();
		}
	}
}

void ULevel::Tick(float elapsed)
{
	if (!engine->LevelInfo->bPlayersOnly())
	{
		for (size_t i = 0; i < Actors.size(); i++)
			if (Actors[i])
				TickActor(elapsed, Actors[i]);
	}
	else
	{
		for (size_t i = 0; i < Actors.size(); i++)
			if (auto player = UObject::TryCast<UPlayerPawn>(Actors[i]))
				TickActor(elapsed, player);
	}

	Array<UActor*> newActorList;
	newActorList.reserve(Actors.size());
	for (UActor* actor : Actors)
	{
		if (actor)
		{
			actor->Index = (int)newActorList.size();
			newActorList.push_back(actor);
		}
	}
	Actors.swap(newActorList);

	ticked = !ticked;
}

/////////////////////////////////////////////////////////////////////////////

void UModel::Load(ObjectStream* stream)
{
	UPrimitive::Load(stream);

	if (stream->GetVersion() <= 61)
	{
		UVectors* vectors = stream->ReadObject<UVectors>();
		UVectors* points = stream->ReadObject<UVectors>();
		UBspNodes* nodes = stream->ReadObject<UBspNodes>();
		UBspSurfs* surfaces = stream->ReadObject<UBspSurfs>();
		UVerts* verts = stream->ReadObject<UVerts>();

		vectors->LoadNow();
		points->LoadNow();
		nodes->LoadNow();
		surfaces->LoadNow();
		verts->LoadNow();

		Vectors = vectors->Vectors;
		Points = points->Vectors;
		Nodes = nodes->Nodes;
		Zones = nodes->Zones;
		if (Zones.size() < 64)
			Zones.resize(64);
		Surfaces = surfaces->Surfaces;
		Vertices = verts->Vertices;
		NumSharedSides = verts->NumSharedSides;
	}
	else
	{
		int count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			vec3 v;
			v.x = stream->ReadFloat();
			v.y = stream->ReadFloat();
			v.z = stream->ReadFloat();
			Vectors.push_back(v);
		}

		count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			vec3 v;
			v.x = stream->ReadFloat();
			v.y = stream->ReadFloat();
			v.z = stream->ReadFloat();
			Points.push_back(v);
		}

		count = stream->ReadIndex();
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

		count = stream->ReadIndex();
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
			surface.BrushActor = stream->ReadIndex();
			Surfaces.push_back(surface);
		}

		count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			BspVert vert;
			vert.Vertex = stream->ReadIndex();
			vert.Side = stream->ReadIndex();
			Vertices.push_back(vert);
		}

		NumSharedSides = stream->ReadInt32();

		int32_t NumZones = stream->ReadInt32();
		for (int i = 0; i < NumZones; i++)
		{
			ZoneProperties zone;
			zone.ZoneActor = stream->ReadObject<UActor>();
			zone.Connectivity = stream->ReadUInt64();
			zone.Visibility = stream->ReadUInt64();
			Zones.push_back(zone);
		}
		if (NumZones < 64)
			Zones.resize(64);
	}

	Polys = stream->ReadObject<UPolys>();

	static uint32_t NextLMCacheID = 0; // For easier unique CacheIDs for lightmap textures

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LightMapIndex entry;
		entry.DataOffset = stream->ReadInt32();
		entry.PanX = stream->ReadFloat();
		entry.PanY = stream->ReadFloat();
		entry.PanZ = stream->ReadFloat();
		entry.UClamp = stream->ReadIndex();
		entry.VClamp = stream->ReadIndex();
		entry.UScale = stream->ReadFloat();
		entry.VScale = stream->ReadFloat();
		entry.LightActors = stream->ReadInt32();
		entry.LMCacheID = NextLMCacheID++;
		LightMap.push_back(entry);
	}

	LightBits.resize(stream->ReadIndex());
	stream->ReadBytes(LightBits.data(), (uint32_t)LightBits.size());

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		BBox boundingBox;
		boundingBox.min.x = stream->ReadFloat();
		boundingBox.min.y = stream->ReadFloat();
		boundingBox.min.z = stream->ReadFloat();
		boundingBox.max.x = stream->ReadFloat();
		boundingBox.max.y = stream->ReadFloat();
		boundingBox.max.z = stream->ReadFloat();
		boundingBox.IsValid = stream->ReadInt8() != 0;
		Bounds.push_back(boundingBox);
	}

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LeafHulls.push_back(stream->ReadInt32());
	}

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		ConvexVolumeLeaf leaf;
		leaf.Zone = stream->ReadIndex();
		leaf.Permeating = stream->ReadIndex();
		leaf.Volumetric = stream->ReadIndex();
		leaf.VisibleZones = stream->ReadUInt64();
		Leaves.push_back(leaf);
	}

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		Lights.push_back(stream->ReadObject<UActor>());
	}

	if (stream->GetVersion() <= 61)
	{
		UObject* unknown1 = stream->ReadObject<UObject>();
		UObject* unknown2 = stream->ReadObject<UObject>();
	}

	RootOutside = stream->ReadInt32();
	Linked = stream->ReadInt32();
}

PointRegion UModel::FindRegion(const vec3& point, UZoneInfo* levelZoneInfo)
{
	PointRegion region;
	region.BspLeaf = 0;
	region.ZoneNumber = 0;

	vec4 location = vec4(point, 1.0f);

	// Search the BSP
	BspNode* nodes = Nodes.data();
	BspNode* node = nodes;
	while (true)
	{
		vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
		float side = dot(location, plane);
		if (node->Front >= 0 && side >= 0.0f)
		{
			node = nodes + node->Front;
		}
		else if (node->Back >= 0 && side <= 0.0f)
		{
			node = nodes + node->Back;
		}
		else
		{
			region.ZoneNumber = node->Zone1;
			region.BspLeaf = side >= 0.0f ? node->Leaf0 : node->Leaf1;
			break;
		}
	}

	region.Zone = UObject::Cast<UZoneInfo>(Zones[region.ZoneNumber].ZoneActor);
	if (!region.Zone)
		region.Zone = levelZoneInfo;

	return region;
}

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////

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
		surface.BrushActor = stream->ReadIndex();
		Surfaces.push_back(surface);
	}
}

/////////////////////////////////////////////////////////////////////////////

void UVectors::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		vec3 v;
		v.x = stream->ReadFloat();
		v.y = stream->ReadFloat();
		v.z = stream->ReadFloat();
		Vectors.push_back(v);
	}
}

/////////////////////////////////////////////////////////////////////////////

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
