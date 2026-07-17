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

	// Strips the head offset that room-scale walking has already moved the pawn by out of a play-space
	// position, leaving only what that position is relative to the head itself. Everything that places
	// something from a play-space pose against the camera - the eyes, the hands - has to agree on this or
	// they drift apart from each other as the player walks around their room.
	//
	// Note that this subtracts the offset the pawn was actually moved to match, not the one the head has
	// right this instant. The two are a frame apart (the pawn moves during the input tick, from the pose
	// the previous frame located), and it is the applied one that the camera's position corresponds to,
	// so it is the one that cancels exactly.
	//
	// Reads back the position unchanged when room-scale movement is off, which leaves the whole head
	// offset in place as a pure view offset - i.e. the pawn stays put and the player's viewpoint moves on
	// its own.
	vec3 RemoveRoomScaleOffset(const vec3& playSpacePosition) const
	{
		return vec3(playSpacePosition.x - AppliedHeadXY.x, playSpacePosition.y - AppliedHeadXY.y, playSpacePosition.z);
	}

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
	// Whether the pointer hand's trigger press was routed to a menu mouse click, so its release routes the
	// same way even if the click closed the menu in between. See UpdateButtons.
	bool PointerTriggerIsMenuClick = false;
	// Snap turning fires once per flick, not once per frame the stick is held over.
	bool SnapTurnArmed = true;
};
