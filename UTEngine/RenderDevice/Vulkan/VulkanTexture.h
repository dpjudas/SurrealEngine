#pragma once

#include "VulkanObjects.h"
#include <functional>

class VulkanRenderDevice;

struct FColor;
struct FTextureInfo;
class UnrealMipmap;

struct UploadedData
{
	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	int width = 1;
	int height = 1;
	std::unique_ptr<VulkanBuffer> stagingbuffer;
	std::vector<VkBufferImageCopy> miplevels;
};

class VulkanTexture
{
public:
	VulkanTexture(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags);
	~VulkanTexture();

	void Update(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags);

	float UMult = 1.0f;
	float VMult = 1.0f;

	std::unique_ptr<VulkanImage> image;
	std::unique_ptr<VulkanImageView> imageView;

private:
	UploadedData UploadData(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags, VkFormat imageFormat, std::function<int(UnrealMipmap* mip)> calcMipSize, std::function<void(UnrealMipmap* mip, void* dst)> copyMip = {});
	UploadedData UploadWhite(VulkanRenderDevice* renderer, const FTextureInfo& Info, uint32_t PolyFlags);
};

class VulkanTextureManager
{
public:
	VulkanTextureManager(VulkanRenderDevice* renderer);
	~VulkanTextureManager();

	VulkanTexture* GetTexture(FTextureInfo* texture, uint32_t polyFlags);
	void ClearTextureCache();

private:
	VulkanRenderDevice* renderer = nullptr;

	std::map<uint64_t, std::unique_ptr<VulkanTexture>> TextureCache;
};
