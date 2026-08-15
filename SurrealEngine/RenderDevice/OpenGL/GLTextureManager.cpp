
#include "Precomp.h"
#include "GLTextureManager.h"
#include "GLRenderDevice.h"
#include "GLCachedTexture.h"

GLTextureManager::GLTextureManager(GLRenderDevice* renderer) : renderer(renderer)
{
}

GLTextureManager::~GLTextureManager()
{
	ClearCache();
}

GLCachedTexture* GLTextureManager::GetFromCache(int masked, uint64_t cacheID)
{
	if (LastTextureResult[masked].first == cacheID && LastTextureResult[masked].second)
		return LastTextureResult[masked].second;

	LastTextureResult[masked].first = cacheID;
	LastTextureResult[masked].second = TextureCache[masked][cacheID].get();

	return LastTextureResult[masked].second;
}

void GLTextureManager::UpdateTextureRect(TextureInfo* info, int x, int y, int w, int h)
{
	GLCachedTexture* tex = GetFromCache(0, info->CacheID);
	if (tex)
	{
		renderer->Uploads->UploadTextureRect(tex, *info, x, y, w, h);
		info->bRealtimeChanged = 0;
	}
}

GLCachedTexture* GLTextureManager::GetTexture(TextureInfo* info, bool masked)
{
	if (!info)
		return GetNullTexture();

	if (info->Texture && (info->Texture->PolyFlags() & PF_Masked))
		masked = true;

	if (info->Format != TextureFormat::P8)
		masked = false;

	GLCachedTexture* tex = GetFromCache((int)masked, info->CacheID);
	if (!tex)
	{
		std::unique_ptr<GLCachedTexture>& tex2 = TextureCache[(int)masked][info->CacheID];
		tex2.reset(new GLCachedTexture());
		tex = tex2.get();

		renderer->Uploads->UploadTexture(tex, *info, masked);
	}
	else
	{
		if (info->bRealtimeChanged)
			UploadTexture(info, masked, tex);
	}

	float uscale = info->UScale;
	float vscale = info->VScale;
	tex->UScale = uscale;
	tex->VScale = vscale;
	tex->PanX = info->Pan.x;
	tex->PanY = info->Pan.y;
	tex->UMult = 1.0f / (uscale * info->USize);
	tex->VMult = 1.0f / (vscale * info->VSize);

	return tex;
}

void GLTextureManager::UploadTexture(TextureInfo* info, bool masked, GLCachedTexture* tex)
{
	if (info->bRealtimeChanged)
	{
		info->bRealtimeChanged = 0;
		renderer->Uploads->UploadTexture(tex, *info, masked);
	}
}

void GLTextureManager::ClearCache()
{
	for (auto& cache : TextureCache)
	{
		cache.clear();
	}
	for (auto& texture : LastTextureResult)
		texture = {};
}

GLCachedTexture* GLTextureManager::CreateNullTexture()
{
	NullTexture.reset(new GLCachedTexture());
	uint32_t white = 0xffffffff;
	NullTexture->Texture = std::make_shared<GLTexture2D>();
	glBindTexture(GL_TEXTURE_2D, NullTexture->Texture->Handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, &white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	return NullTexture.get();
}
