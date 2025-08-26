
#include "Precomp.h"
#include "VisibleNode.h"
#include "VisibleDecal.h"
#include "RenderSubsystem.h"
#include "Engine.h"
#include "UObject/ULevel.h"

void VisibleNode::Draw(VisibleFrame* frame)
{
	UModel* model = engine->Level->Model;
	BspSurface& surface = model->Surfaces[Node->Surf];

	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	engine->render->UpdateTexture(surface.Material);

	// Try to find the Zone the surface is in using the corresponding node, to obtain its ZoneInfo actor.
	// Checking for Zone1 first seems to work better, as otherwise the clouds in CTF-LavaGiant remain fast.
	// Might return NULL if there is no corresponding ZoneInfo actor for the given Zone.
	auto zoneInfo = UObject::Cast<UZoneInfo>(model->Zones[Node->Zone1].ZoneActor);
	if (!zoneInfo)
		zoneInfo = UObject::Cast<UZoneInfo>(model->Zones[Node->Zone0].ZoneActor);

	// If no ZoneInfo is found, use the values from LevelInfo instead.
	float ZoneUPanSpeed = zoneInfo ? zoneInfo->TexUPanSpeed() : engine->LevelInfo->TexUPanSpeed();
	float ZoneVPanSpeed = zoneInfo ? zoneInfo->TexVPanSpeed() : engine->LevelInfo->TexVPanSpeed();

	FTextureInfo texture;
	if (surface.Material)
	{
		UTexture* tex = surface.Material->GetAnimTexture();
		engine->render->UpdateTexture(tex);
		engine->render->UpdateTextureInfo(texture, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
	}

	FTextureInfo detailtex;
	if (surface.Material && surface.Material->DetailTexture())
	{
		UTexture* tex = surface.Material->DetailTexture()->GetAnimTexture();
		engine->render->UpdateTexture(tex);
		engine->render->UpdateTextureInfo(detailtex, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
	}

	FTextureInfo macrotex;
	if (surface.Material && surface.Material->MacroTexture())
	{
		UTexture* tex = surface.Material->MacroTexture()->GetAnimTexture();
		engine->render->UpdateTexture(tex);
		engine->render->UpdateTextureInfo(macrotex, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
	}

	int numverts = Node->NumVertices;
	vec3* points = engine->render->GetTempVertexBuffer(numverts);

	BspVert* v = &model->Vertices[Node->VertPool];
	for (int j = 0; j < numverts; j++)
	{
		points[j] = model->Points[v[j].Vertex];
	}

	FSurfaceFacet facet;
	facet.MapCoords.Origin = Base;
	facet.MapCoords.XAxis = UVec;
	facet.MapCoords.YAxis = VVec;
	facet.Vertices = points;
	facet.VertexCount = numverts;

	FTextureInfo lightmap;
	FTextureInfo fogmap;
	if ((PolyFlags & PF_Unlit) == 0)
	{
		UZoneInfo* zoneActor = !model->Zones.empty() ? UObject::TryCast<UZoneInfo>(model->Zones[Node->Zone1].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = engine->render->GetSurfaceLightmap(surface, facet, zoneActor, model);
		fogmap = engine->render->GetSurfaceFogmap(surface, facet, engine->CameraActor->Region().Zone, model);
	}

	if (PolyFlags & PF_Mirrored)
	{
		// We don't support mirrors or portal surfaces right now. Force them to be rendered as opaque
		PolyFlags = PF_Occlude;
	}

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.Texture = surface.Material ? &texture : nullptr;
	surfaceinfo.MacroTexture = surface.Material && surface.Material->MacroTexture() ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = surface.Material && surface.Material->DetailTexture() ? &detailtex : nullptr;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	frame->Device->DrawComplexSurface(&frame->Frame, surfaceinfo, facet);

	VisibleDecal decal;
	decal.DrawDecals(frame, Node);
}
