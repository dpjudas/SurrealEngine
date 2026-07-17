
#include "Precomp.h"
#include "OpenXRSubsystem.h"
#include "Math/quaternion.h"
#include "Utils/Logger.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstring>

namespace
{
	bool XR_SUCCEEDED_LOG(XrResult result, const char* what)
	{
		if (XR_SUCCEEDED(result))
			return true;
		LogMessage(std::string("OpenXR: ") + what + " failed");
		return false;
	}
}

OpenXRSubsystem::OpenXRSubsystem(int renderScalePercent)
{
	// Clamped rather than trusted: this ends up as a swapchain size, so 0 or a wild value would be an
	// unrecoverable failure much further down.
	RenderScalePercent = std::max(std::min(renderScalePercent, 200), 10);
}

OpenXRSubsystem::~OpenXRSubsystem()
{
	Destroy();
}

void OpenXRSubsystem::Destroy()
{
	for (EyeSwapchain& eye : Eyes)
	{
		if (eye.Swapchain != XR_NULL_HANDLE)
		{
			xrDestroySwapchain(eye.Swapchain);
			eye.Swapchain = XR_NULL_HANDLE;
		}
		eye.Images.clear();
	}

	for (XrSpace* spaces : { AimSpaces, GripSpaces })
	{
		for (int hand = 0; hand < HandCount; hand++)
		{
			if (spaces[hand] != XR_NULL_HANDLE)
			{
				xrDestroySpace(spaces[hand]);
				spaces[hand] = XR_NULL_HANDLE;
			}
		}
	}
	ActionsAttached = false;

	if (ViewSpace != XR_NULL_HANDLE)
	{
		xrDestroySpace(ViewSpace);
		ViewSpace = XR_NULL_HANDLE;
	}

	// Takes every action created on it down with it, so the individual XrActions need no destroy of
	// their own - but they do become dangling, hence clearing them here.
	if (ActionSet != XR_NULL_HANDLE)
	{
		xrDestroyActionSet(ActionSet);
		ActionSet = XR_NULL_HANDLE;
		MoveAction = XR_NULL_HANDLE;
		AimPoseAction = XR_NULL_HANDLE;
		for (XrAction& action : ButtonActions)
			action = XR_NULL_HANDLE;
	}

	if (Session != XR_NULL_HANDLE)
	{
		xrDestroySession(Session);
		Session = XR_NULL_HANDLE;
	}

	if (Instance != XR_NULL_HANDLE)
	{
		xrDestroyInstance(Instance);
		Instance = XR_NULL_HANDLE;
	}

	SessionActive = false;
}

Array<std::string> OpenXRSubsystem::SplitExtensionString(const std::string& s)
{
	Array<std::string> result;
	size_t start = 0;
	while (start < s.size())
	{
		size_t end = s.find(' ', start);
		if (end == std::string::npos)
			end = s.size();
		if (end > start)
			result.push_back(s.substr(start, end - start));
		start = end + 1;
	}
	return result;
}

bool OpenXRSubsystem::CreateInstance()
{
	const char* extensions[] = { "XR_KHR_vulkan_enable" };

	XrInstanceCreateInfo createInfo = { XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = 1;
	createInfo.enabledExtensionNames = extensions;
	std::strncpy(createInfo.applicationInfo.applicationName, "SurrealEngine", XR_MAX_APPLICATION_NAME_SIZE - 1);
	std::strncpy(createInfo.applicationInfo.engineName, "SurrealEngine", XR_MAX_ENGINE_NAME_SIZE - 1);
	// XR_CURRENT_API_VERSION from the SDK headers is 1.1, but SteamVR's OpenXR runtime only
	// implements 1.0 and rejects anything higher with XR_ERROR_API_VERSION_UNSUPPORTED.
	createInfo.applicationInfo.apiVersion = XR_API_VERSION_1_0;

	if (!XR_SUCCEEDED_LOG(xrCreateInstance(&createInfo, &Instance), "xrCreateInstance"))
	{
		Instance = XR_NULL_HANDLE;
		return false;
	}

	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	if (!XR_SUCCEEDED_LOG(xrGetSystem(Instance, &systemInfo, &SystemId), "xrGetSystem"))
	{
		Destroy();
		return false;
	}

	// Not fatal: without actions the controllers go dead, but head tracking and stereo rendering still
	// work, which is strictly better than refusing to start VR at all.
	if (!CreateActions())
		LogMessage("OpenXR: motion controller input unavailable");

	xrGetInstanceProcAddr(Instance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanInstanceExtensionsKHR_);
	xrGetInstanceProcAddr(Instance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&xrGetVulkanDeviceExtensionsKHR_);
	xrGetInstanceProcAddr(Instance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsDeviceKHR_);
	xrGetInstanceProcAddr(Instance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetVulkanGraphicsRequirementsKHR_);

	if (!xrGetVulkanInstanceExtensionsKHR_ || !xrGetVulkanDeviceExtensionsKHR_ || !xrGetVulkanGraphicsDeviceKHR_ || !xrGetVulkanGraphicsRequirementsKHR_)
	{
		Destroy();
		return false;
	}

	// The spec requires this to be called before the Vulkan device is created, even though we don't act on the result.
	XrGraphicsRequirementsVulkanKHR requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	xrGetVulkanGraphicsRequirementsKHR_(Instance, SystemId, &requirements);

	return true;
}

bool OpenXRSubsystem::CreateActions()
{
	if (!XR_SUCCEEDED_LOG(xrStringToPath(Instance, "/user/hand/left", &HandPaths[HandLeft]), "xrStringToPath") ||
		!XR_SUCCEEDED_LOG(xrStringToPath(Instance, "/user/hand/right", &HandPaths[HandRight]), "xrStringToPath"))
		return false;

	XrActionSetCreateInfo setInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
	std::strncpy(setInfo.actionSetName, "gameplay", XR_MAX_ACTION_SET_NAME_SIZE - 1);
	std::strncpy(setInfo.localizedActionSetName, "Gameplay", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE - 1);
	if (!XR_SUCCEEDED_LOG(xrCreateActionSet(Instance, &setInfo, &ActionSet), "xrCreateActionSet"))
		return false;

	auto createAction = [this](XrAction& action, XrActionType type, const char* name, const char* localized)
	{
		XrActionCreateInfo info = { XR_TYPE_ACTION_CREATE_INFO };
		info.actionType = type;
		info.countSubactionPaths = HandCount;
		info.subactionPaths = HandPaths;
		std::strncpy(info.actionName, name, XR_MAX_ACTION_NAME_SIZE - 1);
		std::strncpy(info.localizedActionName, localized, XR_MAX_LOCALIZED_ACTION_NAME_SIZE - 1);
		return XR_SUCCEEDED_LOG(xrCreateAction(ActionSet, &info, &action), "xrCreateAction");
	};

	if (!createAction(MoveAction, XR_ACTION_TYPE_VECTOR2F_INPUT, "thumbstick", "Thumbstick") ||
		!createAction(AimPoseAction, XR_ACTION_TYPE_POSE_INPUT, "aim_pose", "Aim pose") ||
		!createAction(GripPoseAction, XR_ACTION_TYPE_POSE_INPUT, "grip_pose", "Grip pose") ||
		!createAction(HapticAction, XR_ACTION_TYPE_VIBRATION_OUTPUT, "haptic", "Haptic feedback") ||
		!createAction(ButtonActions[Button_Trigger], XR_ACTION_TYPE_BOOLEAN_INPUT, "trigger", "Trigger") ||
		!createAction(ButtonActions[Button_Grip], XR_ACTION_TYPE_BOOLEAN_INPUT, "grip", "Grip") ||
		!createAction(ButtonActions[Button_A], XR_ACTION_TYPE_BOOLEAN_INPUT, "button_a", "A button") ||
		!createAction(ButtonActions[Button_B], XR_ACTION_TYPE_BOOLEAN_INPUT, "button_b", "B button") ||
		!createAction(ButtonActions[Button_ThumbstickClick], XR_ACTION_TYPE_BOOLEAN_INPUT, "thumbstick_click", "Thumbstick click") ||
		!createAction(ButtonActions[Button_Menu], XR_ACTION_TYPE_BOOLEAN_INPUT, "menu", "Menu button") ||
		!createAction(ButtonActions[Button_Trackpad], XR_ACTION_TYPE_BOOLEAN_INPUT, "trackpad", "Trackpad press"))
		return false;

	// Bindings are listed per profile rather than generated, because the profiles genuinely differ: the
	// Index has no menu button, and Touch's face buttons are X/Y on the left hand but A/B on the right.
	// A path that a profile doesn't define makes the runtime reject that profile's *whole* suggestion,
	// so these have to be exact. Grip and trigger bind to float inputs where a profile has no click
	// variant; OpenXR thresholds float sources for us when the action is boolean.

	SuggestBindings("/interaction_profiles/valve/index_controller", {
		{ MoveAction, "/user/hand/left/input/thumbstick" },
		{ MoveAction, "/user/hand/right/input/thumbstick" },
		{ AimPoseAction, "/user/hand/left/input/aim/pose" },
		{ AimPoseAction, "/user/hand/right/input/aim/pose" },
		{ GripPoseAction, "/user/hand/left/input/grip/pose" },
		{ GripPoseAction, "/user/hand/right/input/grip/pose" },
		{ HapticAction, "/user/hand/left/output/haptic" },
		{ HapticAction, "/user/hand/right/output/haptic" },
		{ ButtonActions[Button_Trigger], "/user/hand/left/input/trigger/click" },
		{ ButtonActions[Button_Trigger], "/user/hand/right/input/trigger/click" },
		{ ButtonActions[Button_Grip], "/user/hand/left/input/squeeze/value" },
		{ ButtonActions[Button_Grip], "/user/hand/right/input/squeeze/value" },
		{ ButtonActions[Button_A], "/user/hand/left/input/a/click" },
		{ ButtonActions[Button_A], "/user/hand/right/input/a/click" },
		{ ButtonActions[Button_B], "/user/hand/left/input/b/click" },
		{ ButtonActions[Button_B], "/user/hand/right/input/b/click" },
		{ ButtonActions[Button_ThumbstickClick], "/user/hand/left/input/thumbstick/click" },
		{ ButtonActions[Button_ThumbstickClick], "/user/hand/right/input/thumbstick/click" },
		// The pill has no trackpad/click on this profile - force is the closest thing to a press, and
		// OpenXR thresholds the float for us because the action is boolean. trackpad/touch would fire
		// on a resting thumb, which for an alt-fire is not what anyone wants.
		{ ButtonActions[Button_Trackpad], "/user/hand/left/input/trackpad/force" },
		{ ButtonActions[Button_Trackpad], "/user/hand/right/input/trackpad/force" },
		});

	SuggestBindings("/interaction_profiles/oculus/touch_controller", {
		{ MoveAction, "/user/hand/left/input/thumbstick" },
		{ MoveAction, "/user/hand/right/input/thumbstick" },
		{ AimPoseAction, "/user/hand/left/input/aim/pose" },
		{ AimPoseAction, "/user/hand/right/input/aim/pose" },
		{ GripPoseAction, "/user/hand/left/input/grip/pose" },
		{ GripPoseAction, "/user/hand/right/input/grip/pose" },
		{ HapticAction, "/user/hand/left/output/haptic" },
		{ HapticAction, "/user/hand/right/output/haptic" },
		// No trackpad on Touch, so Button_Trackpad simply stays unbound here and reads as never pressed.
		{ ButtonActions[Button_Trigger], "/user/hand/left/input/trigger/value" },
		{ ButtonActions[Button_Trigger], "/user/hand/right/input/trigger/value" },
		{ ButtonActions[Button_Grip], "/user/hand/left/input/squeeze/value" },
		{ ButtonActions[Button_Grip], "/user/hand/right/input/squeeze/value" },
		{ ButtonActions[Button_A], "/user/hand/left/input/x/click" },
		{ ButtonActions[Button_A], "/user/hand/right/input/a/click" },
		{ ButtonActions[Button_B], "/user/hand/left/input/y/click" },
		{ ButtonActions[Button_B], "/user/hand/right/input/b/click" },
		{ ButtonActions[Button_ThumbstickClick], "/user/hand/left/input/thumbstick/click" },
		{ ButtonActions[Button_ThumbstickClick], "/user/hand/right/input/thumbstick/click" },
		{ ButtonActions[Button_Menu], "/user/hand/left/input/menu/click" },
		});

	// The fallback every runtime must support. No thumbstick, so no stick movement - but it keeps aim
	// poses and a fire button alive on controllers we have no specific profile for.
	SuggestBindings("/interaction_profiles/khr/simple_controller", {
		{ AimPoseAction, "/user/hand/left/input/aim/pose" },
		{ AimPoseAction, "/user/hand/right/input/aim/pose" },
		{ GripPoseAction, "/user/hand/left/input/grip/pose" },
		{ GripPoseAction, "/user/hand/right/input/grip/pose" },
		{ HapticAction, "/user/hand/left/output/haptic" },
		{ HapticAction, "/user/hand/right/output/haptic" },
		{ ButtonActions[Button_Trigger], "/user/hand/left/input/select/click" },
		{ ButtonActions[Button_Trigger], "/user/hand/right/input/select/click" },
		{ ButtonActions[Button_Menu], "/user/hand/left/input/menu/click" },
		{ ButtonActions[Button_Menu], "/user/hand/right/input/menu/click" },
		});

	return true;
}

void OpenXRSubsystem::SuggestBindings(const char* interactionProfile, const std::vector<std::pair<XrAction, std::string>>& bindings)
{
	std::vector<XrActionSuggestedBinding> suggested;
	suggested.reserve(bindings.size());
	for (const auto& binding : bindings)
	{
		XrPath path = XR_NULL_PATH;
		if (!XR_SUCCEEDED(xrStringToPath(Instance, binding.second.c_str(), &path)))
		{
			LogMessage("OpenXR: not a valid input path: " + binding.second);
			return;
		}
		suggested.push_back({ binding.first, path });
	}

	XrPath profilePath = XR_NULL_PATH;
	if (!XR_SUCCEEDED(xrStringToPath(Instance, interactionProfile, &profilePath)))
		return;

	XrInteractionProfileSuggestedBinding info = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	info.interactionProfile = profilePath;
	info.countSuggestedBindings = (uint32_t)suggested.size();
	info.suggestedBindings = suggested.data();

	// Only worth a log line, never a failure: a runtime is entitled to reject a profile it doesn't
	// implement, and the profiles it does implement still bind. This is the normal path on a headset
	// whose controllers aren't one of the ones listed above.
	if (XR_FAILED(xrSuggestInteractionProfileBindings(Instance, &info)))
		LogMessage(std::string("OpenXR: runtime rejected the bindings for ") + interactionProfile);
}

bool OpenXRSubsystem::AttachActions()
{
	if (ActionSet == XR_NULL_HANDLE)
		return false;

	for (int hand = 0; hand < HandCount; hand++)
	{
		XrActionSpaceCreateInfo spaceInfo = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		spaceInfo.subactionPath = HandPaths[hand];
		spaceInfo.poseInActionSpace = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };

		spaceInfo.action = AimPoseAction;
		if (!XR_SUCCEEDED_LOG(xrCreateActionSpace(Session, &spaceInfo, &AimSpaces[hand]), "xrCreateActionSpace"))
			return false;

		spaceInfo.action = GripPoseAction;
		if (!XR_SUCCEEDED_LOG(xrCreateActionSpace(Session, &spaceInfo, &GripSpaces[hand]), "xrCreateActionSpace"))
			return false;
	}

	// One shot per session: the spec forbids attaching twice, and forbids xrSyncActions before this.
	XrSessionActionSetsAttachInfo attachInfo = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attachInfo.countActionSets = 1;
	attachInfo.actionSets = &ActionSet;
	if (!XR_SUCCEEDED_LOG(xrAttachSessionActionSets(Session, &attachInfo), "xrAttachSessionActionSets"))
		return false;

	ActionsAttached = true;
	return true;
}

bool OpenXRSubsystem::ReadButton(XrAction action, int handIndex) const
{
	if (action == XR_NULL_HANDLE)
		return false;

	XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = action;
	getInfo.subactionPath = HandPaths[handIndex];

	XrActionStateBoolean value = { XR_TYPE_ACTION_STATE_BOOLEAN };
	if (XR_FAILED(xrGetActionStateBoolean(Session, &getInfo, &value)) || !value.isActive)
		return false;
	return value.currentState == XR_TRUE;
}

void OpenXRSubsystem::SyncInput()
{
	// Cleared up front so that every early return below - no session, no focus, controller asleep -
	// reads as "nothing pressed, stick centred" instead of leaving the player running forwards on a
	// stale stick value.
	for (ControllerState& controller : Controllers)
		controller = ControllerState();

	if (!SessionActive || !ActionsAttached)
		return;

	XrActiveActionSet activeActionSet = { ActionSet, XR_NULL_PATH };
	XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
	syncInfo.countActiveActionSets = 1;
	syncInfo.activeActionSets = &activeActionSet;
	// XR_SESSION_NOT_FOCUSED is a success code and an entirely normal one - it is what we get while the
	// runtime's own dashboard holds input focus. Every action then reports itself inactive, which the
	// clear above has already turned into neutral input, so it needs no case of its own here.
	if (XR_FAILED(xrSyncActions(Session, &syncInfo)))
		return;

	for (int hand = 0; hand < HandCount; hand++)
	{
		ControllerState& state = Controllers[hand];

		XrActionStateGetInfo getInfo = { XR_TYPE_ACTION_STATE_GET_INFO };
		getInfo.action = MoveAction;
		getInfo.subactionPath = HandPaths[hand];

		XrActionStateVector2f stick = { XR_TYPE_ACTION_STATE_VECTOR2F };
		if (XR_SUCCEEDED(xrGetActionStateVector2f(Session, &getInfo, &stick)) && stick.isActive)
			state.Thumbstick = vec2(stick.currentState.x, stick.currentState.y); // OpenXR: +x right, +y away from the player

		for (int button = 0; button < ButtonCount; button++)
			state.Buttons[button] = ReadButton(ButtonActions[button], hand);

		// The two poses are located independently: a runtime is free to have one tracked and not the
		// other, and a grip that silently fell back to the aim ray would put the weapon somewhere subtly
		// wrong rather than visibly missing.
		LocatePose(GripSpaces[hand], state.Grip);

		TrackedPose aim;
		if (!LocatePose(AimSpaces[hand], aim))
			continue;

		state.PoseValid = true;
		state.Position = aim.Position;
		state.Forward = aim.Forward;
		state.Right = aim.Right;
		state.Up = aim.Up;
	}
}

bool OpenXRSubsystem::LocatePose(XrSpace space, TrackedPose& outPose) const
{
	// Poses are located against the same play space anchor the eyes are, so a controller pose and a head
	// pose can be compared directly. predictedDisplayTime is only meaningful once a frame has been waited
	// on, and locating at time 0 is an error, so before the first frame there is no pose.
	if (space == XR_NULL_HANDLE || FrameState.predictedDisplayTime <= 0)
		return false;

	XrSpaceLocation location = { XR_TYPE_SPACE_LOCATION };
	if (XR_FAILED(xrLocateSpace(space, ViewSpace, FrameState.predictedDisplayTime, &location)))
		return false;
	if (!(location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) || !(location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT))
		return false;

	Pose pose = ConvertPose(location.pose);
	outPose.Valid = true;
	outPose.Position = pose.Position;
	outPose.Forward = pose.Forward;
	outPose.Right = pose.Right;
	outPose.Up = pose.Up;
	return true;
}

void OpenXRSubsystem::Haptic(int handIndex, float amplitude, float duration)
{
	if (!SessionActive || !ActionsAttached || HapticAction == XR_NULL_HANDLE)
		return;
	if (handIndex < 0 || handIndex >= HandCount)
		return;

	XrHapticVibration vibration = { XR_TYPE_HAPTIC_VIBRATION };
	vibration.amplitude = std::min(std::max(amplitude, 0.0f), 1.0f);
	// XR_MIN_HAPTIC_DURATION asks the runtime for the shortest pulse it can do, which is what a "tick"
	// wants and is also the only sane answer for a duration nobody specified.
	vibration.duration = duration > 0.0f ? (XrDuration)(duration * 1e9) : XR_MIN_HAPTIC_DURATION;
	vibration.frequency = XR_FREQUENCY_UNSPECIFIED;

	XrHapticActionInfo info = { XR_TYPE_HAPTIC_ACTION_INFO };
	info.action = HapticAction;
	info.subactionPath = HandPaths[handIndex];

	// Not worth logging on failure: this is fire-and-forget garnish, and a runtime with no haptics on
	// that hand returns XR_SESSION_NOT_FOCUSED or an inactive action every single call.
	xrApplyHapticFeedback(Session, &info, (const XrHapticBaseHeader*)&vibration);
}

Array<std::string> OpenXRSubsystem::GetRequiredVulkanInstanceExtensions()
{
	if (!xrGetVulkanInstanceExtensionsKHR_)
		return {};

	uint32_t count = 0;
	xrGetVulkanInstanceExtensionsKHR_(Instance, SystemId, 0, &count, nullptr);
	if (count == 0)
		return {};

	std::string buffer(count, '\0');
	if (!XR_SUCCEEDED_LOG(xrGetVulkanInstanceExtensionsKHR_(Instance, SystemId, count, &count, buffer.data()), "xrGetVulkanInstanceExtensionsKHR"))
		return {};
	buffer.resize(std::strlen(buffer.c_str()));

	return SplitExtensionString(buffer);
}

Array<std::string> OpenXRSubsystem::GetRequiredVulkanDeviceExtensions(VkPhysicalDevice)
{
	if (!xrGetVulkanDeviceExtensionsKHR_)
		return {};

	uint32_t count = 0;
	xrGetVulkanDeviceExtensionsKHR_(Instance, SystemId, 0, &count, nullptr);
	if (count == 0)
		return {};

	std::string buffer(count, '\0');
	if (!XR_SUCCEEDED_LOG(xrGetVulkanDeviceExtensionsKHR_(Instance, SystemId, count, &count, buffer.data()), "xrGetVulkanDeviceExtensionsKHR"))
		return {};
	buffer.resize(std::strlen(buffer.c_str()));

	return SplitExtensionString(buffer);
}

VkPhysicalDevice OpenXRSubsystem::GetRequiredPhysicalDevice(VkInstance instance)
{
	if (!xrGetVulkanGraphicsDeviceKHR_)
		return nullptr;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	if (!XR_SUCCEEDED_LOG(xrGetVulkanGraphicsDeviceKHR_(Instance, SystemId, instance, &physicalDevice), "xrGetVulkanGraphicsDeviceKHR"))
		return nullptr;
	return physicalDevice;
}

bool OpenXRSubsystem::InitSession(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex)
{
	XrGraphicsBindingVulkanKHR binding = { XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
	binding.instance = instance;
	binding.physicalDevice = physicalDevice;
	binding.device = device;
	binding.queueFamilyIndex = queueFamilyIndex;
	binding.queueIndex = queueIndex;

	XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
	sessionInfo.next = &binding;
	sessionInfo.systemId = SystemId;
	if (!XR_SUCCEEDED_LOG(xrCreateSession(Instance, &sessionInfo, &Session), "xrCreateSession"))
		return false;

	XrPosef identityPose = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };
	XrReferenceSpaceCreateInfo spaceInfo = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	spaceInfo.poseInReferenceSpace = identityPose;
	if (!XR_SUCCEEDED_LOG(xrCreateReferenceSpace(Session, &spaceInfo, &ViewSpace), "xrCreateReferenceSpace"))
	{
		Destroy();
		return false;
	}

	// Same reasoning as CreateActions(): losing the controllers is not a reason to lose the headset too.
	if (!AttachActions())
		LogMessage("OpenXR: motion controller input unavailable");

	uint32_t viewCount = 0;
	xrEnumerateViewConfigurationViews(Instance, SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
	if (viewCount < EyeCount)
	{
		Destroy();
		return false;
	}

	XrViewConfigurationView views[EyeCount] = { { XR_TYPE_VIEW_CONFIGURATION_VIEW }, { XR_TYPE_VIEW_CONFIGURATION_VIEW } };
	xrEnumerateViewConfigurationViews(Instance, SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, EyeCount, &viewCount, views);
	// SteamVR's OpenXR runtime can recommend a per-eye size far larger than the headset's native
	// panel resolution (e.g. 2168x2412 on an Index, whose panels are ~1440x1600) - and its compositor
	// does not handle a swapchain that large correctly, silently only displaying a fraction of it even
	// though the image itself renders and uploads fine. Rendering below the recommendation avoids that,
	// which is why RenderScalePercent defaults to well under 100. Confirmed on Index/SteamVR; unclear if
	// this is universal to OpenXR runtimes, hence a setting rather than a constant - another headset may
	// well want 100 here for a sharper picture.
	EyeWidth = (int)views[0].recommendedImageRectWidth * RenderScalePercent / 100;
	EyeHeight = (int)views[0].recommendedImageRectHeight * RenderScalePercent / 100;
	LogMessage("OpenXR: eye size = " + std::to_string(EyeWidth) + "x" + std::to_string(EyeHeight) +
		" (recommended was " + std::to_string(views[0].recommendedImageRectWidth) + "x" + std::to_string(views[0].recommendedImageRectHeight) +
		", render scale " + std::to_string(RenderScalePercent) + "%)");

	uint32_t formatCount = 0;
	xrEnumerateSwapchainFormats(Session, 0, &formatCount, nullptr);
	std::vector<int64_t> formats(formatCount);
	xrEnumerateSwapchainFormats(Session, formatCount, &formatCount, formats.data());
	if (formats.empty())
	{
		Destroy();
		return false;
	}
	SwapchainFormat = (VkFormat)formats[0]; // Formats are listed in runtime-preferred order.

	for (int eye = 0; eye < EyeCount; eye++)
	{
		XrSwapchainCreateInfo swapchainInfo = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchainInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
		swapchainInfo.format = formats[0];
		swapchainInfo.sampleCount = 1;
		swapchainInfo.width = (uint32_t)EyeWidth;
		swapchainInfo.height = (uint32_t)EyeHeight;
		swapchainInfo.faceCount = 1;
		swapchainInfo.arraySize = 1;
		swapchainInfo.mipCount = 1;

		if (!XR_SUCCEEDED_LOG(xrCreateSwapchain(Session, &swapchainInfo, &Eyes[eye].Swapchain), "xrCreateSwapchain"))
		{
			Destroy();
			return false;
		}

		uint32_t imageCount = 0;
		xrEnumerateSwapchainImages(Eyes[eye].Swapchain, 0, &imageCount, nullptr);
		Eyes[eye].Images.resize(imageCount, XrSwapchainImageVulkanKHR{ XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
		xrEnumerateSwapchainImages(Eyes[eye].Swapchain, imageCount, &imageCount, (XrSwapchainImageBaseHeader*)Eyes[eye].Images.data());
	}

	using namespace std::chrono;
	auto deadline = steady_clock::now() + seconds(5);
	while (CurrentSessionState != XR_SESSION_STATE_READY && steady_clock::now() < deadline)
	{
		PollEvents();
		if (CurrentSessionState == XR_SESSION_STATE_READY)
			break;
		std::this_thread::sleep_for(milliseconds(10));
	}

	if (!SessionActive)
	{
		Destroy();
		return false;
	}

	return true;
}

void OpenXRSubsystem::PollEvents()
{
	XrEventDataBuffer event;
	event.type = XR_TYPE_EVENT_DATA_BUFFER;
	event.next = nullptr;
	while (xrPollEvent(Instance, &event) == XR_SUCCESS)
	{
		if (event.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED)
		{
			auto& stateEvent = reinterpret_cast<XrEventDataSessionStateChanged&>(event);
			CurrentSessionState = stateEvent.state;

			if (CurrentSessionState == XR_SESSION_STATE_READY)
			{
				XrSessionBeginInfo beginInfo = { XR_TYPE_SESSION_BEGIN_INFO };
				beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
				if (XR_SUCCEEDED_LOG(xrBeginSession(Session, &beginInfo), "xrBeginSession"))
					SessionActive = true;
			}
			else if (CurrentSessionState == XR_SESSION_STATE_STOPPING)
			{
				xrEndSession(Session);
				SessionActive = false;
			}
			else if (CurrentSessionState == XR_SESSION_STATE_EXITING || CurrentSessionState == XR_SESSION_STATE_LOSS_PENDING)
			{
				SessionActive = false;
			}
		}

		event.type = XR_TYPE_EVENT_DATA_BUFFER;
		event.next = nullptr;
	}
}

bool OpenXRSubsystem::WaitFrame()
{
	PollEvents();
	if (!SessionActive)
		return false;

	XrFrameWaitInfo waitInfo = { XR_TYPE_FRAME_WAIT_INFO };
	FrameState = { XR_TYPE_FRAME_STATE };
	return XR_SUCCEEDED_LOG(xrWaitFrame(Session, &waitInfo, &FrameState), "xrWaitFrame");
}

void OpenXRSubsystem::BeginFrame()
{
	ViewsLocated = false;
	EyesReleased = 0;

	XrFrameBeginInfo beginInfo = { XR_TYPE_FRAME_BEGIN_INFO };
	xrBeginFrame(Session, &beginInfo);
}

// OpenXR is right handed, metres, -Z forward / +X right / +Y up. SurrealEngine is Unreal units,
// X forward / Y right / Z up. Hence (x,y,z) -> (-z, x, y) on both points and axes.
OpenXRSubsystem::Pose OpenXRSubsystem::ConvertPose(const XrPosef& pose)
{
	auto convertAxis = [](const vec3& v) { return vec3(-v.z, v.x, v.y); };

	Pose result;
	const XrVector3f& p = pose.position;
	result.Position = vec3(-p.z, p.x, p.y) * MetersToUnrealUnits;

	quaternion q(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
	result.Forward = convertAxis(q * vec3(0.0f, 0.0f, -1.0f));
	result.Right = convertAxis(q * vec3(1.0f, 0.0f, 0.0f));
	result.Up = convertAxis(q * vec3(0.0f, 1.0f, 0.0f));
	return result;
}

VRSubsystem::EyeView OpenXRSubsystem::ConvertView(const XrView& view) const
{
	Pose pose = ConvertPose(view.pose);

	VRSubsystem::EyeView result;
	result.Position = pose.Position;
	result.Forward = pose.Forward;
	result.Right = pose.Right;
	result.Up = pose.Up;

	result.AngleLeft = view.fov.angleLeft;
	result.AngleRight = view.fov.angleRight;
	result.AngleUp = view.fov.angleUp;
	result.AngleDown = view.fov.angleDown;
	return result;
}

bool OpenXRSubsystem::LocateViews(EyeView outViews[EyeCount])
{
	XrViewLocateInfo locateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	locateInfo.displayTime = FrameState.predictedDisplayTime;
	locateInfo.space = ViewSpace;

	XrViewState viewState = { XR_TYPE_VIEW_STATE };
	uint32_t viewCount = 0;
	Views[0] = { XR_TYPE_VIEW };
	Views[1] = { XR_TYPE_VIEW };
	Head.Valid = false;
	if (!XR_SUCCEEDED_LOG(xrLocateViews(Session, &locateInfo, &viewState, EyeCount, &viewCount, Views), "xrLocateViews") || viewCount < EyeCount)
		return false;
	if (!(viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) || !(viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT))
		return false;

	for (int eye = 0; eye < EyeCount; eye++)
		outViews[eye] = ConvertView(Views[eye]);

	// The head sits midway between the eyes. Both eyes carry the same orientation (they differ only by
	// the IPD offset along the head's right axis), so either one's axes are the head's.
	Head.Valid = true;
	Head.Position = (outViews[0].Position + outViews[1].Position) * 0.5f;
	Head.Forward = outViews[0].Forward;
	Head.Right = outViews[0].Right;
	Head.Up = outViews[0].Up;

	ViewsLocated = true;
	return true;
}

VkImage OpenXRSubsystem::AcquireSwapchainImage(int eyeIndex)
{
	EyeSwapchain& eye = Eyes[eyeIndex];

	XrSwapchainImageAcquireInfo acquireInfo = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t index = 0;
	if (!XR_SUCCEEDED_LOG(xrAcquireSwapchainImage(eye.Swapchain, &acquireInfo, &index), "xrAcquireSwapchainImage"))
		return nullptr;

	XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	waitInfo.timeout = XR_INFINITE_DURATION;
	if (!XR_SUCCEEDED_LOG(xrWaitSwapchainImage(eye.Swapchain, &waitInfo), "xrWaitSwapchainImage"))
		return nullptr;

	return eye.Images[index].image;
}

void OpenXRSubsystem::ReleaseSwapchainImage(int eyeIndex)
{
	XrSwapchainImageReleaseInfo releaseInfo = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	if (XR_SUCCEEDED_LOG(xrReleaseSwapchainImage(Eyes[eyeIndex].Swapchain, &releaseInfo), "xrReleaseSwapchainImage"))
		EyesReleased++;
}

void OpenXRSubsystem::EndFrame()
{
	// Submitting the projection layer without a fresh pose for every eye and a released image behind it
	// would have the runtime reject the whole frame (and, since we can't fix it up here, silently drop
	// it): the poses would be last frame's, and the swapchains would still be checked out to us.
	bool haveCompleteFrame = FrameState.shouldRender && ViewsLocated && EyesReleased == EyeCount;

	XrCompositionLayerProjectionView projViews[EyeCount] = { { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW }, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW } };
	for (int eye = 0; eye < EyeCount; eye++)
	{
		projViews[eye].pose = Views[eye].pose;
		projViews[eye].fov = Views[eye].fov;
		projViews[eye].subImage.swapchain = Eyes[eye].Swapchain;
		projViews[eye].subImage.imageRect.offset = { 0, 0 };
		projViews[eye].subImage.imageRect.extent = { EyeWidth, EyeHeight };
		projViews[eye].subImage.imageArrayIndex = 0;
	}

	XrCompositionLayerProjection layer = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	layer.space = ViewSpace;
	layer.viewCount = EyeCount;
	layer.views = projViews;

	const XrCompositionLayerBaseHeader* layers[1] = { (const XrCompositionLayerBaseHeader*)&layer };

	XrFrameEndInfo endInfo = { XR_TYPE_FRAME_END_INFO };
	endInfo.displayTime = FrameState.predictedDisplayTime;
	endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	endInfo.layerCount = haveCompleteFrame ? 1 : 0;
	endInfo.layers = haveCompleteFrame ? layers : nullptr;
	XR_SUCCEEDED_LOG(xrEndFrame(Session, &endInfo), "xrEndFrame");
}
