
#include "Precomp.h"
#include "TextureManager.h"
#include "VulkanRenderDevice.h"
#include "CachedTexture.h"
#include <zvulkan/vulkanbuilders.h>
#include "UObject/UTexture.h"

TextureManager::TextureManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	CreateNullTexture();
	CreateDitherTexture();
}

TextureManager::~TextureManager()
{
}

void TextureManager::UpdateTextureRect(FTextureInfo* info, int x, int y, int w, int h)
{
	std::unique_ptr<CachedTexture>& tex = TextureCache[0][info->CacheID];
	if (tex)
	{
		renderer->Uploads->UploadTextureRect(tex.get(), *info, x, y, w, h);
		info->bRealtimeChanged = 0;
	}
}

CachedTexture* TextureManager::GetTexture(FTextureInfo* info, bool masked)
{
	if (!info)
		return nullptr;

	std::unique_ptr<CachedTexture>& tex = TextureCache[(int)masked][info->CacheID];
	if (!tex)
	{
		tex.reset(new CachedTexture());
		renderer->Uploads->UploadTexture(tex.get(), *info, masked);
	}
	else if (info->bRealtimeChanged /*&& (!info->Texture || info->Texture->RealtimeChangeCount != tex->RealtimeChangeCount)*/)
	{
		/*if (info->Texture)
			info->Texture->RealtimeChangeCount = tex->RealtimeChangeCount;*/
		info->bRealtimeChanged = 0;
		renderer->Uploads->UploadTexture(tex.get(), *info, masked);
	}
	return tex.get();
}

void TextureManager::ClearCache()
{
	for (auto& cache : TextureCache)
	{
		cache.clear();
	}
}

void TextureManager::CreateNullTexture()
{
	auto cmdbuffer = renderer->Commands->GetTransferCommands();

	NullTexture = ImageBuilder()
		.Format(VK_FORMAT_R8G8B8A8_UNORM)
		.Size(1, 1)
		.Usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.DebugName("NullTexture")
		.Create(renderer->Device.get());

	NullTextureView = ImageViewBuilder()
		.Image(NullTexture.get(), VK_FORMAT_R8G8B8A8_UNORM)
		.DebugName("NullTextureView")
		.Create(renderer->Device.get());

	auto stagingbuffer = BufferBuilder()
		.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY)
		.Size(4)
		.DebugName("NullTextureStaging")
		.Create(renderer->Device.get());

	auto data = (uint32_t*)stagingbuffer->Map(0, 4);
	data[0] = 0xffffffff;
	stagingbuffer->Unmap();

	PipelineBarrier()
		.AddImage(NullTexture.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region = {};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = { 1, 1, 1 };
	cmdbuffer->copyBufferToImage(stagingbuffer->buffer, NullTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	PipelineBarrier()
		.AddImage(NullTexture.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	renderer->Commands->FrameDeleteList->buffers.push_back(std::move(stagingbuffer));
}

void TextureManager::CreateDitherTexture()
{
	static const float ditherdata[64] =
	{
			.0078125, .2578125, .1328125, .3828125, .0234375, .2734375, .1484375, .3984375,
			.7578125, .5078125, .8828125, .6328125, .7734375, .5234375, .8984375, .6484375,
			.0703125, .3203125, .1953125, .4453125, .0859375, .3359375, .2109375, .4609375,
			.8203125, .5703125, .9453125, .6953125, .8359375, .5859375, .9609375, .7109375,
			.0390625, .2890625, .1640625, .4140625, .0546875, .3046875, .1796875, .4296875,
			.7890625, .5390625, .9140625, .6640625, .8046875, .5546875, .9296875, .6796875,
			.1015625, .3515625, .2265625, .4765625, .1171875, .3671875, .2421875, .4921875,
			.8515625, .6015625, .9765625, .7265625, .8671875, .6171875, .9921875, .7421875,
	};

	auto cmdbuffer = renderer->Commands->GetTransferCommands();

	DitherImage = ImageBuilder()
		.Format(VK_FORMAT_R32_SFLOAT)
		.Size(8, 8)
		.Usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.DebugName("DitherImage")
		.Create(renderer->Device.get());

	DitherImageView = ImageViewBuilder()
		.Image(DitherImage.get(), VK_FORMAT_R32_SFLOAT)
		.DebugName("DitherImageView")
		.Create(renderer->Device.get());

	auto stagingbuffer = BufferBuilder()
		.Usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY)
		.Size(sizeof(ditherdata))
		.DebugName("DitherImageStaging")
		.Create(renderer->Device.get());

	auto data = (uint32_t*)stagingbuffer->Map(0, sizeof(ditherdata));
	memcpy(data, ditherdata, sizeof(ditherdata));
	stagingbuffer->Unmap();

	PipelineBarrier()
		.AddImage(DitherImage.get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region = {};
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = { 8, 8, 1 };
	cmdbuffer->copyBufferToImage(stagingbuffer->buffer, DitherImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	PipelineBarrier()
		.AddImage(DitherImage.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)
		.Execute(cmdbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	renderer->Commands->FrameDeleteList->buffers.push_back(std::move(stagingbuffer));
}
