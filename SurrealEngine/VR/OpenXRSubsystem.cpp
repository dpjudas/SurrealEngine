
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

	if (ViewSpace != XR_NULL_HANDLE)
	{
		xrDestroySpace(ViewSpace);
		ViewSpace = XR_NULL_HANDLE;
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

VRSubsystem::EyeView OpenXRSubsystem::ConvertView(const XrView& view) const
{
	VRSubsystem::EyeView result;

	const XrVector3f& p = view.pose.position;
	result.Position = vec3(-p.z, p.x, p.y) * MetersToUnrealUnits;

	quaternion q(view.pose.orientation.x, view.pose.orientation.y, view.pose.orientation.z, view.pose.orientation.w);
	vec3 forwardXR = q * vec3(0.0f, 0.0f, -1.0f);
	vec3 rightXR = q * vec3(1.0f, 0.0f, 0.0f);
	vec3 upXR = q * vec3(0.0f, 1.0f, 0.0f);

	auto convertAxis = [](const vec3& v) { return vec3(-v.z, v.x, v.y); };
	result.Forward = convertAxis(forwardXR);
	result.Right = convertAxis(rightXR);
	result.Up = convertAxis(upXR);

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
	if (!XR_SUCCEEDED_LOG(xrLocateViews(Session, &locateInfo, &viewState, EyeCount, &viewCount, Views), "xrLocateViews") || viewCount < EyeCount)
		return false;
	if (!(viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) || !(viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT))
		return false;

	for (int eye = 0; eye < EyeCount; eye++)
		outViews[eye] = ConvertView(Views[eye]);
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
