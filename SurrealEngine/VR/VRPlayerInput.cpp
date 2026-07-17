
#include "Precomp.h"
#include "VRPlayerInput.h"
#include "Engine.h"
#include "GameWindow.h"
#include "LauncherSettings.h"
#include "UObject/UActor.h"
#include "UObject/UClient.h" // UViewport, which Engine.h only forward declares
#include "Math/coords.h"
#include <cmath>

namespace
{
	// Below this the stick is treated as centred. The runtime applies its own deadzone, but not every
	// one of them does, and a stick that rests at 0.02 would otherwise walk the player into a wall
	// over the course of a match.
	const float StickDeadzone = 0.15f;

	// Snap turning needs the stick pushed this far to fire, and released back inside the second value
	// before it can fire again. The gap is what stops a stick resting near the threshold from
	// machine-gunning turns.
	const float SnapTurnFireThreshold = 0.7f;
	const float SnapTurnRearmThreshold = 0.4f;

	// Same idea on the same stick's Y axis, for jump (up) and crouch (down). Higher than the snap-turn
	// threshold on purpose: this axis shares a stick with turning, and a diagonal flick meant as a turn
	// should not also jump.
	const float JumpCrouchFireThreshold = 0.8f;
	const float JumpCrouchRearmThreshold = 0.5f;

	const float DegreesToRotatorUnits = 65536.0f / 360.0f;

	// Half a metre of head movement inside one frame. Far beyond anything a person does at any sane
	// frame rate (a brisk 3 m/s is ~3cm at 90Hz), so it can only be the tracking jumping.
	const float TrackingJumpThreshold = 0.5f * MetersToUnrealUnits;

	// What a fully deflected stick is worth on aBaseY/aStrafe, chosen so that it means exactly the same
	// thing as holding a movement key. Engine::InputEvent turns a key press into delta 20, which
	// Engine::InputCommand multiplies by the binding's Speed= - and every stock UE1 game binds movement
	// at Speed=300 ("Axis aBaseY Speed=+300.0", "Axis aStrafe Speed=+300.0"), so a held key is 6000.
	//
	// Deliberately a constant rather than read back from the player's bindings. There is no single
	// Speed= to read: a stock Unreal ini also binds the same axes for an analog joystick at
	// "JoyX=Axis astrafe speed=2", so scanning the bindings finds either 300 or 2 depending on map
	// order and casing. The pawn's own GroundSpeed clamps the top speed regardless of what goes in
	// here, so this only shapes how partial deflection feels - turn it down if the stick is too twitchy
	// off centre.
	const float MoveAxisScale = 300.0f * 20.0f;

	// The flat direction the stick is measured against, as (forward, right) in the pawn's own frame.
	// Play space is anchored to the camera's rotation, so a pose's local XY is already relative to the
	// pawn's yaw and needs no trip through world space. Returns false if the direction is too close to
	// straight up or down to have a meaningful heading (looking at your feet, controller pointed at the
	// ceiling), in which case the caller should leave the last good heading alone rather than snap to
	// an arbitrary one.
	bool FlattenHeading(const vec3& direction, vec2& outHeading)
	{
		vec2 flat(direction.x, direction.y);
		float lengthSquared = dot(flat, flat);
		if (lengthSquared < 0.01f)
			return false;

		outHeading = flat / std::sqrt(lengthSquared);
		return true;
	}

	// The controller buttons land on IK_Joy1..IK_Joy16, laid out hand-major: left takes the first
	// ButtonCount keys, right the next.
	EInputKey ButtonToKey(int hand, int button)
	{
		return (EInputKey)(IK_Joy1 + hand * VRSubsystem::ButtonCount + button);
	}

	// Jump and crouch are edge-triggered from the stick rather than being buttons, so they have no Joy
	// key of their own to be released against - but Engine::InputEvent's release path cleans up
	// activeInputButtons/activeInputAxes *by key*, so they need one anyway. These are the two keys left
	// over after the buttons above, which VR claims and binds to nothing, so nothing else can generate
	// them and their bookkeeping can't collide with a real button's.
	const EInputKey JumpSyntheticKey = IK_Joy15;
	const EInputKey CrouchSyntheticKey = IK_Joy16;

	// At 7 buttons per hand the Joy range is exactly full: Joy1..Joy14 for the buttons, Joy15/Joy16 for
	// the two above. One more button and the right hand's keys would run off the end of IK_Joy16 - which
	// wouldn't fail, it would quietly deliver presses as IK_Escape and whatever else follows Joy16 in
	// EInputKey. So it's a build error instead. This is the only bound worth asserting: it is tighter
	// than "fits in Joy1..Joy16" and fires first.
	static_assert(VRSubsystem::HandCount * VRSubsystem::ButtonCount <= IK_Joy15 - IK_Joy1,
		"Too many VR buttons: hand * ButtonCount + button has grown into IK_Joy15/IK_Joy16, which are "
		"reserved for the synthetic jump/crouch presses. Drop a button, or stop mapping them onto Joy keys.");

	// What VR binds each controller button to, indexed [hand][button] in VRSubsystem::Button order.
	// Empty means deliberately bound to nothing.
	//
	// VR claims the whole Joy range rather than reading it from the player's ini, because the ini is
	// never neutral: Epic ships Joy1..Joy16 in DefUser.ini and every install inherits them, so a VR
	// player who has never touched a binding still starts with left grip on Jump, left B on Duck and the
	// right hand scattered across SwitchWeapon. Those defaults were written for a gamepad and are actively
	// wrong for hands. The cost is that Joy bindings in the ini no longer do anything while VR is on.
	const char* const VRButtonCommands[VRSubsystem::HandCount][VRSubsystem::ButtonCount] =
	{
		// Left hand: IK_Joy1..IK_Joy7
		{
			"Fire",          // Trigger
			"",              // Grip - freed from the stock Jump; phase 6 wants it for the item wheel
			"",              // A - reserved for the item wheel (phase 6)
			"",              // B
			"NextWeapon",    // ThumbstickClick - keeps weapon switching alive until the wheel exists
			"",              // Menu - the Index has no menu button
			"AltFire",       // Trackpad
		},
		// Right hand: IK_Joy8..IK_Joy14
		{
			"Fire",          // Trigger - also the menu click while a menu is up, see UpdateButtons
			"",              // Grip
			"",              // A - reserved for the weapon wheel (phase 6)
			"",              // B - intercepted as IK_Escape before it reaches a binding, see UpdateButtons
			"NextWeapon",    // ThumbstickClick
			"",              // Menu
			"AltFire",       // Trackpad
		},
	};
}

void VRPlayerInput::ApplyKeybindings()
{
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
	{
		for (int button = 0; button < VRSubsystem::ButtonCount; button++)
			engine->keybindings[Engine::keynames[ButtonToKey(hand, button)]] = VRButtonCommands[hand][button];
	}

	// Claimed too, so that a stock InventoryNext/InventoryPrevious binding can't fire when the jump and
	// crouch aliases borrow these keys for their press/release bookkeeping.
	engine->keybindings[Engine::keynames[JumpSyntheticKey]] = "";
	engine->keybindings[Engine::keynames[CrouchSyntheticKey]] = "";
}

void VRPlayerInput::Tick(float timeElapsed)
{
	VRSubsystem* vr = engine->vr.get();
	if (!vr || !vr->IsActive())
		return;

	vr->SyncInput();

	// Routed through the normal keybinding path, so the console and menus get first refusal on them the
	// same way they do for the keyboard.
	UpdateButtons();

	if (!engine->viewport || !engine->viewport->Actor())
		return;

	// While the console or menu has the screen, the keyboard cannot move the player: its keys are
	// swallowed by the console's KeyEvent before Engine::InputEvent ever turns them into an axis. The
	// stick writes the movement axes directly and so has no such thing happening to it - hence doing it
	// explicitly here, or pushing the stick while the pause menu is up would walk the player around
	// behind it.
	bool menuOpen = engine->console && engine->console->bNoDrawWorld();

	// Note UpdateMovement() still runs with the menu open, just ignoring the stick: the axes it writes
	// are absolute, so it has to write them every single frame. Skipping the call would leave whatever
	// the stick last said latched into the axis, and the player jogging into a wall behind the menu.
	UpdateMovement(!menuOpen);
	if (!menuOpen)
	{
		UpdateTurning(timeElapsed);
		UpdateJumpCrouch();
	}
	else
	{
		// Same reason the stick stops moving the player behind a menu. A crouch left latched down while
		// the menu is up would still be held when it closes, and the player has no way to see why.
		ReleaseStickAction();
	}

	// Room-scale keeps running even behind the menu, deliberately. Freezing it would let the player
	// physically walk away from their pawn and then get yanked back the moment they closed the menu,
	// whereas leaving it on just means the body keeps following the head like it always does.
	UpdateRoomScale();
}

int VRPlayerInput::GetMovementHand() const
{
	return LauncherSettings::Get().VR.MovementHand == VRHand::Left ? VRSubsystem::HandLeft : VRSubsystem::HandRight;
}

int VRPlayerInput::GetMovementDirectionHand() const
{
	return LauncherSettings::Get().VR.MovementDirectionHand == VRHand::Left ? VRSubsystem::HandLeft : VRSubsystem::HandRight;
}

void VRPlayerInput::UpdateButtons()
{
	VRSubsystem* vr = engine->vr.get();

	// The pause/main menu has the screen when the console has taken over the whole frame. While it does,
	// the pointer hand's trigger stands in for a left mouse click on the VR menu plane (RenderSubsystem
	// casts the laser and moves the cursor), so the player works the menu by pointing and pulling the
	// trigger instead of with a mouse they can't see in the headset.
	bool menuOpen = engine->console && engine->console->bNoDrawWorld();

	// Left hand takes the first ButtonCount Joy keys from IK_Joy1, right hand the next, in
	// VRSubsystem::Button order. VR has already overwritten what those keys are bound to
	// (ApplyKeybindings) - the point of going out through a key at all rather than driving the pawn
	// directly is that a controller button is then just another key, and gets the same aliases, console
	// and menu handling as one.
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
	{
		const VRSubsystem::ControllerState& controller = vr->GetController(hand);
		for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		{
			bool isDown = controller.Buttons[button];
			bool wasDown = ButtonWasDown[hand][button];
			if (isDown == wasDown)
				continue;

			ButtonWasDown[hand][button] = isDown;
			EInputType type = isDown ? IST_Press : IST_Release;

			// The pointer hand's B toggles the menu, standing in for the Escape key so it opens and closes
			// whatever Escape is bound to for this game - and it does so whether or not a menu is already
			// up. Routed as IK_Escape rather than a Joy key so it needs no ini binding of its own and can't
			// be accidentally rebound out from under the player.
			if (hand == VRSubsystem::MenuPointerHand && button == VRSubsystem::Button_B)
			{
				engine->InputEvent(IK_Escape, type);
				continue;
			}

			// While a menu is up, the pointer hand's trigger is the menu's left mouse button. The routing is
			// decided at press time and remembered, so a click that closes the menu still gets its matching
			// mouse-button release - otherwise the button would latch down, or an unpaired Joy release would
			// fire once the menu was gone.
			if (hand == VRSubsystem::MenuPointerHand && button == VRSubsystem::Button_Trigger)
			{
				if (isDown)
					PointerTriggerIsMenuClick = menuOpen;
				if (PointerTriggerIsMenuClick)
				{
					// Native URootWindow menus (Deus Ex et al.) take the click through their own window
					// event path, not the key input path - the same split Engine::OnWindowMouseDown makes for
					// the desktop mouse. dxRootWindow is null for the UT/Unreal script menus, which do read
					// IK_LeftMouse. Either way the cursor was already parked by UpdateVRMenuLaser, so the
					// position passed here is unused (the window reads its own tracked cursor).
					if (engine->dxRootWindow)
					{
						if (isDown)
							engine->dxRootWindow->OnWindowMouseDown(Point(0.0, 0.0), IK_LeftMouse);
						else
							engine->dxRootWindow->OnWindowMouseUp(Point(0.0, 0.0), IK_LeftMouse);
					}
					else
					{
						engine->InputEvent(IK_LeftMouse, type);
					}
					continue;
				}
			}

			engine->InputEvent(ButtonToKey(hand, button), type);
		}
	}
}

void VRPlayerInput::UpdateMovement(bool allowStick)
{
	const auto& settings = LauncherSettings::Get().VR;
	VRSubsystem* vr = engine->vr.get();

	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(engine->viewport->Actor());
	if (!player)
		return;

	vec2 stick = allowStick ? vr->GetController(GetMovementHand()).Thumbstick : vec2(0.0f);
	if (dot(stick, stick) < StickDeadzone * StickDeadzone)
		stick = vec2(0.0f);

	// Which way "forward on the stick" points. The hand whose controller aims the movement is a
	// separate setting from the hand whose stick is pushed, so that e.g. the left stick can walk you
	// wherever the right hand - the one holding the gun - is pointing.
	//
	// Controller mode falls back to the head when that controller isn't being tracked, so putting a
	// controller down mid-game leaves the player able to walk rather than stuck.
	const VRSubsystem::ControllerState& aimController = vr->GetController(GetMovementDirectionHand());
	vec2 heading = vec2(0.0f);
	bool haveHeading = false;
	if (settings.MovementReference == VRMovementReference::Controller && aimController.PoseValid)
		haveHeading = FlattenHeading(aimController.Forward, heading);
	if (!haveHeading && vr->GetHead().Valid)
		haveHeading = FlattenHeading(vr->GetHead().Forward, heading);

	float forward = 0.0f;
	float strafe = 0.0f;
	if (haveHeading)
	{
		// heading holds the reference's forward as (forward, right) in the pawn's own frame; turning
		// that 90 degrees to the right gives the reference's right axis. Laying the stick along those
		// two is what expresses it back in the frame the movement axes are measured in.
		const vec2 headingRight(-heading.y, heading.x);
		const vec2 move = (heading * stick.y + headingRight * stick.x) * MoveAxisScale;
		forward = move.x;
		strafe = move.y;
	}

	// Written absolutely, on top of whatever the keyboard contributes this frame, rather than added to
	// the axis' current value: the keyboard only writes these while a key is actually held, so a
	// read-modify-write would keep re-adding the stick's own contribution and run away.
	auto keyboardAxis = [](const char* name)
	{
		auto it = engine->activeInputAxes.find(name);
		return it != engine->activeInputAxes.end() ? it->second.Value : 0.0f;
	};

	// aStrafe, NOT aBaseX. aBaseX is the *turn* axis - it is what the Left/Right arrow keys bind to
	// ("Axis aBaseX Speed=-150.0", aliased TurnLeft/TurnRight), while strafing is its own axis bound to
	// A/D. Feeding sideways stick into aBaseX spins the player instead of stepping them sideways, and
	// spins them ever faster, because the resulting yaw change feeds straight back into the angle
	// between the pawn and the hand the stick is measured against.
	player->aBaseY() = keyboardAxis("aBaseY") + forward;
	player->aStrafe() = keyboardAxis("aStrafe") + strafe;
}

void VRPlayerInput::UpdateTurning(float timeElapsed)
{
	const auto& settings = LauncherSettings::Get().VR;
	if (settings.TurnMode == VRTurnMode::Off)
		return;

	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(engine->viewport->Actor());
	if (!player)
		return;

	// The turn stick is whichever one movement isn't on.
	float stickX = engine->vr->GetController(1 - GetMovementHand()).Thumbstick.x;

	int deltaYaw = 0;
	if (settings.TurnMode == VRTurnMode::Snap)
	{
		if (std::fabs(stickX) >= SnapTurnFireThreshold && SnapTurnArmed)
		{
			deltaYaw = (int)std::lround(settings.SnapTurnDegrees * DegreesToRotatorUnits);
			if (stickX < 0.0f)
				deltaYaw = -deltaYaw;
			SnapTurnArmed = false;
		}
		else if (std::fabs(stickX) <= SnapTurnRearmThreshold)
		{
			SnapTurnArmed = true;
		}
	}
	else if (std::fabs(stickX) >= StickDeadzone)
	{
		deltaYaw = (int)std::lround(stickX * settings.SmoothTurnDegreesPerSecond * timeElapsed * DegreesToRotatorUnits);
	}

	if (deltaYaw == 0)
		return;

	// ViewRotation rather than Rotation: PlayerPawn.UpdateRotation() treats ViewRotation as the master
	// and copies it down onto the body, and it *adds* to it (that is how aTurn and the mouse get in),
	// so writing it here before the level ticks composes with the game's own turning instead of
	// fighting it. Masked to 16 bits the same way the scripts keep it.
	player->ViewRotation().Yaw = (player->ViewRotation().Yaw + deltaYaw) & 0xffff;
}

void VRPlayerInput::ReleaseStickAction()
{
	if (StickActionHeld == StickAction::None)
		return;

	// Released by key, not by command: Engine::InputEvent's IST_Release path walks activeInputButtons and
	// activeInputAxes looking for entries this key owns and clears exactly those. That is what undoes both
	// halves of the alias - the button/exec half *and* the aUp axis - without having to know what the
	// alias expanded into.
	engine->InputEvent(StickActionHeld == StickAction::Jump ? JumpSyntheticKey : CrouchSyntheticKey, IST_Release);
	StickActionHeld = StickAction::None;
}

void VRPlayerInput::UpdateJumpCrouch()
{
	// The same stick that turns: its X is the turn axis (UpdateTurning) and its Y was going spare.
	float stickY = engine->vr->GetController(1 - GetMovementHand()).Thumbstick.y;

	StickAction wanted = StickAction::None;
	if (std::fabs(stickY) >= JumpCrouchFireThreshold)
		wanted = stickY > 0.0f ? StickAction::Jump : StickAction::Crouch;

	// Only jump is gated on rearming, and that is the whole difference between the two: a jump is one
	// action per flick, so holding the stick up must not machine-gun it, whereas a crouch is a hold and
	// wants to stay down for exactly as long as the stick is. The stick has to come back near centre
	// before another jump can fire - which also means going straight from a held jump to a crouch and back
	// up again gives you the crouch but not a second jump, since the stick never passed through centre.
	if (std::fabs(stickY) <= JumpCrouchRearmThreshold)
		JumpArmed = true;

	if (wanted == StickActionHeld)
		return;

	ReleaseStickAction();

	if (wanted == StickAction::None)
		return;
	if (wanted == StickAction::Jump && !JumpArmed)
		return;

	// Drive the alias rather than the pawn. Both aliases are two commands - "Jump | Axis aUp Speed=+300.0"
	// and "Button bDuck | Axis aUp Speed=-300.0" - and setting only the exec/button half skips aUp, which
	// is the half that actually moves a Pawn up or down in water, on ladders and when flying. Which half
	// matters is game-specific, so drive the whole thing the way a key would.
	//
	// Delta 20 is what Engine::InputEvent gives a key press, and InputCommand multiplies it by the
	// binding's Speed=. Passing anything else here would make the VR stick a differently-sized press than
	// the spacebar.
	const char* aliasName = wanted == StickAction::Jump ? "Jump" : "Duck";
	auto alias = engine->inputAliases.find(aliasName);
	if (alias == engine->inputAliases.end())
		return; // A game whose ini defines no such alias. Nothing to drive, and nothing to release later.

	engine->InputCommand(alias->second, wanted == StickAction::Jump ? JumpSyntheticKey : CrouchSyntheticKey, 20);
	StickActionHeld = wanted;

	if (wanted == StickAction::Jump)
		JumpArmed = false;
}

void VRPlayerInput::UpdateRoomScale()
{
	// Left at zero when off, which is what tells the renderer to keep applying the head's full offset
	// as a view offset (the pre-room-scale behaviour) instead of subtracting anything.
	if (!LauncherSettings::Get().VR.RoomScaleMovement)
		return;

	const VRSubsystem::HeadPose& head = engine->vr->GetHead();
	if (!head.Valid)
		return;

	vec2 headXY(head.Position.x, head.Position.y);
	if (!HasHeadReference)
	{
		// First pose we ever see: adopt it. Treating the player's standing offset from the play space
		// anchor as one frame's worth of walking would fling the pawn across the room.
		AppliedHeadXY = headXY;
		HasHeadReference = true;
		return;
	}

	vec2 deltaLocal = headXY - AppliedHeadXY;

	// Advanced even if the move below is blocked or refused. The pawn then stops at the wall while the
	// player's real head carries on, so the view stops with the pawn - which is what should happen -
	// and stepping back reverses the delta and brings the pawn with it. Tracking the head rather than
	// the pawn is also what makes this survive a teleport, a respawn or a knockback without a reset:
	// the delta never refers to where the pawn actually is.
	AppliedHeadXY = headXY;

	if (dot(deltaLocal, deltaLocal) < 0.0001f)
		return;

	// A single frame's worth of walking is centimetres. Anything on this scale is not the player having
	// moved, it is the tracking having jumped - lost and reacquired, recentred by the runtime, or the
	// headset picked up off a desk. Re-anchoring (which AppliedHeadXY above has already done) and
	// dropping the move keeps the body under the head without hurling the pawn across the level.
	if (dot(deltaLocal, deltaLocal) > TrackingJumpThreshold * TrackingJumpThreshold)
		return;

	// Play space is anchored to the camera's rotation, so the head's local offset maps into the world
	// through the camera's axes. Yaw only: the camera's pitch must not turn a sideways step into a
	// vertical one.
	Coords yawCoords = Coords::Rotation(Rotator(0, engine->CameraRotation.Yaw, 0));
	vec3 deltaWorld = yawCoords.XAxis * deltaLocal.x + yawCoords.YAxis * deltaLocal.y;

	// Smooth so that walking at a wall slides along it rather than stopping dead, matching how the
	// pawn's own movement behaves.
	engine->viewport->Actor()->MoveSmooth(deltaWorld);
}
