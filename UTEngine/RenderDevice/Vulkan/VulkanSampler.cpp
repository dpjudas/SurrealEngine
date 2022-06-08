
#include "Precomp.h"
#include "VulkanSampler.h"
#include "VulkanBuilders.h"

VulkanSamplerManager::VulkanSamplerManager(VulkanDevice *device)
{
	for (int i = 0; i < 4; i++)
	{
		SamplerBuilder builder;
		builder.setAnisotropy(8.0);

		if (i & 1)
		{
			builder.setMinFilter(VK_FILTER_NEAREST);
			builder.setMagFilter(VK_FILTER_NEAREST);
			builder.setMipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST);
		}
		else
		{
			builder.setMinFilter(VK_FILTER_LINEAR);
			builder.setMagFilter(VK_FILTER_LINEAR);
			builder.setMipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR);
		}

		if (i & 2)
		{
			builder.setAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		}
		else
		{
			builder.setAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT);
		}

		samplers[i] = builder.create(device);
	}

	// To do: detail texture needs a zbias of 15
}
