#pragma once

#include "Math/vec.h"
#include "VRSubsystem.h"

// Drives the player pawn from the headset and motion controllers: thumbstick movement and turning,
// controller buttons, and walking the pawn around so that it stays underneath the headset.
//
// Ticked from Engine::UpdateInput, which runs before the level ticks, so everything written here is
// picked up by the same frame's PlayerInput/PlayerTick. Nothing here reaches into the pawn's physics:
// movement is expressed as the same aBaseY/aStrafe axes the keyboard drives and turning as the same
// ViewRotation the mouse drives, so the game's own scripts stay in charge of what that means.
class VRPlayerInput
{
public:
	void Tick(float timeElapsed);

	// The head's horizontal position in play space, as of the last room-scale move. The renderer must
	// subtract this from the eye it draws from, or room-scale movement lands twice - once by having
	// moved the pawn (and with it the camera), and again as a view offset on top. Zero when room-scale
	// movement is off, which leaves the renderer's head offset untouched.
	// See RenderSubsystem::BuildVREyeView.
	vec2 GetAppliedHeadOffset() const { return AppliedHeadXY; }

private:
	void UpdateButtons();
	// allowStick false zeroes the stick but still writes the axes - see the call site.
	void UpdateMovement(bool allowStick);
	void UpdateTurning(float timeElapsed);
	void UpdateRoomScale();

	int GetMovementHand() const;          // whose stick is pushed
	int GetMovementDirectionHand() const; // whose controller aims it (controller reference only)

	vec2 AppliedHeadXY = vec2(0.0f);
	bool HasHeadReference = false;

	bool ButtonWasDown[VRSubsystem::HandCount][VRSubsystem::ButtonCount] = {};
	// Snap turning fires once per flick, not once per frame the stick is held over.
	bool SnapTurnArmed = true;
};
