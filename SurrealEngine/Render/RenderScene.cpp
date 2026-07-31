
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "VisibleFrame.h"
#include "Packages/Engine/Actors/Pawn/UPawn.h"
#include "Packages/Extension/Windows/UViewportWindow.h"
#include "Packages/Extension/Windows/TabGroup/URootWindow.h"

void RenderSubsystem::DrawScene()
{
	if (!engine->Level)
		return;

	Light.FogFrameCounter++;
	TextureFrameCounter++;

	// Make sure all actors are at the right location in the BSP
	for (UActor* actor : engine->Level->Actors)
	{
		if (actor)
			actor->UpdateBspInfo();
	}

	mat4 worldToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Location(engine->CameraLocation).ToMatrix();
	MainFrame.Process(engine->CameraLocation, worldToView, Coords::Rotation(engine->CameraRotation));
	MainFrame.Draw();
	MainFrame.DrawCoronas();
}

void RenderSubsystem::DrawViewport(UViewportWindow* viewport)
{
	if (!engine->Level)
		return;

	float x = 0.0f, y = 0.0f;
	engine->dxRootWindow->ConvertCoordinates(viewport, 0.0f, 0.0f, engine->dxRootWindow, x, y);
	engine->dxRootWindow->SetRenderViewport(x, y, viewport->Width(), viewport->Height());

	if (viewport->bClearZ())
		Device->ClearZ();

	bool originActorWasHidden = false;
	vec3 location;
	Rotator rotation(0,0,0);
	if (UActor* originActor = viewport->originActor())
	{
		originActorWasHidden = originActor->bHidden();
		originActor->bHidden() = true;
		location = originActor->Location() + viewport->relLocation();
		if (viewport->bUseEyeHeight())
		{
			if (auto pawn = UObject::TryCast<UPawn>(originActor))
				location.z += pawn->BaseEyeHeight();
		}
		rotation = originActor->Rotation();
	}
	else
	{
		location = viewport->Location() + viewport->relLocation();
		rotation = viewport->Rotation();
	}

	if (viewport->bUseViewRotation())
	{
		rotation = engine->CameraRotation;
	}
	else if (UActor* watchActor = viewport->watchActor())
	{
		vec3 lookAt = watchActor->Location();
		if (viewport->bWatchEyeHeight())
		{
			if (auto pawn = UObject::TryCast<UPawn>(watchActor))
				lookAt.z += pawn->BaseEyeHeight();
		}
		rotation = Rotator::FromVector(lookAt - location);
	}

	rotation += viewport->relRotation();

	mat4 worldToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(rotation).Inverse().ToMatrix() * Coords::Location(location).ToMatrix();
	MainFrame.Process(location, worldToView, Coords::Rotation(rotation));
	MainFrame.Draw();
	MainFrame.DrawCoronas();

	Device->SetSceneNode(&Canvas.Frame);

	engine->dxRootWindow->ResetRenderViewport();

	if (UActor* originActor = viewport->originActor())
		originActor->bHidden() = originActorWasHidden;
}
