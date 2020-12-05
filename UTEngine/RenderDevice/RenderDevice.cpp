
#include "Precomp.h"
#include "RenderDevice.h"
#include "Vulkan/VulkanRenderDevice.h"

std::unique_ptr<RenderDevice> RenderDevice::Create(::Viewport* viewport)
{
	return std::make_unique<VulkanRenderDevice>(viewport);
}
