
#include "Precomp.h"
#include "VulkanLight.h"
#include "VulkanBuilders.h"
#include "VulkanRenderDevice.h"
#include "VulkanCommandBuffer.h"
#include "UObject/UActor.h"

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

void VulkanLightManager::UpdateLights(const std::vector<std::pair<int, UActor*>>& LightUpdates)
{
	if (LightUpdates.empty())
		return;

	int minIndex = LightUpdates.front().first;
	int maxIndex = LightUpdates.front().first;
	for (auto& update : LightUpdates)
	{
		minIndex = std::min(update.first, minIndex);
		maxIndex = std::max(update.first, maxIndex);
	}
	int count = maxIndex - minIndex + 1;

	size_t offset = minIndex * sizeof(SceneLight);
	size_t size = count * sizeof(SceneLight);
	SceneLight* dest = (SceneLight*)StagingLights->Map(offset, size);

	for (auto& update : LightUpdates)
	{
		int index = update.first;
		UActor* slight = update.second;
		SceneLight& dlight = dest[index - minIndex];

		dlight.Location = slight->Location();
		dlight.Unused = 0.0f;
		dlight.LightBrightness = (float)slight->LightBrightness();
		dlight.LightHue = (float)slight->LightHue();
		dlight.LightSaturation = (float)slight->LightSaturation();
		dlight.LightRadius = (float)slight->LightRadius();
	}

	StagingLights->Unmap();

	auto cmdbuffer = renderer->Commands->GetTransferCommands();
	cmdbuffer->copyBuffer(StagingLights.get(), Lights.get(), offset, size);
}

void VulkanLightManager::UpdateSurfaceLights(const std::vector<int32_t>& surfaceLights)
{
	if (surfaceLights.empty())
		return;

	size_t size = surfaceLights.size() * sizeof(int32_t);
	int32_t* dest = (int32_t*)StagingSurfaceLights->Map(0, size);
	memcpy(dest, surfaceLights.data(), size);
	StagingSurfaceLights->Unmap();

	auto cmdbuffer = renderer->Commands->GetTransferCommands();
	cmdbuffer->copyBuffer(StagingSurfaceLights.get(), SurfaceLights.get(), 0, size);
}
