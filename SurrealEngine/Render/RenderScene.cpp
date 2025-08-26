
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "VisibleFrame.h"

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

	// To do: use the zone specified in the surface with the PF_FakeBackdrop PolyFlags
	UZoneInfo* skyZone = nullptr;
	for (const auto& zone : engine->Level->Model->Zones)
	{
		UZoneInfo* zoneInfo = UObject::TryCast<UZoneInfo>(zone.ZoneActor);
		if (zoneInfo && zoneInfo->SkyZone())
		{
			skyZone = zoneInfo->SkyZone();
			break;
		}
	}

	if (skyZone)
	{
		mat4 skyToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Rotation(skyZone->Rotation()).ToMatrix() * Coords::Location(skyZone->Location()).ToMatrix();

		SkyFrame.Process(skyZone->Location(), skyToView);
		SkyFrame.Draw();
		Device->ClearZ();
	}

	mat4 worldToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Location(engine->CameraLocation).ToMatrix();
	MainFrame.Process(engine->CameraLocation, worldToView);
	MainFrame.Draw();
	MainFrame.DrawCoronas();
}
