#pragma once

#include "Utils/Array.h"
#include "Math/vec.h"
#include <memory>
#include <string>
#include <cstdint>

// Forward declarations matching the real Vulkan handle typedefs exactly, so this header
// stays includable from translation units that never pull in the Vulkan headers
// (D3D11 render device, Engine.h, etc). A typedef name isn't a preprocessor token, so
// this can't be guarded with #ifndef - but repeating an identical typedef (as either
// SurrealWidgets' window.h or the real vulkan.h will if also included in the same
// translation unit) is legal in C++, so declaring these unconditionally is safe.
typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkImage_T* VkImage;

// 1 Unreal unit = 1.905 cm, a long-standing UE1/UT community reverse-engineered constant. OpenXR works
// in metres, so everything crossing that boundary - head poses, and anything placed relative to the
// player in real-world units, like the VR menu plane - has to go through this.
static constexpr float MetersToUnrealUnits = 52.4934f;

// Optional VR support (head tracking + stereo rendering). Off by default at runtime.
// See VRSubsystem::Create. All virtuals default to inert/no-op behavior so callers
// never need to branch on whether VR was compiled in - only on IsActive().
class VRSubsystem
{
public:
	// Returns a NullVRSubsystem when !enabled, when not built with USE_OPENXR, or when
	// no OpenXR runtime could be reached - VR failing to initialize must never prevent
	// the desktop game from starting.
	//
	// renderScalePercent scales the runtime's recommended per-eye resolution; it is clamped to something
	// sane, so an unset or nonsense value can't produce an unusable swapchain. See OpenXRSubsystem::InitSession.
	static std::unique_ptr<VRSubsystem> Create(bool enabled, int renderScalePercent);

	virtual ~VRSubsystem() = default;

	virtual bool IsActive() const = 0;

	// Called while building the Vulkan instance/device, before either exists.
	virtual Array<std::string> GetRequiredVulkanInstanceExtensions() { return {}; }
	virtual Array<std::string> GetRequiredVulkanDeviceExtensions(VkPhysicalDevice physicalDevice) { return {}; }
	// Returns the physical device the active VR runtime requires rendering to happen on, or null if there's no preference.
	virtual VkPhysicalDevice GetRequiredPhysicalDevice(VkInstance instance) { return nullptr; }

	// Called once after the VkInstance/VkPhysicalDevice/VkDevice are all known.
	// Returns false (and leaves IsActive() false) if no headset/runtime could be reached.
	virtual bool InitSession(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex) { return false; }

	// Head/eye pose and asymmetric field of view for one eye, already converted into
	// SurrealEngine's world axes (X forward, Y right, Z up) and units, relative to the
	// play space anchor (i.e. relative to where the game camera would otherwise be).
	struct EyeView
	{
		vec3 Position = vec3(0.0f);
		vec3 Forward = vec3(1.0f, 0.0f, 0.0f);
		vec3 Right = vec3(0.0f, 1.0f, 0.0f);
		vec3 Up = vec3(0.0f, 0.0f, 1.0f);

		// Tangent-space field of view angles (radians) of the eye's asymmetric frustum.
		float AngleLeft = -0.75f;
		float AngleRight = 0.75f;
		float AngleUp = 0.75f;
		float AngleDown = -0.75f;
	};

	static const int EyeCount = 2;

	// Pose of the head itself (midway between the eyes) rather than of one eye, in the same space and
	// units as EyeView. Refreshed by LocateViews. Movement direction and room-scale walking are both
	// about where the player's head is, not where either eyeball is, so they want this and not EyeView.
	struct HeadPose
	{
		bool Valid = false;
		vec3 Position = vec3(0.0f);
		vec3 Forward = vec3(1.0f, 0.0f, 0.0f);
		vec3 Right = vec3(0.0f, 1.0f, 0.0f);
		vec3 Up = vec3(0.0f, 0.0f, 1.0f);
	};

	// Motion controller buttons, in the order they map onto EInputKey's IK_Joy1..IK_Joy16 (see
	// VRPlayerInput).
	//
	// Appending here shifts every right-hand key, because the mapping is IK_Joy1 + hand * ButtonCount +
	// button. That used to mean an append silently rebound the player's right hand; it no longer does,
	// because VR now claims the whole Joy1..Joy16 range and regenerates its bindings from this enum at
	// startup (VRPlayerInput::ApplyKeybindings), so the two can't drift apart. What an append *can* still
	// do is overrun IK_Joy16 - HandCount * ButtonCount must stay <= 16, which VRPlayerInput.cpp
	// static_asserts.
	enum Button
	{
		Button_Trigger,
		Button_Grip,
		Button_A,
		Button_B,
		Button_ThumbstickClick,
		Button_Menu,
		// The Index's pill trackpad, as a button. There is no trackpad/click on that profile, so this
		// comes from thresholding trackpad/force - see OpenXRSubsystem's binding table.
		Button_Trackpad,
		ButtonCount
	};

	static const int HandCount = 2;
	enum { HandLeft = 0, HandRight = 1 };

	// Which hand's laser drives the pause/main menu: VRPlayerInput routes that hand's trigger to a mouse
	// click while a menu is up, and RenderSubsystem casts the pointing laser and moves the cursor from the
	// same hand. Kept here so the input and render sides can never disagree about which hand points. The
	// menu is toggled with that hand's B button (see VRPlayerInput::UpdateButtons).
	static const int MenuPointerHand = HandRight;

	// One tracked pose, in the same space and units as EyeView. Deliberately not called Pose:
	// OpenXRSubsystem already has its own nested Pose (its raw XrPosef conversion), and a base-class Pose
	// would be silently shadowed by it inside every OpenXRSubsystem member.
	struct TrackedPose
	{
		// False when the controller is off, asleep, or otherwise not being tracked. Position/orientation
		// are stale rather than meaningful when this is false.
		bool Valid = false;
		vec3 Position = vec3(0.0f);
		vec3 Forward = vec3(1.0f, 0.0f, 0.0f);
		vec3 Right = vec3(0.0f, 1.0f, 0.0f);
		vec3 Up = vec3(0.0f, 0.0f, 1.0f);
	};

	struct ControllerState
	{
		// False when the controller is off, asleep, or otherwise not being tracked. Position/orientation
		// are stale rather than meaningful when this is false.
		bool PoseValid = false;
		vec3 Position = vec3(0.0f);
		vec3 Forward = vec3(1.0f, 0.0f, 0.0f);
		vec3 Right = vec3(0.0f, 1.0f, 0.0f);
		vec3 Up = vec3(0.0f, 0.0f, 1.0f);

		// Where the hand is *gripping*, as opposed to the fields above, which are the aim pose - the ray
		// the controller points along. The two diverge a lot on the Index, where the aim ray leaves the
		// front of the controller while the grip sits in the fist. Anything held in the hand (the weapon,
		// phase 4) wants this; anything pointed (the menu laser) wants the aim pose.
		TrackedPose Grip;

		// x = right, y = forward, each in [-1, 1]. Already deadzoned by the runtime, but not by us.
		vec2 Thumbstick = vec2(0.0f);
		bool Buttons[ButtonCount] = {};
	};

	// Refreshes GetController() from the runtime. Unlike the render sequence below this is independent of
	// the frame loop, so it's driven from the engine's input tick instead - see VRPlayerInput::Tick.
	virtual void SyncInput() {}

	// Buzz one controller. duration is in seconds; amplitude is [0, 1] and gets clamped. A zero or
	// negative duration asks the runtime for its own minimum pulse, which is what a single "tick" (a
	// wheel entry crossed, a trigger pulled) wants. No-op when VR is off or the runtime has no haptics,
	// so callers never need to check.
	virtual void Haptic(int handIndex, float amplitude, float duration = 0.0f) {}

	// Read back whatever the last SyncInput()/LocateViews() saw. Cheap; call as often as convenient.
	// Both return inert defaults when VR is off, so callers don't have to branch on IsActive().
	const ControllerState& GetController(int handIndex) const { return Controllers[handIndex]; }
	const HeadPose& GetHead() const { return Head; }

	// Per-frame sequence: WaitFrame -> BeginFrame -> LocateViews -> for each eye:
	// AcquireSwapchainImage, render, ReleaseSwapchainImage -> EndFrame.
	//
	// Every BeginFrame must be matched by an EndFrame, including when LocateViews or an eye's
	// AcquireSwapchainImage failed part way through - EndFrame submits the eyes that were actually
	// rendered and released this frame, or no image at all if they weren't all there.
	virtual bool WaitFrame() { return false; }
	virtual void BeginFrame() {}
	virtual bool LocateViews(EyeView outViews[EyeCount]) { return false; }
	virtual VkImage AcquireSwapchainImage(int eyeIndex) { return nullptr; }
	virtual void ReleaseSwapchainImage(int eyeIndex) {}
	virtual void EndFrame() {}

	virtual int GetRecommendedEyeWidth() const { return 0; }
	virtual int GetRecommendedEyeHeight() const { return 0; }
	virtual uint32_t GetSwapchainFormat() const { return 0; } // VkFormat, as uint32_t to avoid needing vulkan.h here

protected:
	// Written by SyncInput()/LocateViews() in the derived class, read through the getters above. They
	// live here so NullVRSubsystem answers with inert defaults for free.
	ControllerState Controllers[HandCount];
	HeadPose Head;
};

class NullVRSubsystem : public VRSubsystem
{
public:
	bool IsActive() const override { return false; }
};
