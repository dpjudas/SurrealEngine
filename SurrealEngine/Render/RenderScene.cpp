
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

	mat4 worldToView;
	bool useProvidedProjection = false;
	mat4 providedProjection = mat4::identity();
	Coords head = Coords::Identity();

	if (CurrentVREye)
	{
		// The head pose is relative to the play space anchor - CameraLocation/CameraRotation (still
		// driven by PlayerCalcView/mouse yaw as usual) - not a replacement for it.
		head.Origin = CurrentVREye->Position;
		head.XAxis = CurrentVREye->Forward;
		head.YAxis = CurrentVREye->Right;
		head.ZAxis = CurrentVREye->Up;

		worldToView = Coords::ViewToRenderDev().ToMatrix() * head.Inverse().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Location(engine->CameraLocation).ToMatrix();

		const float n = 1.0f; // matches the near plane used below and by every other Projection in this renderer
		providedProjection = mat4::frustum(std::tan(CurrentVREye->AngleLeft) * n, std::tan(CurrentVREye->AngleRight) * n, std::tan(CurrentVREye->AngleDown) * n, std::tan(CurrentVREye->AngleUp) * n, n, 32768.0f, handedness::left, clipzrange::zero_positive_w);
		useProvidedProjection = true;
	}
	else
	{
		worldToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Location(engine->CameraLocation).ToMatrix();
	}

	// head is passed through separately (rather than folded into the viewRotation param below) so that
	// VisibleFrame::DrawPortals() can correctly combine it into each sky/portal/mirror sub-frame's own
	// view matrix exactly once, instead of it accumulating across nested portals - see VisibleFrame::HeadCoords.
	MainFrame.Process(engine->CameraLocation, worldToView, Coords::Rotation(engine->CameraRotation), false, 0, {}, vec4(0.0f, 0.0f, 0.0f, 1.0f), useProvidedProjection, providedProjection, head);
	MainFrame.Draw();
	MainFrame.DrawCoronas();
}
