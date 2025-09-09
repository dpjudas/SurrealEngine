
#include "Precomp.h"
#include "VisibleBrush.h"
#include "VisibleFrame.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

bool VisibleBrush::Draw(VisibleFrame* frame, UActor* actor, bool translucentPass)
{
	UMover* mover = UActor::TryCast<UMover>(actor);
	if (!mover)
		return false;

	engine->render->Stats.Actors++;

	UModel* brush = mover->Brush();
	const vec3& location = mover->Location();
	FSceneNode brushframe = frame->Frame;

	engine->render->UpdateActorLightList(mover);

	brushframe.ObjectToWorld = mat4::translate(location) * Coords::Rotation(mover->Rotation()).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-mover->PrePivot());

	frame->Device->SetSceneNode(&brushframe);
	bool needTranslucentPass = false;

	for (const Poly& poly : brush->Polys->Polys)
	{
		if (DrawBrushPoly(frame, brush, poly, 0, mover, translucentPass))
			needTranslucentPass = true;
	}

	for (const Poly& poly : brush->Polys->Polys)
	{
		if (DrawBrushPoly(frame, brush, poly, 1, mover, translucentPass))
			needTranslucentPass = true;
	}

	frame->Device->SetSceneNode(&frame->Frame);
	return needTranslucentPass;
}

bool VisibleBrush::DrawBrushPoly(VisibleFrame* frame, UModel* model, const Poly& poly, int pass, UMover* mover, bool translucentPass)
{
	uint32_t PolyFlags = poly.PolyFlags;

	if (!engine->getEditorMode() && PolyFlags & PolyFlags::PF_Invisible)
		return false;

	bool isTranslucent = (PolyFlags & (PF_Translucent | PF_Modulated | PF_Highlighted)) != 0;
	if (isTranslucent && !translucentPass)
		return true;
	else if (!isTranslucent && translucentPass)
		return false;

	//UpdateTexture(poly.Texture);

	auto zoneActor = mover->Region().Zone;
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
		engine->render->UpdateTexture(tex);
		engine->render->UpdateTextureInfo(texture, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
		surfaceinfo.Texture = &texture;

		if (polyTex->DetailTexture())
		{
			UTexture* tex = polyTex->DetailTexture()->GetAnimTexture();
			engine->render->UpdateTexture(tex);
			engine->render->UpdateTextureInfo(detailtex, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
			surfaceinfo.DetailTexture = &detailtex;
		}

		if (polyTex->MacroTexture())
		{
			UTexture* tex = poly.Texture->MacroTexture()->GetAnimTexture();
			engine->render->UpdateTexture(tex);
			engine->render->UpdateTextureInfo(macrotex, poly, tex, ZoneUPanSpeed, ZoneVPanSpeed);
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
		lightmap = engine->render->GetBrushLightmap(mover, poly, zoneActor, model);
		fogmap = engine->render->GetBrushFogmap(mover, poly, zoneActor, model);
	}

	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	frame->Device->DrawComplexSurface(&frame->Frame, surfaceinfo, facet);
	return false;
}
