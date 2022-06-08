
#include "Precomp.h"
#include "VulkanTexture.h"
#include "VulkanObjects.h"
#include "VulkanBuilders.h"
#include "VulkanDescriptorSet.h"
#include "VulkanRenderDevice.h"
#include "VulkanCommandBuffer.h"
#include "PixelBuffer.h"
#include "VulkanRenderDevice.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"

VulkanTextureManager::VulkanTextureManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
}

VulkanTextureManager::~VulkanTextureManager()
{
	ClearTextureCache();
}

VulkanTexture* VulkanTextureManager::GetTexture(FTextureInfo* texture, uint32_t polyFlags)
{
	if (!texture)
		return nullptr;

	std::unique_ptr<VulkanTexture>& tex = TextureCache[texture->CacheID];
	if (!tex)
	{
		tex = std::make_unique<VulkanTexture>(renderer, *texture, polyFlags);
	}
	else if (texture->bRealtimeChanged)
	{
		texture->bRealtimeChanged = false;
		tex->Update(renderer, *texture, polyFlags);
	}
	return tex.get();
}

void VulkanTextureManager::ClearTextureCache()
{
	renderer->DescriptorSets->ClearTextureDescriptors();
	TextureCache.clear();
}

/////////////////////////////////////////////////////////////////////////////

VulkanTexture::VulkanTexture(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags)
{
	Update(renderer, Info, PolyFlags);
}

VulkanTexture::~VulkanTexture()
{
}

void VulkanTexture::Update(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags)
{
	int USize = Info.Texture->Mipmaps.front().Width;
	int VSize = Info.Texture->Mipmaps.front().Height;

	UMult = 1.0f / (Info.UScale * USize);
	VMult = 1.0f / (Info.VScale * VSize);

	UploadedData data;
	if ((uint32_t)USize > renderer->Device->physicalDevice.properties.limits.maxImageDimension2D || (uint32_t)VSize > renderer->Device->physicalDevice.properties.limits.maxImageDimension2D)
	{
		// To do: texture is too big. find the first mipmap level that fits and use that as the base size
		data = UploadWhite(renderer, Info, PolyFlags);
	}
	else
	{
		auto block4x4_to_64bits = [](auto mip) { return ((mip->Width + 3) / 4) * ((mip->Height + 3) / 4) * 8; };
		auto block4x4_to_128bits = [](auto mip) { return ((mip->Width + 3) / 4) * ((mip->Height + 3) / 4) * 16; };

		switch (Info.Texture->ActualFormat)
		{
		case TextureFormat::P8:
			{
				uint32_t NewPal[256];
				for (int i = 0; i < 256; i++)
				{
					NewPal[i] = Info.Texture->Palette()->Colors[i];
				}
				if (PolyFlags & PF_Masked)
					NewPal[0] = 0;

				data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_R8G8B8A8_UNORM, [](auto mip) { return mip->Width * mip->Height * 4; },
					[&](auto mip, auto dst)
					{
						auto Ptr = (uint32_t*)dst;
						uint32_t mipwidth = mip->Width;
						uint32_t mipheight = mip->Height;
						for (uint32_t y = 0; y < mipheight; y++)
						{
							uint8_t* line = (uint8_t*)mip->Data.data() + y * mip->Width;
							for (uint32_t x = 0; x < mipwidth; x++)
							{
								*Ptr++ = NewPal[line[x]];
							}
						}
					});
			}
			break;
		case TextureFormat::RGBA7:
			data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_R8G8B8A8_UNORM, [](auto mip) { return mip->Width * mip->Height * 4; },
				[](auto mip, auto dst)
				{
					auto Ptr = (FColor*)dst;
					uint32_t mipwidth = mip->Width;
					uint32_t mipheight = mip->Height;
					for (uint32_t y = 0; y < mipheight; y++)
					{
						FColor* line = (FColor*)mip->Data.data() + y * mip->Width;
						for (uint32_t x = 0; x < mipwidth; x++)
						{
							const FColor& Src = line[x];
							Ptr->R = Src.B;
							Ptr->G = Src.G;
							Ptr->B = Src.R;
							Ptr->A = Src.A * 2;
							Ptr++;
						}
					}
				});
			break;
		case TextureFormat::RGB16: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_R5G6B5_UNORM_PACK16, [](auto mip) { return mip->Width * mip->Height * 2; }); break;
		case TextureFormat::DXT1: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, block4x4_to_64bits); break;
		case TextureFormat::RGB8: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_R8G8B8_UNORM, [](auto mip) { return mip->Width * mip->Height * 3; }); break;
		case TextureFormat::RGBA8: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_B8G8R8A8_UNORM, [](auto mip) { return mip->Width * mip->Height * 4; }); break;
		case TextureFormat::BC2: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC2_UNORM_BLOCK, block4x4_to_128bits); break;
		case TextureFormat::BC3: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC3_UNORM_BLOCK, block4x4_to_128bits); break;
		case TextureFormat::BC1_PA: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC1_RGBA_UNORM_BLOCK, block4x4_to_64bits); break;
		case TextureFormat::BC7: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC7_UNORM_BLOCK, block4x4_to_128bits); break;
		case TextureFormat::BC6H_S: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC6H_SFLOAT_BLOCK, block4x4_to_128bits); break;
		case TextureFormat::BC6H: data = UploadData(renderer, Info, PolyFlags, VK_FORMAT_BC6H_UFLOAT_BLOCK, block4x4_to_128bits); break;
		default: data = UploadWhite(renderer, Info, PolyFlags); break;
		}
	}

	if (!image)
	{
		ImageBuilder imgbuilder;
		imgbuilder.setFormat(data.imageFormat);
		imgbuilder.setSize(data.width, data.height, (int)data.miplevels.size());
		imgbuilder.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		image = imgbuilder.create(renderer->Device);

		ImageViewBuilder viewbuilder;
		viewbuilder.setImage(image.get(), data.imageFormat);
		imageView = viewbuilder.create(renderer->Device);
	}

	auto cmdbuffer = renderer->Commands->GetTransferCommands();

	PipelineBarrier imageTransition0;
	imageTransition0.addImage(image.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, (int)data.miplevels.size());
	imageTransition0.execute(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	cmdbuffer->copyBufferToImage(data.stagingbuffer->buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)data.miplevels.size(), data.miplevels.data());

	PipelineBarrier imageTransition1;
	imageTransition1.addImage(image.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t)data.miplevels.size());
	imageTransition1.execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	renderer->Commands->FrameDeleteList->buffers.push_back(std::move(data.stagingbuffer));
}

UploadedData VulkanTexture::UploadData(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags, VkFormat imageFormat, std::function<int(UnrealMipmap* mip)> calcMipSize, std::function<void(UnrealMipmap* mip, void* dst)> copyMip)
{
	int USize = Info.Texture->Mipmaps.front().Width;
	int VSize = Info.Texture->Mipmaps.front().Height;

	UploadedData result;
	result.imageFormat = imageFormat;
	result.width = USize;
	result.height = VSize;

	size_t pixelsSize = 0;
	for (auto& Mip : Info.Texture->Mipmaps)
	{
		if (!Mip.Data.empty())
		{
			int mipsize = calcMipSize(&Mip);
			mipsize = (mipsize + 15) / 16 * 16; // memory alignment
			pixelsSize += mipsize;
		}
	}

	BufferBuilder builder;
	builder.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	builder.setSize(pixelsSize);
	result.stagingbuffer = builder.create(renderer->Device);

	auto data = (uint8_t*)result.stagingbuffer->Map(0, pixelsSize);
	auto Ptr = data;

	for (int level = 0; level < (int)Info.Texture->Mipmaps.size(); level++)
	{
		UnrealMipmap* Mip = &Info.Texture->Mipmaps[level];
		if (!Mip->Data.empty())
		{
			uint32_t mipwidth = Mip->Width;
			uint32_t mipheight = Mip->Height;

			VkBufferImageCopy region = {};
			region.bufferOffset = (VkDeviceSize)((uint8_t*)Ptr - (uint8_t*)data);
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = level;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mipwidth, mipheight, 1 };
			result.miplevels.push_back(region);

			int mipsize = calcMipSize(Mip);
			if (copyMip)
				copyMip(Mip, Ptr);
			else
				memcpy(Ptr, Mip->Data.data(), mipsize);
			mipsize = (mipsize + 15) / 16 * 16; // memory alignment
			Ptr += mipsize;
		}
	}

	result.stagingbuffer->Unmap();
	return result;
}

UploadedData VulkanTexture::UploadWhite(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags)
{
	UploadedData result;
	result.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	result.width = 1;
	result.height = 1;

	BufferBuilder builder;
	builder.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	builder.setSize(4);
	result.stagingbuffer = builder.create(renderer->Device);
	auto data = (uint32_t*)result.stagingbuffer->Map(0, 4);

	data[0] = 0xffffffff;

	result.stagingbuffer->Unmap();

	VkBufferImageCopy region = {};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = { 1, 1, 1 };
	result.miplevels.push_back(region);

	return result;
}
