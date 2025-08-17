#pragma once

#include "D3D11TextureUploader.h"
#include <d3d11.h>

class D3D11RenderDevice;
class D3D11CachedTexture;
struct FTextureInfo;

class D3D11UploadManager
{
public:
	D3D11UploadManager(D3D11RenderDevice* renderer);
	~D3D11UploadManager();

	bool SupportsTextureFormat(TextureFormat Format) const;

	void UploadTexture(D3D11CachedTexture* tex, const FTextureInfo& Info, bool masked);
	void UploadTextureRect(D3D11CachedTexture* tex, const FTextureInfo& Info, int x, int y, int w, int h);

	void SubmitUploads();

	void UploadWhite(ID3D11Texture2D* image);

private:
	void UploadData(ID3D11Texture2D* image, const FTextureInfo& Info, bool masked, D3D11TextureUploader* uploader, int dummyMipmapCount, INT minSize);

	uint8_t* GetUploadBuffer(size_t size);

	D3D11RenderDevice* renderer = nullptr;
	std::vector<uint32_t> UploadBuffer;
};
