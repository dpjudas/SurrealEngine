
#include "Precomp.h"
#include "RenderDevice.h"
#include "Vulkan/VulkanRenderDevice.h"

std::unique_ptr<RenderDevice> RenderDevice::Create(DisplayWindow* viewport, std::shared_ptr<VulkanSurface> surface)
{
	return std::make_unique<VulkanRenderDevice>(viewport, surface);
}
