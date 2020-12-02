#pragma once

#include "VulkanObjects.h"
#include "Math/mat.h"
#include "Math/vec.h"

class Renderer;

struct SceneLight
{
	vec3 Location;
	float Shadowmap;
	float LightBrightness;
	float LightHue;
	float LightSaturation;
	float LightRadius;
};

class SceneLights
{
public:
	SceneLights(Renderer* renderer);
	~SceneLights();

	static const int ShadowmapSize = 64;
	static const int MaxLights = 682;

	std::unique_ptr<VulkanImage> ShadowColorBuffer;
	std::unique_ptr<VulkanImage> ShadowDepthBuffer;
	std::unique_ptr<VulkanImageView> ShadowColorBufferView;
	std::unique_ptr<VulkanImageView> ShadowDepthBufferView;

	std::unique_ptr<VulkanImage> Shadowmap;
	std::unique_ptr<VulkanImageView> ShadowmapView;

	std::unique_ptr<VulkanBuffer> Lights;
	std::unique_ptr<VulkanBuffer> StagingLights;

	std::unique_ptr<VulkanBuffer> SurfaceLights;
	std::unique_ptr<VulkanBuffer> StagingSurfaceLights;

private:
	SceneLights(const SceneLights&) = delete;
	SceneLights& operator=(const SceneLights&) = delete;

	static void createImage(std::unique_ptr<VulkanImage>& image, std::unique_ptr<VulkanImageView>& view, VulkanDevice* device, int width, int height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

	Renderer* renderer = nullptr;
};
