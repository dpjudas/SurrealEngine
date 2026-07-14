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

// Optional VR support (head tracking + stereo rendering). Off by default at runtime.
// See VRSubsystem::Create. All virtuals default to inert/no-op behavior so callers
// never need to branch on whether VR was compiled in - only on IsActive().
class VRSubsystem
{
public:
	// Returns a NullVRSubsystem when !enabled, when not built with USE_OPENXR, or when
	// no OpenXR runtime could be reached - VR failing to initialize must never prevent
	// the desktop game from starting.
	static std::unique_ptr<VRSubsystem> Create(bool enabled);

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

	// Per-frame sequence: WaitFrame -> BeginFrame -> LocateViews -> for each eye:
	// AcquireSwapchainImage, render, ReleaseSwapchainImage -> EndFrame.
	virtual bool WaitFrame() { return false; }
	virtual void BeginFrame() {}
	virtual bool LocateViews(EyeView outViews[EyeCount]) { return false; }
	virtual VkImage AcquireSwapchainImage(int eyeIndex) { return nullptr; }
	virtual void ReleaseSwapchainImage(int eyeIndex) {}
	virtual void EndFrame(const EyeView views[EyeCount]) {}

	virtual int GetRecommendedEyeWidth() const { return 0; }
	virtual int GetRecommendedEyeHeight() const { return 0; }
	virtual uint32_t GetSwapchainFormat() const { return 0; } // VkFormat, as uint32_t to avoid needing vulkan.h here
};

class NullVRSubsystem : public VRSubsystem
{
public:
	bool IsActive() const override { return false; }
};
