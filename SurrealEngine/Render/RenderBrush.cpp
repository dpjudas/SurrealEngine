
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

	brushframe.ObjectToWorld = mat4::translate(location) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::scale(drawscale);

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

	UpdateTexture(poly.Texture);

	auto zoneActor = actor->Region().Zone;
	if (!zoneActor)
		zoneActor = engine->LevelInfo;
	float ZoneUPanSpeed = zoneActor->TexUPanSpeed();
	float ZoneVPanSpeed = zoneActor->TexVPanSpeed();

	FTextureInfo texture;
	if (poly.Texture)
	{
		UTexture* tex = poly.Texture->GetAnimTexture();
		UpdateTexture(tex);
		texture.CacheID = (uint64_t)(ptrdiff_t)tex;
		texture.bRealtimeChanged = tex->TextureModified;
		texture.UScale = tex->DrawScale();
		texture.VScale = tex->DrawScale();
		texture.Pan.x = -(float)poly.PanU;
		texture.Pan.y = -(float)poly.PanV;
		texture.Texture = tex;
		texture.Format = texture.Texture->ActualFormat;
		texture.Mips = texture.Texture->Mipmaps.data();
		texture.NumMips = (int)texture.Texture->Mipmaps.size();
		texture.USize = texture.Texture->USize();
		texture.VSize = texture.Texture->VSize();
		if (texture.Texture->Palette())
			texture.Palette = (FColor*)texture.Texture->Palette()->Colors.data();

		if (poly.Texture->TextureModified)
			poly.Texture->TextureModified = false;

		if (PolyFlags & PF_AutoUPan) texture.Pan.x -= AutoUV * ZoneUPanSpeed;
		if (PolyFlags & PF_AutoVPan) texture.Pan.y -= AutoUV * ZoneVPanSpeed;
	}

	FTextureInfo detailtex;
	if (poly.Texture && poly.Texture->DetailTexture())
	{
		UTexture* tex = poly.Texture->DetailTexture()->GetAnimTexture();
		UpdateTexture(tex);
		detailtex.CacheID = (uint64_t)(ptrdiff_t)tex;
		detailtex.bRealtimeChanged = false;
		detailtex.UScale = tex->DrawScale();
		detailtex.VScale = tex->DrawScale();
		detailtex.Pan.x = -(float)poly.PanU;
		detailtex.Pan.y = -(float)poly.PanV;
		detailtex.Texture = tex;
		detailtex.Format = detailtex.Texture->ActualFormat;
		detailtex.Mips = detailtex.Texture->Mipmaps.data();
		detailtex.NumMips = (int)detailtex.Texture->Mipmaps.size();
		detailtex.USize = detailtex.Texture->USize();
		detailtex.VSize = detailtex.Texture->VSize();
		if (detailtex.Texture->Palette())
			detailtex.Palette = (FColor*)detailtex.Texture->Palette()->Colors.data();

		if (PolyFlags & PF_AutoUPan) detailtex.Pan.x -= AutoUV * ZoneUPanSpeed;
		if (PolyFlags & PF_AutoVPan) detailtex.Pan.y -= AutoUV * ZoneVPanSpeed;
	}

	FTextureInfo macrotex;
	if (poly.Texture && poly.Texture->MacroTexture())
	{
		UTexture* tex = poly.Texture->MacroTexture()->GetAnimTexture();
		UpdateTexture(tex);
		macrotex.CacheID = (uint64_t)(ptrdiff_t)tex;
		macrotex.bRealtimeChanged = false;
		macrotex.UScale = tex->DrawScale();
		macrotex.VScale = tex->DrawScale();
		macrotex.Pan.x = -(float)poly.PanU;
		macrotex.Pan.y = -(float)poly.PanV;
		macrotex.Format = macrotex.Texture->ActualFormat;
		macrotex.Texture = tex;
		macrotex.Mips = macrotex.Texture->Mipmaps.data();
		macrotex.NumMips = (int)macrotex.Texture->Mipmaps.size();
		macrotex.USize = macrotex.Texture->USize();
		macrotex.VSize = macrotex.Texture->VSize();
		if (macrotex.Texture->Palette())
			macrotex.Palette = (FColor*)macrotex.Texture->Palette()->Colors.data();

		if (PolyFlags & PF_AutoUPan) macrotex.Pan.x -= AutoUV * ZoneUPanSpeed;
		if (PolyFlags & PF_AutoVPan) macrotex.Pan.y -= AutoUV * ZoneVPanSpeed;
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
		// lightmap = GetSurfaceLightmap(surface, facet, zoneActor, model);
		// fogmap = GetSurfaceFogmap(surface, facet, engine->CameraActor->Region().Zone, model);
	}

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.Texture = poly.Texture ? &texture : nullptr;
	surfaceinfo.MacroTexture = poly.Texture && poly.Texture->MacroTexture() ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = poly.Texture && poly.Texture->DetailTexture() ? &detailtex : nullptr;
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

	if (surface.PolyFlags & (PF_Invisible | PF_FakeBackdrop))
		return;

	bool opaqueSurface = (surface.PolyFlags & PF_NoOcclude) == 0;
	if ((pass == 0 && !opaqueSurface) || (pass == 1 && opaqueSurface))
		return;

	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	if (!surface.Material)
		return;

	UpdateTexture(surface.Material);

	FTextureInfo texture;
	if (surface.Material)
	{
		UTexture* tex = surface.Material->GetAnimTexture();
		UpdateTexture(tex);

		texture.CacheID = (uint64_t)(ptrdiff_t)tex;
		texture.bRealtimeChanged = tex->TextureModified;
		texture.UScale = tex->DrawScale();
		texture.VScale = tex->DrawScale();
		texture.Pan.x = -(float)surface.PanU;
		texture.Pan.y = -(float)surface.PanV;
		texture.Texture = tex;
		texture.Format = texture.Texture->ActualFormat;
		texture.Mips = tex->Mipmaps.data();
		texture.NumMips = (int)tex->Mipmaps.size();
		texture.USize = tex->USize();
		texture.VSize = tex->VSize();
		if (tex->Palette())
			texture.Palette = (FColor*)tex->Palette()->Colors.data();

		if (tex->TextureModified)
			tex->TextureModified = false;

		if (surface.PolyFlags & PF_AutoUPan) texture.Pan.x -= AutoUV;
		if (surface.PolyFlags & PF_AutoVPan) texture.Pan.y -= AutoUV;
	}

	BspVert* v = &model->Vertices[node.VertPool];

	float UDot = dot(UVec, Base);
	float VDot = dot(VVec, Base);
	float UPan = UDot + texture.Pan.x;
	float VPan = VDot + texture.Pan.y;

	std::vector<GouraudVertex> vertices;
	for (int j = 0; j < node.NumVertices; j++)
	{
		GouraudVertex gv;
		gv.Point = model->Points[v[j].Vertex];
		gv.Light = GetVertexLight(actor, gv.Point, model->Vectors[surface.vNormal], surface.PolyFlags & PF_Unlit);
		gv.UV = { dot(UVec, gv.Point) - UPan, dot(VVec, gv.Point) - VPan };
		vertices.push_back(gv);
	}

	Device->DrawGouraudPolygon(frame, texture, vertices.data(), (int)vertices.size(), surface.PolyFlags);
}

#endif
