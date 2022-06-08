
#include "Precomp.h"
#include "VulkanLight.h"
#include "VulkanBuilders.h"
#include "VulkanRenderDevice.h"

VulkanLightManager::VulkanLightManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	BufferBuilder bufbuild;

	bufbuild.setSize(sizeof(SceneLight) * MaxLights);
	bufbuild.setUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	Lights = bufbuild.create(renderer->Device);
	bufbuild.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	StagingLights = bufbuild.create(renderer->Device);

	bufbuild.setSize(1'000'000 * sizeof(int32_t));
	bufbuild.setUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	SurfaceLights = bufbuild.create(renderer->Device);
	bufbuild.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	StagingSurfaceLights = bufbuild.create(renderer->Device);
}

VulkanLightManager::~VulkanLightManager()
{
}
