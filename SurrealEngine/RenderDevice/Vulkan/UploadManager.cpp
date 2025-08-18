
#include "Precomp.h"
#include "UploadManager.h"
#include "VulkanRenderDevice.h"
#include "CachedTexture.h"
#include <zvulkan/vulkanbuilders.h>

UploadManager::UploadManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
}

UploadManager::~UploadManager()
{
}

void UploadManager::ClearCache()
{
	PendingUploads.clear();
}

bool UploadManager::SupportsTextureFormat(TextureFormat Format) const
{
	return TextureUploader::GetUploader(Format);
}

void UploadManager::UploadTexture(CachedTexture* tex, const FTextureInfo& Info, bool masked)
{
	int width = Info.USize;
	int height = Info.VSize;
	int mipcount = Info.NumMips;

	TextureUploader* uploader = TextureUploader::GetUploader(Info.Format);

	if ((uint32_t)Info.USize > renderer->Device.get()->PhysicalDevice.Properties.Properties.limits.maxImageDimension2D ||
		(uint32_t)Info.VSize > renderer->Device.get()->PhysicalDevice.Properties.Properties.limits.maxImageDimension2D ||
		!uploader)
	{
		width = 1;
		height = 1;
		mipcount = 1;
		uploader = nullptr;
	}

	VkFormat format = uploader ? uploader->GetVkFormat() : VK_FORMAT_R8G8B8A8_UNORM;

	if (!tex->image)
	{
		tex->image = ImageBuilder()
			.Format(format)
			.Size(width, height, mipcount)
			.Usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.DebugName("CachedTexture.Image")
			.Create(renderer->Device.get());

		tex->imageView = ImageViewBuilder()
			.Image(tex->image.get(), format)
			.DebugName("CachedTexture.ImageView")
			.Create(renderer->Device.get());
	}

	tex->pendingUploads[0].clear();
	tex->pendingUploads[1].clear();

	if (uploader)
		UploadData(tex, Info, masked, uploader);
	else
		UploadWhite(tex);
}

void UploadManager::UploadTextureRect(CachedTexture* tex, const FTextureInfo& Info, int x, int y, int w, int h)
{
	TextureUploader* uploader = TextureUploader::GetUploader(Info.Format);
	if (!uploader || Info.NumMips < 1 || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > Info.Mips[0].Width || y + h > Info.Mips[0].Height || Info.Mips[0].Data.empty())
		return;

	size_t pixelsSize = uploader->GetUploadSize(x, y, w, h);
	pixelsSize = (pixelsSize + 15) / 16 * 16; // memory alignment

	WaitIfUploadBufferIsFull((int)pixelsSize);

	uint8_t* data = renderer->Buffers->UploadData;
	uint8_t* Ptr = data + UploadBufferPos;
	uploader->UploadRect(Ptr, &Info.Mips[0], x, y, w, h, Info.Palette, false);

	VkBufferImageCopy region = {};
	region.bufferOffset = (VkDeviceSize)(Ptr - data);
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { (int32_t)x, (int32_t)y, 0 };
	region.imageExtent = { (uint32_t)w, (uint32_t)h, 1 };

	AddPendingUpload(tex, region, true);

	UploadBufferPos += (int)pixelsSize;
}

void UploadManager::UploadData(CachedTexture* tex, const FTextureInfo& Info, bool masked, TextureUploader* uploader)
{
	size_t pixelsSize = 0;
	for (INT level = 0; level < Info.NumMips; level++)
	{
		UnrealMipmap* Mip = &Info.Mips[level];
		if (!Mip->Data.empty())
		{
			INT mipsize = uploader->GetUploadSize(0, 0, Mip->Width, Mip->Height);
			mipsize = (mipsize + 15) / 16 * 16; // memory alignment
			pixelsSize += mipsize;
		}
	}

	WaitIfUploadBufferIsFull((int)pixelsSize);

	for (INT level = 0; level < Info.NumMips; level++)
	{
		UnrealMipmap* Mip = &Info.Mips[level];
		if (!Mip->Data.empty())
		{
			uint32_t mipwidth = Mip->Width;
			uint32_t mipheight = Mip->Height;

			VkBufferImageCopy region = {};
			region.bufferOffset = UploadBufferPos;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = level;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { mipwidth, mipheight, 1 };
			AddPendingUpload(tex, region, false);

			uploader->UploadRect(renderer->Buffers->UploadData + UploadBufferPos, Mip, 0, 0, Mip->Width, Mip->Height, Info.Palette, masked);

			INT mipsize = uploader->GetUploadSize(0, 0, Mip->Width, Mip->Height);
			mipsize = (mipsize + 15) / 16 * 16; // memory alignment
			UploadBufferPos += mipsize;
		}
	}
}

void UploadManager::UploadWhite(CachedTexture* tex)
{
	WaitIfUploadBufferIsFull(16);

	auto data = (uint32_t*)(renderer->Buffers->UploadData + UploadBufferPos);
	data[0] = 0xffffffff;

	VkBufferImageCopy region = {};
	region.bufferOffset = UploadBufferPos;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = { 1, 1, 1 };
	AddPendingUpload(tex, region, false);

	UploadBufferPos += 16; // 16-byte aligned
}

void UploadManager::WaitIfUploadBufferIsFull(int bytes)
{
	if (UploadBufferPos + bytes > BufferManager::UploadBufferSize)
	{
		renderer->Commands->WaitForTransfer();
	}
}

void UploadManager::AddPendingUpload(CachedTexture* tex, const VkBufferImageCopy& region, bool isPartial)
{
	if (!tex->inPendingUploads)
	{
		PendingUploads.push_back(tex);
		tex->inPendingUploads = true;
	}

	tex->pendingUploads[isPartial].push_back(region);
}

void UploadManager::SubmitUploads()
{
	if (PendingUploads.empty())
		return;

	auto cmdbuffer = renderer->Commands->GetTransferCommands();

	// Transition images to transfer
	PipelineBarrier beforeBarrier;
	for (CachedTexture* tex : PendingUploads)
	{
		beforeBarrier.AddImage(
			tex->image->image,
			tex->imageLayout,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, tex->image->mipLevels);

		tex->imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	beforeBarrier.Execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	// Do full texture uploads, then partial
	for (int i = 0; i < 2; i++)
	{
		// Copy from buffer to images
		VkBuffer buffer = renderer->Buffers->UploadBuffer->buffer;
		for (CachedTexture* tex : PendingUploads)
		{
			if (!tex->pendingUploads[i].empty())
			{
				if (i == 0)
					renderer->Stats.Uploads++;
				else
					renderer->Stats.RectUploads++;

				cmdbuffer->copyBufferToImage(buffer, tex->image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)tex->pendingUploads[i].size(), tex->pendingUploads[i].data());
			}
		}
	}

	// Transition images to texture sampling
	PipelineBarrier afterBarrier;
	for (CachedTexture* tex : PendingUploads)
	{
		afterBarrier.AddImage(
			tex->image->image,
			tex->imageLayout,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, tex->image->mipLevels);

		tex->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	afterBarrier.Execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	// Remove textures from pending uploads
	for (CachedTexture* tex : PendingUploads)
	{
		tex->pendingUploads[0].clear();
		tex->pendingUploads[1].clear();
		tex->inPendingUploads = false;
	}
	PendingUploads.clear();
	UploadBufferPos = 0;
}
