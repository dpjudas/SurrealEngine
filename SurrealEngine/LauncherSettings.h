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
