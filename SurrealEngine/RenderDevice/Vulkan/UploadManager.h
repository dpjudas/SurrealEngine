#pragma once

#include "TextureUploader.h"
#include <unordered_map>

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

	void ClearCache();

private:
	void UploadData(CachedTexture* tex, const FTextureInfo& Info, bool masked, TextureUploader* uploader);
	void UploadWhite(CachedTexture* tex);
	void WaitIfUploadBufferIsFull(int bytes);
	void AddPendingUpload(CachedTexture* tex, const VkBufferImageCopy& region, bool isPartial);

	VulkanRenderDevice* renderer = nullptr;

	int UploadBufferPos = 0;
	std::vector<CachedTexture*> PendingUploads;
};
