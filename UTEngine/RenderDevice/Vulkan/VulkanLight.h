#pragma once

#include "VulkanObjects.h"
#include "Math/mat.h"
#include "Math/vec.h"

class VulkanRenderDevice;

struct SceneLight
{
	vec3 Location;
	float Unused;
	float LightBrightness;
	float LightHue;
	float LightSaturation;
	float LightRadius;
};

class VulkanLightManager
{
public:
	VulkanLightManager(VulkanRenderDevice* renderer);
	~VulkanLightManager();

	static const int MaxLights = 682;

	std::unique_ptr<VulkanBuffer> Lights;
	std::unique_ptr<VulkanBuffer> StagingLights;

	std::unique_ptr<VulkanBuffer> SurfaceLights;
	std::unique_ptr<VulkanBuffer> StagingSurfaceLights;

private:
	VulkanLightManager(const VulkanLightManager&) = delete;
	VulkanLightManager& operator=(const VulkanLightManager&) = delete;

	VulkanRenderDevice* renderer = nullptr;
};
