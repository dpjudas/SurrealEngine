
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
		BuildVREyeView(setup.WorldToView, setup.ProvidedProjection, setup.HeadCoords, setup.HeadPosition, engine->CameraLocation, engine->CameraRotation);
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

vec3 RenderSubsystem::RemoveRoomScaleOffset(const vec3& playSpacePosition) const
{
	// Room-scale walking has already moved the pawn - and with it the camera this offset is applied on
	// top of - to sit underneath the head, so the head's horizontal offset from the play space anchor
	// must not be applied a second time here. What is left over is the eye's offset from the head
	// itself: the IPD, which stereo depends on, plus how far the head is above the camera. See
	// VRPlayerInput::RemoveRoomScaleOffset, which the hands go through too.
	if (!engine->vrInput)
		return playSpacePosition;

	return engine->vrInput->RemoveRoomScaleOffset(playSpacePosition);
}

void RenderSubsystem::BuildVREyeView(mat4& worldToView, mat4& projection, Coords& headRotation, vec3& headPosition, const vec3& cameraLocation, const Rotator& cameraRotation) const
{
	// The head pose is relative to the play space anchor - CameraLocation/CameraRotation (still driven
	// by PlayerCalcView/mouse yaw as usual) - not a replacement for it. Caller must ensure CurrentVREye
	// is set (this is only meaningful while rendering a VR eye). cameraLocation/cameraRotation are passed
	// in separately (rather than always reading engine->CameraLocation/CameraRotation) so DrawVRMenuPlane()
	// can render against a frozen camera reference instead of a live one that might still be animating
	// (e.g. a cinematic) - see VRMenuFrozenCameraLocation/Rotation.
	//
	// The head's orientation and its position must go in as two separate terms. Coords::ToMatrix() emits
	// the origin as a plain negated translation without rotating it into the axes, which only agrees with
	// Coords' own operator* when the axes are identity. Coords::Location() is identity-axed so it is fine,
	// but a Coords carrying the eye's orientation *and* its position is not: folding them together makes
	// the matrix place the eye at -Position instead of +Position, i.e. head movement comes out inverted.
	headRotation.Origin = vec3(0.0f);
	headRotation.XAxis = CurrentVREye->Forward;
	headRotation.YAxis = CurrentVREye->Right;
	headRotation.ZAxis = CurrentVREye->Up;
	headPosition = RemoveRoomScaleOffset(CurrentVREye->Position);

	worldToView =
		Coords::ViewToRenderDev().ToMatrix() *
		headRotation.Inverse().ToMatrix() *
		Coords::Location(headPosition).ToMatrix() *
		Coords::Rotation(cameraRotation).Inverse().ToMatrix() *
		Coords::Location(cameraLocation).ToMatrix();

	const float n = 1.0f; // matches the near plane used below and by every other Projection in this renderer
	projection = mat4::frustum(std::tan(CurrentVREye->AngleLeft) * n, std::tan(CurrentVREye->AngleRight) * n, std::tan(CurrentVREye->AngleDown) * n, std::tan(CurrentVREye->AngleUp) * n, n, 32768.0f, handedness::left, clipzrange::zero_positive_w);
}
