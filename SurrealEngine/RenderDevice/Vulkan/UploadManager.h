#pragma once

#include <zvulkan/vulkanobjects.h>
#include "TextureUploader.h"

class VulkanRenderDevice;
class CachedTexture;
struct FTextureInfo;

class UploadManager
{
public:
	UploadManager(VulkanRenderDevice* renderer);
	~UploadManager();

	bool SupportsTextureFormat(TextureFormat Format) const;

	void UploadTexture(CachedTexture* tex, const FTextureInfo& Info, bool masked);
	void UploadTextureRect(CachedTexture* tex, const FTextureInfo& Info, int x, int y, int w, int h);

	void SubmitUploads();

private:
	void UploadData(VkImage image, const FTextureInfo& Info, bool masked, TextureUploader* uploader);
	void UploadWhite(VkImage image);
	void WaitIfUploadBufferIsFull(int bytes);

	VulkanRenderDevice* renderer = nullptr;

	struct UploadedTexture
	{
		VkImage Image = {};
		int Index = 0;
		int Count = 0;
		bool PartialUpdate = false;
	};

	int UploadBufferPos = 0;
	Array<UploadedTexture> Uploads;
	Array<VkBufferImageCopy> ImageCopies;
	std::unordered_map<VkImage, Array<VkBufferImageCopy>> RectUploads;
};
