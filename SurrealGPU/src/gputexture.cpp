
#include "gputexture.h"
#include "gpudevice.h"
#include "vulkanbuilders.h"

GPUSampler::GPUSampler(GPUDevice* device, const GPUSamplerDesc& desc) : device(device), desc(desc)
{
	VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.magFilter = (VkFilter)desc.magFilter;
	samplerInfo.minFilter = (VkFilter)desc.minFilter;
	samplerInfo.addressModeU = (VkSamplerAddressMode)desc.addressModeU;
	samplerInfo.addressModeV = (VkSamplerAddressMode)desc.addressModeV;
	samplerInfo.addressModeW = (VkSamplerAddressMode)desc.addressModeW;
	samplerInfo.anisotropyEnable = (VkBool32)desc.anisotropyEnable;
	samplerInfo.maxAnisotropy = desc.maxAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = (VkSamplerMipmapMode)desc.mipmapMode;
	samplerInfo.mipLodBias = desc.mipLodBias;
	samplerInfo.minLod = desc.minLod;
	samplerInfo.maxLod = desc.maxLod;

	VkSampler vksampler;
	VkResult result = vkCreateSampler(device->getVulkanDevice()->device, &samplerInfo, nullptr, &vksampler);
	device->getVulkanDevice()->CheckVulkanError(result, "Could not create texture sampler");
	sampler = std::make_unique<VulkanSampler>(device->getVulkanDevice(), vksampler);
	if (!desc.label.empty())
		sampler->SetDebugName(desc.label.c_str());
}

/////////////////////////////////////////////////////////////////////////////

GPUTexture::GPUTexture(GPUDevice* device, const GPUTextureDesc& desc) : device(device), desc(desc)
{
	ImageBuilder builder;
	image = builder.Create(device->getVulkanDevice());
}

std::shared_ptr<GPUTextureView> GPUTexture::createView()
{
	return std::make_shared<GPUTextureView>(this);
}

void GPUTexture::destroy()
{
}

/////////////////////////////////////////////////////////////////////////////

GPUTextureView::GPUTextureView(GPUTexture* texture)
{
	ImageViewBuilder builder;
	view = builder.Create(texture->device->getVulkanDevice());
}

/////////////////////////////////////////////////////////////////////////////

GPUExternalTexture::GPUExternalTexture(GPUDevice* device, const GPUExternalTextureDesc& desc) : device(device), desc(desc)
{
}
