#pragma once

#include <string>
#include "VR/VRSubsystem.h" // VRSubsystem::HandCount / ButtonCount / Button order, for the control bindings below

enum class RenderDeviceType
{
	Vulkan,
	D3D11,
	D3D12
};

enum class AntialiasMode
{
	Off,
	MSAA2x,
	MSAA4x
};

enum class LightMode
{
	Normal,
	OneX,
	BrighterActors
};

enum class GammaMode
{
	D3D9,
	XOpenGL
};

// Which real-world direction the movement thumbstick is measured against.
enum class VRMovementReference
{
	Controller, // the direction the movement hand's controller is pointing
	Head        // the direction the headset is facing
};

enum class VRHand
{
	Left,
	Right
};

enum class VRTurnMode
{
	Snap,   // flicking the turn stick rotates instantly by SnapTurnDegrees
	Smooth, // holding the turn stick rotates continuously at SmoothTurnDegreesPerSecond
	Off     // turn with the mouse (or your neck) only
};

class LauncherSettings
{
public:
	static LauncherSettings& Get();
	void Save();

	struct
	{
		RenderDeviceType Type = RenderDeviceType::Vulkan;
		bool UseVSync = true;
		AntialiasMode Antialias = AntialiasMode::MSAA4x;
		LightMode Light = LightMode::Normal;
		GammaMode Gamma = GammaMode::D3D9;
		bool GammaCorrectScreenshots = false;
		bool Hdr = false;
		int HdrScale = 128;
		bool Bloom = false;
		int BloomAmount = 128;
		bool UseDebugLayer = false;
	} RenderDevice;

	struct
	{
		Array<std::string> SearchList;
		int LastSelected = -1;
	} Games;

	struct
	{
		bool Enabled = false;
		// Percentage of the OpenXR runtime's recommended per-eye resolution to actually render at.
		// Defaults well below 100 deliberately - see OpenXRSubsystem::InitSession.
		int RenderScale = 60;

		// Thumbstick movement. The movement stick is on MovementHand; the turn stick is on the other one.
		VRMovementReference MovementReference = VRMovementReference::Controller;
		VRHand MovementHand = VRHand::Left;
		// Which controller points the way, when MovementReference is Controller. Kept separate from
		// MovementHand so the stick you push and the hand you aim with need not be the same one.
		VRHand MovementDirectionHand = VRHand::Left;

		VRTurnMode TurnMode = VRTurnMode::Snap;
		int SnapTurnDegrees = 45;
		int SmoothTurnDegreesPerSecond = 90;

		// Thumbstick centre deadzone, [0, 1). Below this the stick reads as centred. Not every runtime
		// applies its own, and a stick resting at 0.02 would otherwise creep the player into a wall.
		float StickDeadzone = 0.15f;

		// Walk the player pawn around so it stays underneath the headset when you physically move.
		// Off means the pawn stays put and physically walking only moves your viewpoint (which lets you
		// put your head through walls), so this defaults on.
		bool RoomScaleMovement = true;

		// Which hand holds the weapon - its grip pose positions and orients the first-person weapon mesh
		// (see RenderSubsystem::DrawActor). Defaults opposite the HUD tablet hand so the tablet rides the
		// empty wrist.
		VRHand WeaponHand = VRHand::Right;

		// Fine placement of the weapon mesh relative to the weapon hand's grip pose, for in-headset tuning
		// (the grip pose sits in the fist, but each weapon mesh has its own origin and forward axis, so the
		// raw grip transform lands the gun somewhere plausible but rarely perfect). Position offsets are in
		// centimetres along the grip's own axes (forward down the barrel, right, up); rotation offsets are in
		// degrees applied in the grip's local frame (pitch tilts the muzzle, yaw swings it, roll banks it).
		// All default 0 == weapon rigidly on the raw grip pose. See RenderSubsystem::DrawActor.
		int WeaponForwardOffsetCm = 0;
		int WeaponRightOffsetCm = 0;
		int WeaponUpOffsetCm = 0;
		int WeaponPitchOffsetDegrees = 0;
		int WeaponYawOffsetDegrees = 0;
		int WeaponRollOffsetDegrees = 0;

		// Scale applied to the first-person weapon mesh while it is held in the VR hand, as a percentage of
		// the mesh's own DrawScale (100 == unchanged). First-person view meshes are modelled tiny and only
		// look right because the desktop parks them against the camera; at arm's length in VR they need
		// scaling up. See RenderSubsystem::DrawActor.
		int WeaponScalePercent = 500;

		// Weapon/item wheel (hold A on a hand to fan its inventory out in a circle centred on that
		// controller; see VR/VRWheel.h). Radius and deadzone are in centimetres (real-world hand travel).
		// EntryScalePercent scales a wheel weapon's PickupViewMesh - unlike WeaponScalePercent, this is
		// NOT the held-weapon multiplier: the pickup mesh apparently models the weapon much larger than
		// its real-world size (in-headset testing found 60% still ~10x too big), so this trims it down
		// hard rather than boosting it the way the tiny first-person view mesh wants. The wheel-open
		// button itself is hardcoded to A, not a ButtonCommands slot - see VRWheel.
		int WheelRadiusCm = 20;
		int WheelSelectDeadzoneCm = 4;
		int WheelEntryScalePercent = 6;
		// Item-wheel PickupViewMesh entries scale off their own knob, not WheelEntryScalePercent above -
		// the two shared one setting at first, and a non-weapon pickup's mesh doesn't necessarily model at
		// the same raw scale a weapon's does (in-headset finding: 6% read right for weapons but still too
		// small for items). 15% (2.5x that shared value) is the corrected default; see DrawWheelEntryMesh's
		// caller in DrawVRWheel, and DrawVRActiveItem's PickupViewMesh fallback, which reuses it too.
		int WheelItemScalePercent = 15;
		// Item-wheel icon billboards (only entries with no PickupViewMesh at all fall back to these) scale
		// off yet another knob - a fixed-cm-reference billboard and a DrawScale-multiplied mesh have
		// unrelated size logic and can't share one setting either. See DrawWheelItemIcon.
		int WheelIconScalePercent = 36;

		// Active item (the off hand's SelectedItem, set via the item wheel) rides the off hand the way the
		// weapon rides the weapon hand - same offset/scale knobs, its own copy since the item and weapon
		// meshes are unrelated sizes. See RenderCanvas.cpp's off-hand DrawActor override.
		int ItemScalePercent = 500;
		int ItemForwardOffsetCm = 0;
		int ItemRightOffsetCm = 0;
		int ItemUpOffsetCm = 0;
		int ItemPitchOffsetDegrees = 0;
		int ItemYawOffsetDegrees = 0;
		int ItemRollOffsetDegrees = 0;

		// Which wrist the gameplay HUD tablet rides (see RenderSubsystem::DrawVRHudPlane).
		VRHand HudHand = VRHand::Left;
		// Which hand aims the menu laser and toggles the pause menu with its B button (see
		// VRPlayerInput::UpdateButtons / RenderSubsystem::UpdateVRMenuLaser).
		VRHand MenuPointerHand = VRHand::Right;

		// Radius of the hand collider - and of the drawn hand ball - in Unreal units (see VRHands).
		int HandColliderRadius = 6;

		// Which hand(s) can collect a pickup by touching it (see VRHands::Classify/UpdateContacts).
		// 0 = off, 1 = off-hand only (not WeaponHand), 2 = both. Both is the default: it's what every VR
		// game trains the player to expect, and the failure mode it invites (bumping a pickup with the gun
		// hand mid-fight) is exactly what walking over a pickup already does in the desktop game.
		int PickupHands = 2;

		// Draw a crosshair where the weapon's shot will land (see RenderSubsystem::DrawVRCrosshair).
		// Independent of AimLaser: either, both or neither. Off by default - hand-aimed shooting is meant
		// to be read off the gun, and this exists for the players and the weapons where that isn't enough.
		bool Crosshair = false;
		// Scales the crosshair's apparent size (RenderSubsystem::DrawVRCrosshair's CrosshairApparentSizeK),
		// confirmed too large in-headset at 100% of the original constant - the base constant itself was
		// halved, so 100% is that already-halved size, not the original.
		int CrosshairSizePercent = 100;

		// Draw a transparent ray from the muzzle to the impact point (RenderSubsystem::DrawVRAimLaser).
		bool AimLaser = false;

		// HUD wrist tablet geometry, in centimetres. Width sets the panel size (its height follows the
		// canvas aspect); ForearmOffset slides it back up the forearm toward the player, WristOffset floats
		// it off the back of the wrist. See RenderSubsystem::DrawVRHudPlane.
		int HudTabletWidthCm = 18;
		int HudTabletForearmOffsetCm = 18;
		int HudTabletWristOffsetCm = 4;

		// What each controller button is bound to, indexed [hand][button] in VRSubsystem::Button order.
		// Empty means unbound. Any console/exec command or ini alias works, exactly like a Joy binding -
		// VR claims the Joy keys and writes these at startup (see VRPlayerInput::ApplyKeybindings). Kept
		// here rather than in the game ini because Epic's stock Joy defaults are gamepad-shaped and wrong
		// for hands; see the notes in VRPlayerInput.cpp.
		std::string ButtonCommands[VRSubsystem::HandCount][VRSubsystem::ButtonCount] =
		{
			// Left hand:  Trigger,  Grip, A,  B,  ThumbstickClick, Menu, Trackpad
			{ "Fire", "", "", "", "NextWeapon", "", "AltFire" },
			// Right hand: Trigger,  Grip, A,  B,  ThumbstickClick, Menu, Trackpad
			{ "Fire", "", "", "", "NextWeapon", "", "AltFire" },
		};
	} VR;

private:
	LauncherSettings();
};
