#pragma once

#include <zvulkan/vulkanobjects.h>
#include <unordered_map>
#include "SceneTextures.h"

struct FTextureInfo;
class VulkanRenderDevice;
class CachedTexture;

class TextureManager
{
public:
	TextureManager(VulkanRenderDevice* renderer);
	~TextureManager();

	void UpdateTextureRect(FTextureInfo* info, int x, int y, int w, int h);
	CachedTexture* GetTexture(FTextureInfo* info, bool masked);

	void ClearCache();

	std::unique_ptr<VulkanImage> NullTexture;
	std::unique_ptr<VulkanImageView> NullTextureView;

	std::unique_ptr<VulkanImage> DitherImage;
	std::unique_ptr<VulkanImageView> DitherImageView;

	std::unique_ptr<SceneTextures> Scene;

	int GetTexturesInCache() { return (int)(TextureCache[0].size() + TextureCache[1].size()); }

private:
	void CreateNullTexture();
	void CreateDitherTexture();

	VulkanRenderDevice* renderer = nullptr;
	std::unordered_map<uint64_t, std::unique_ptr<CachedTexture>> TextureCache[2];
};
