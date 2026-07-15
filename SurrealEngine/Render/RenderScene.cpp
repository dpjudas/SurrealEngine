
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

	// HeadCoords is set separately (rather than folded into ViewRotation) so that
	// VisibleFrame::DrawPortals() can correctly combine it into each sky/portal/mirror sub-frame's own
	// view matrix exactly once, instead of it accumulating across nested portals - see VisibleFrame::HeadCoords.
	VisibleFrame::ViewSetup setup;
	setup.Location = engine->CameraLocation;
	setup.ViewRotation = Coords::Rotation(engine->CameraRotation);

	if (CurrentVREye)
	{
		BuildVREyeViewProjection(setup.WorldToView, setup.ProvidedProjection, setup.HeadCoords);
		setup.UseProvidedProjection = true;
	}
	else
	{
		setup.WorldToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Location(engine->CameraLocation).ToMatrix();
	}

	MainFrame.Process(setup);
	MainFrame.Draw();
	MainFrame.DrawCoronas();
}

void RenderSubsystem::BuildVREyeViewProjection(mat4& worldToView, mat4& projection, Coords& head) const
{
	BuildVREyeViewProjection(worldToView, projection, head, engine->CameraLocation, engine->CameraRotation);
}

void RenderSubsystem::BuildVREyeViewProjection(mat4& worldToView, mat4& projection, Coords& head, const vec3& cameraLocation, const Rotator& cameraRotation) const
{
	// The head pose is relative to the play space anchor - CameraLocation/CameraRotation (still driven
	// by PlayerCalcView/mouse yaw as usual) - not a replacement for it. Caller must ensure CurrentVREye
	// is set (this is only meaningful while rendering a VR eye). cameraLocation/cameraRotation are passed
	// in separately (rather than always reading engine->CameraLocation/CameraRotation) so DrawVRMenuPlane()
	// can render against a frozen camera reference instead of a live one that might still be animating
	// (e.g. a cinematic) - see VRMenuFrozenCameraLocation/Rotation.
	head.Origin = CurrentVREye->Position;
	head.XAxis = CurrentVREye->Forward;
	head.YAxis = CurrentVREye->Right;
	head.ZAxis = CurrentVREye->Up;

	worldToView = Coords::ViewToRenderDev().ToMatrix() * head.Inverse().ToMatrix() * Coords::Rotation(cameraRotation).Inverse().ToMatrix() * Coords::Location(cameraLocation).ToMatrix();

	const float n = 1.0f; // matches the near plane used below and by every other Projection in this renderer
	projection = mat4::frustum(std::tan(CurrentVREye->AngleLeft) * n, std::tan(CurrentVREye->AngleRight) * n, std::tan(CurrentVREye->AngleDown) * n, std::tan(CurrentVREye->AngleUp) * n, n, 32768.0f, handedness::left, clipzrange::zero_positive_w);
}
