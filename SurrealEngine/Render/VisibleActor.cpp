
#include "Precomp.h"
#include "VisibleActor.h"
#include "VisibleFrame.h"
#include "VisibleBrush.h"
#include "VisibleMesh.h"
#include "VisibleSprite.h"

void VisibleActor::Process(VisibleFrame* frame, UActor* actor)
{
	if (actor->bCorona())
		frame->Coronas.push_back(actor);

	if (actor->bHidden())
		return;

	UActor* viewportActor = engine->viewport->Actor();
	bool isOwnedByViewport = actor->IsOwnedBy(viewportActor);

	bool behindView = frame->PortalDepth > 0;
	if (UPawn* pawn = UObject::TryCast<UPawn>(viewportActor))
	{
		if (pawn->bBehindView())
			behindView = true;
	}

	if ((behindView || !isOwnedByViewport) && actor->bOnlyOwnerSee())
		return;

	if (engine->LaunchInfo.engineVersion > 219)
	{
		if (!behindView && isOwnedByViewport && actor->bOwnerNoSee())
			return;
	}

	if (!behindView && actor == engine->CameraActor) // Hide CameraActor as if bOwnerNoSee was set for it
		return;

	if (!frame->Clipper.IsAABBVisible(actor->BspInfo.BoundingBox))
		return;

	EDrawType dt = (EDrawType)actor->DrawType();
	if (dt == DT_Mesh && actor->Mesh())
	{
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
	else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && actor->Texture())
	{
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
	else if (dt == DT_Brush && actor->Brush() && UActor::TryCast<UMover>(actor))
	{
		Type = dt;
		Actor = actor;
		frame->Actors.push_back(*this);
	}
}

void VisibleActor::DrawOpaque(VisibleFrame* frame)
{
	if (Type == DT_Mesh)
	{
		VisibleMesh vismesh;
		if (vismesh.DrawMesh(frame, Actor, false, false))
		{
			vec3 v = Actor->Location() - frame->ViewLocation.xyz();
			frame->Translucents.emplace_back(*this, dot(v, v));
		}
	}
	else if (Type == DT_Sprite || Type == DT_SpriteAnimOnce)
	{
		// Assume all sprites are translucent for now
		vec3 v = Actor->Location() - frame->ViewLocation.xyz();
		frame->Translucents.emplace_back(*this, dot(v, v));
	}
	else if (Type == DT_Brush)
	{
		VisibleBrush visbrush;
		if (visbrush.Draw(frame, Actor, false))
		{
			vec3 v = Actor->Location() - frame->ViewLocation.xyz();
			frame->Translucents.emplace_back(*this, dot(v, v));
		}
	}
}

void VisibleActor::DrawTranslucent(VisibleFrame* frame)
{
	if (Type == DT_Mesh)
	{
		VisibleMesh vismesh;
		vismesh.DrawMesh(frame, Actor, false, true);
	}
	else if (Type == DT_Sprite || Type == DT_SpriteAnimOnce)
	{
		VisibleSprite vissprite;
		vissprite.Draw(frame, Actor);
	}
	else if (Type == DT_Brush)
	{
		VisibleBrush visbrush;
		visbrush.Draw(frame, Actor, true);
	}
}
