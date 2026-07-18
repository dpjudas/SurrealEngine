#pragma once

#include "VRSubsystem.h"
#include <surrealgpu/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>

class OpenXRSubsystem : public VRSubsystem
{
public:
	explicit OpenXRSubsystem(int renderScalePercent);
	~OpenXRSubsystem() override;

	// Creates the XrInstance and finds a HMD system. Must succeed before any of the
	// other VRSubsystem calls are used. Returns false (with everything cleaned back up)
	// if no OpenXR runtime is installed/running or no HMD is currently connected.
	bool CreateInstance();

	bool IsActive() const override { return SessionActive; }

	Array<std::string> GetRequiredVulkanInstanceExtensions() override;
	Array<std::string> GetRequiredVulkanDeviceExtensions(VkPhysicalDevice physicalDevice) override;
	VkPhysicalDevice GetRequiredPhysicalDevice(VkInstance instance) override;

	bool InitSession(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex) override;

	void SyncInput() override;
	void Haptic(int handIndex, float amplitude, float duration) override;

	bool WaitFrame() override;
	void BeginFrame() override;
	bool LocateViews(EyeView outViews[EyeCount]) override;
	VkImage AcquireSwapchainImage(int eyeIndex) override;
	void ReleaseSwapchainImage(int eyeIndex) override;
	void EndFrame() override;

	int GetRecommendedEyeWidth() const override { return EyeWidth; }
	int GetRecommendedEyeHeight() const override { return EyeHeight; }
	uint32_t GetSwapchainFormat() const override { return (uint32_t)SwapchainFormat; }

private:
	void Destroy();
	void PollEvents();

	// Actions live on the XrInstance and their bindings must be suggested before any session is
	// attached, so this runs from CreateInstance(). The action *spaces* need a session, so those are
	// made later, by AttachActions().
	bool CreateActions();
	void SuggestBindings(const char* interactionProfile, const std::vector<std::pair<XrAction, std::string>>& bindings);
	bool AttachActions();
	bool ReadButton(XrAction action, int handIndex) const;
	// Locates one hand's action space against the play space anchor. Returns false when the pose isn't
	// tracked this frame, or before the first frame has been waited on (see SyncInput).
	bool LocatePose(XrSpace space, TrackedPose& outPose) const;

	struct Pose { vec3 Position, Forward, Right, Up; };
	static Pose ConvertPose(const XrPosef& pose);
	EyeView ConvertView(const XrView& view) const;
	static Array<std::string> SplitExtensionString(const std::string& s);

	XrInstance Instance = XR_NULL_HANDLE;
	XrSystemId SystemId = XR_NULL_SYSTEM_ID;
	XrSession Session = XR_NULL_HANDLE;
	XrSpace ViewSpace = XR_NULL_HANDLE; // LOCAL reference space - the seated/standing play space origin
	XrSessionState CurrentSessionState = XR_SESSION_STATE_UNKNOWN;
	bool SessionActive = false;
	// Latched when the runtime signals XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING. Never cleared: once the
	// instance is lost we stay off VR for the rest of the run rather than call into freed runtime state.
	bool InstanceLost = false;

	int RenderScalePercent = 100; // clamped in the constructor; see InitSession
	int EyeWidth = 0;
	int EyeHeight = 0;
	VkFormat SwapchainFormat = VK_FORMAT_UNDEFINED;

	struct EyeSwapchain
	{
		XrSwapchain Swapchain = XR_NULL_HANDLE;
		std::vector<XrSwapchainImageVulkanKHR> Images;
	};
	EyeSwapchain Eyes[EyeCount];

	XrActionSet ActionSet = XR_NULL_HANDLE;
	XrAction MoveAction = XR_NULL_HANDLE;      // thumbstick, Vector2f
	XrAction AimPoseAction = XR_NULL_HANDLE;   // where the controller points
	XrAction GripPoseAction = XR_NULL_HANDLE;  // where the hand holds; see ControllerState::Grip
	XrAction HapticAction = XR_NULL_HANDLE;    // XR_ACTION_TYPE_VIBRATION_OUTPUT
	XrAction ButtonActions[ButtonCount] = {};
	// Every action is declared with both hands as subaction paths, so one action covers both controllers
	// and the hand is chosen at read time rather than by having two of everything.
	XrPath HandPaths[HandCount] = {};
	XrSpace AimSpaces[HandCount] = {};
	XrSpace GripSpaces[HandCount] = {};
	bool ActionsAttached = false;

	XrFrameState FrameState = { XR_TYPE_FRAME_STATE };
	XrView Views[EyeCount] = { { XR_TYPE_VIEW }, { XR_TYPE_VIEW } };

	// Reset by BeginFrame. A projection layer may only be submitted to xrEndFrame if the poses it is
	// built from are this frame's and every swapchain it names had an image released, so EndFrame() uses
	// these to decide whether it has a complete frame to show or has to submit an empty one.
	bool ViewsLocated = false;
	int EyesReleased = 0;

	PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR_ = nullptr;
	PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR_ = nullptr;
	PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR_ = nullptr;
	PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR_ = nullptr;
};
