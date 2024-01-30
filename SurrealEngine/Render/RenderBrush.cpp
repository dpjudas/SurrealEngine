
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

// #define USE_BRUSH_BSP

void RenderSubsystem::DrawBrush(FSceneNode* frame, UActor* actor)
{
	UModel* brush = actor->Brush();
	const vec3& location = actor->Location();
	float drawscale = actor->DrawScale();
	FSceneNode brushframe = *frame;

	UpdateActorLightList(actor);

	brushframe.ObjectToWorld = mat4::translate(location) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::translate(-actor->PrePivot()) * mat4::scale(drawscale);

	Device->SetSceneNode(&brushframe);

#ifndef USE_BRUSH_BSP
	// Draw the original polygons the bsp is compiled from
	for (const Poly& poly : brush->Polys->Polys)
	{
		DrawBrushPoly(frame, brush, poly, 0, actor);
	}

	for (const Poly& poly : brush->Polys->Polys)
	{
		DrawBrushPoly(frame, brush, poly, 1, actor);
	}
#else
	// Draw the bsp nodes
	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(frame, brush, node, 0, actor);
	}

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(frame, brush, node, 1, actor);
	}
#endif

	Device->SetSceneNode(frame);
}

#ifndef USE_BRUSH_BSP

void RenderSubsystem::DrawBrushPoly(FSceneNode* frame, UModel* model, const Poly& poly, int pass, UActor* actor)
{
	uint32_t PolyFlags = poly.PolyFlags;

	//UpdateTexture(poly.Texture);

	auto zoneActor = actor->Region().Zone;
	if (!zoneActor)
		zoneActor = engine->LevelInfo;
	float ZoneUPanSpeed = zoneActor->TexUPanSpeed();
	float ZoneVPanSpeed = zoneActor->TexVPanSpeed();

	FSurfaceInfo surfaceinfo;
	UTexture* polyTex = poly.Texture;
	if (!polyTex)
		polyTex = engine->LevelInfo->DefaultTexture();

	FTextureInfo texture;
	FTextureInfo detailtex;
	FTextureInfo macrotex;

	if (polyTex)
	{
		UTexture* tex = polyTex->GetAnimTexture();
		UpdateTexture(tex);
		UpdateTextureInfo(texture, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
		surfaceinfo.Texture = &texture;

		if (polyTex->DetailTexture())
		{
			UTexture* tex = polyTex->DetailTexture()->GetAnimTexture();
			UpdateTexture(tex);
			UpdateTextureInfo(detailtex, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.DetailTexture = &detailtex;
		}

		if (polyTex->MacroTexture())
		{
			UTexture* tex = poly.Texture->MacroTexture()->GetAnimTexture();
			UpdateTexture(tex);
			UpdateTextureInfo(macrotex, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.MacroTexture = &macrotex;
		}
	}

	int numverts = (int)poly.Vertices.size();
	const vec3* points = poly.Vertices.data();

	FSurfaceFacet facet;
	facet.MapCoords.Origin = poly.Base;
	facet.MapCoords.XAxis = poly.TextureU;
	facet.MapCoords.YAxis = poly.TextureV;
	facet.Vertices = const_cast<vec3*>(points);
	facet.VertexCount = numverts;

	FTextureInfo lightmap;
	FTextureInfo fogmap;
	if ((PolyFlags & PF_Unlit) == 0)
	{
		lightmap = GetBrushLightmap(actor, poly, zoneActor, model);
		//fogmap = GetSurfaceFogmap(poly, engine->CameraActor->Region().Zone, model);
	}

	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	Device->DrawComplexSurface(&Scene.Frame, surfaceinfo, facet);
}

#else

void RenderSubsystem::DrawNodeSurfaceGouraud(FSceneNode* frame, UModel* model, const BspNode& node, int pass, UActor* actor)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return;

	BspSurface& surface = model->Surfaces[node.Surf];
	uint32_t PolyFlags = surface.PolyFlags;

	if (surface.PolyFlags & (PF_Invisible | PF_FakeBackdrop))
		return;

	bool opaqueSurface = (PolyFlags & PF_NoOcclude) == 0;
	if ((pass == 0 && !opaqueSurface) || (pass == 1 && opaqueSurface))
		return;


	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	UpdateTexture(surface.Material);

	// Try to find the Zone the surface is in using the corresponding node, to obtain its ZoneInfo actor.
	// Checking for Zone1 first seems to work better, as otherwise the clouds in CTF-LavaGiant remain fast.
	// Might return NULL if there is no corresponding ZoneInfo actor for the given Zone.
	UZoneInfo* zoneInfo = actor->Region().Zone;

	// If no ZoneInfo is found, use the values from LevelInfo instead.
	float ZoneUPanSpeed = zoneInfo ? zoneInfo->TexUPanSpeed() : engine->LevelInfo->TexUPanSpeed();
	float ZoneVPanSpeed = zoneInfo ? zoneInfo->TexVPanSpeed() : engine->LevelInfo->TexVPanSpeed();

	FSurfaceInfo surfaceinfo;
	UTexture* surfaceTex = surface.Material;
	if (!surfaceTex)
		surfaceTex = engine->LevelInfo->DefaultTexture();

	FTextureInfo texture;
	FTextureInfo detailtex;
	FTextureInfo macrotex;

	if (surfaceTex)
	{
		UTexture* tex = surfaceTex->GetAnimTexture();
		UpdateTexture(tex);
		UpdateTextureInfo(texture, surface, surfaceTex, ZoneUPanSpeed, ZoneVPanSpeed);

		if (surfaceTex->DetailTexture())
		{
			UTexture* tex = surface.Material->DetailTexture()->GetAnimTexture();
			UpdateTexture(tex);
			UpdateTextureInfo(detailtex, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.DetailTexture = &detailtex;
		}

		if (surfaceTex->MacroTexture())
		{
			UTexture* tex = surface.Material->MacroTexture()->GetAnimTexture();
			UpdateTexture(tex);
			UpdateTextureInfo(macrotex, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.MacroTexture = &macrotex;
		}
	}

	int numverts = node.NumVertices;
	vec3* points = GetTempVertexBuffer(numverts);

	BspVert* v = &model->Vertices[node.VertPool];
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
		UZoneInfo* zoneActor = !model->Zones.empty() ? static_cast<UZoneInfo*>(model->Zones[node.Zone1].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = GetSurfaceLightmap(surface, facet, zoneActor, model);
		fogmap = GetSurfaceFogmap(surface, facet, engine->CameraActor->Region().Zone, model);
	}

	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.Texture = &texture;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	Device->DrawComplexSurface(&Scene.Frame, surfaceinfo, facet);
}

#endif
