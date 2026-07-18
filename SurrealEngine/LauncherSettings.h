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

		// Which wrist the gameplay HUD tablet rides (see RenderSubsystem::DrawVRHudPlane).
		VRHand HudHand = VRHand::Left;
		// Which hand aims the menu laser and toggles the pause menu with its B button (see
		// VRPlayerInput::UpdateButtons / RenderSubsystem::UpdateVRMenuLaser).
		VRHand MenuPointerHand = VRHand::Right;

		// Radius of the hand collider - and of the drawn hand ball - in Unreal units (see VRHands).
		int HandColliderRadius = 6;

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
