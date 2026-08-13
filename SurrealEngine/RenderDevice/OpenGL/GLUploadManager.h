#pragma once

#include "GLTextureUploader.h"

class GLRenderDevice;
class GLCachedTexture;
struct TextureInfo;

class GLUploadManager
{
public:
	GLUploadManager(GLRenderDevice* renderer);
	~GLUploadManager();

	bool SupportsTextureFormat(TextureFormat Format) const;

	void UploadTexture(GLCachedTexture* tex, const TextureInfo& Info, bool masked);
	void UploadTextureRect(GLCachedTexture* tex, const TextureInfo& Info, int x, int y, int w, int h);

	void SubmitUploads();

	void UploadWhite(GLTexture2D* image);

private:
	void UploadData(GLTexture2D* image, const TextureInfo& Info, bool masked, GLTextureUploader* uploader, int dummyMipmapCount, int minSize);

	uint8_t* GetUploadBuffer(size_t size);

	GLRenderDevice* renderer = nullptr;
	std::vector<uint32_t> UploadBuffer;
};
