#pragma once

#include <unordered_map>

struct TextureInfo;
class GLRenderDevice;
class GLCachedTexture;

class GLTextureManager
{
public:
	GLTextureManager(GLRenderDevice* renderer);
	~GLTextureManager();

	void UpdateTextureRect(TextureInfo* info, int x, int y, int w, int h);
	GLCachedTexture* GetTexture(TextureInfo* info, bool masked);

	void ClearCache();
	int GetTexturesInCache() { return (int)(TextureCache[0].size() + TextureCache[1].size()); }

	GLCachedTexture* GetNullTexture()
	{
		if (NullTexture)
			return NullTexture.get();
		return CreateNullTexture();
	}

private:
	void UploadTexture(TextureInfo* info, bool masked, GLCachedTexture* tex);
	GLCachedTexture* CreateNullTexture();

	GLCachedTexture* GetFromCache(int masked, uint64_t cacheID);

	GLRenderDevice* renderer = nullptr;
	std::unordered_map<uint64_t, std::unique_ptr<GLCachedTexture>> TextureCache[2];
	std::unique_ptr<GLCachedTexture> NullTexture;

	std::pair<uint64_t, GLCachedTexture*> LastTextureResult[2];
};
