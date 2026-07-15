#pragma once

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

		// Walk the player pawn around so it stays underneath the headset when you physically move.
		// Off means the pawn stays put and physically walking only moves your viewpoint (which lets you
		// put your head through walls), so this defaults on.
		bool RoomScaleMovement = true;
	} VR;

private:
	LauncherSettings();
};
