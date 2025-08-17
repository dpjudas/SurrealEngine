#pragma once

#include <unordered_map>

struct FTextureInfo;
class D3D11RenderDevice;
class D3D11CachedTexture;

class D3D11TextureManager
{
public:
	D3D11TextureManager(D3D11RenderDevice* renderer);
	~D3D11TextureManager();

	void UpdateTextureRect(FTextureInfo* info, int x, int y, int w, int h);
	D3D11CachedTexture* GetTexture(FTextureInfo* info, bool masked);

	void ClearCache();
	int GetTexturesInCache() { return (int)(TextureCache[0].size() + TextureCache[1].size()); }

	D3D11CachedTexture* GetNullTexture()
	{
		if (NullTexture)
			return NullTexture.get();
		return CreateNullTexture();
	}

private:
	void UploadTexture(FTextureInfo* info, bool masked, D3D11CachedTexture* tex);
	D3D11CachedTexture* CreateNullTexture();

	D3D11CachedTexture* GetFromCache(int masked, uint64_t cacheID);

	D3D11RenderDevice* renderer = nullptr;
	std::unordered_map<uint64_t, std::unique_ptr<D3D11CachedTexture>> TextureCache[2];
	std::unique_ptr<D3D11CachedTexture> NullTexture;

	std::pair<uint64_t, D3D11CachedTexture*> LastTextureResult[2];
};
