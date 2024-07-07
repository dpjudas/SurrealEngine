
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

void RenderSubsystem::DrawBrush(FSceneNode* frame, UActor* actor)
{
	UModel* brush = actor->Brush();
	const vec3& location = actor->Location();
	float drawscale = actor->DrawScale();
	FSceneNode brushframe = *frame;

	UpdateActorLightList(actor);

	brushframe.ObjectToWorld = mat4::translate(location) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::translate(-actor->PrePivot()) * mat4::scale(drawscale);

	Device->SetSceneNode(&brushframe);

	for (const Poly& poly : brush->Polys->Polys)
	{
		DrawBrushPoly(&brushframe, brush, poly, 0, actor);
	}

	for (const Poly& poly : brush->Polys->Polys)
	{
		DrawBrushPoly(&brushframe, brush, poly, 1, actor);
	}

	Device->SetSceneNode(frame);
}

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
		lightmap = GetBrushLightmap(actor, poly, zoneActor, model, frame->ObjectToWorld);
		//fogmap = GetSurfaceFogmap(poly, engine->CameraActor->Region().Zone, model);
	}

	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	Device->DrawComplexSurface(&Scene.Frame, surfaceinfo, facet);
}
