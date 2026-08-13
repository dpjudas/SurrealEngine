
#include "Precomp.h"
#include "GLUploadManager.h"
#include "GLRenderDevice.h"
#include "GLCachedTexture.h"

GLUploadManager::GLUploadManager(GLRenderDevice* renderer) : renderer(renderer)
{
}

GLUploadManager::~GLUploadManager()
{
}

bool GLUploadManager::SupportsTextureFormat(TextureFormat Format) const
{
	return GLTextureUploader::GetUploader(Format);
}

void GLUploadManager::UploadTexture(GLCachedTexture* tex, const TextureInfo& Info, bool masked)
{
	int width = Info.USize;
	int height = Info.VSize;
	int mipcount = Info.NumMips;

	GLTextureUploader* uploader = GLTextureUploader::GetUploader(Info.Format);

	if ((uint32_t)Info.USize > 16384 || (uint32_t)Info.VSize > 16384 || !uploader)
	{
		width = 1;
		height = 1;
		mipcount = 1;
		uploader = nullptr;
	}

	GLint internalFormat = GL_RGBA8;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;
	if (uploader)
	{
		internalFormat = uploader->GetInternalformat();
		if (uploader->GetFormat() != 0)
		{
			format = uploader->GetFormat();
			type = uploader->GetType();
		}
	}

	// Base texture must use complete 4x4 compression blocks in Direct3D 11 or some drivers crash.
	// It is unclear if some OpenGL drivers have the same problem or not.
	// 
	// We fix this by creating 1 or 2 additional mipmap levels that are empty.

	int minSize = Info.Format == TextureFormat::BC1 || (Info.Format >= TextureFormat::BC2 && Info.Format <= TextureFormat::BC6H) ? 4 : 0;

	if (!tex->Texture)
	{
		if (width < minSize || height < minSize)
		{
			if (width == 1 || height == 1)
			{
				width *= 4;
				height *= 4;
				mipcount += 2;
				tex->DummyMipmapCount = 2;
			}
			else
			{
				width *= 2;
				height *= 2;
				mipcount += 1;
				tex->DummyMipmapCount = 1;
			}
		}

		tex->Texture = std::make_shared<GLTexture2D>();
		glBindTexture(GL_TEXTURE_2D, tex->Texture->Handle);
		int mipwidth = width;
		int mipheight = height;
		for (int miplevel = 0; miplevel < mipcount; miplevel)
		{
			glTexImage2D(GL_TEXTURE_2D, miplevel, internalFormat, mipwidth, mipheight, 0, format, type, nullptr);
			ThrowIfGLError("UploadTexture failed");
			mipwidth = std::max(mipwidth >> 1, 1);
			mipheight = std::max(mipheight >> 1, 1);
		}
	}

	if (uploader)
		UploadData(tex->Texture.get(), Info, masked, uploader, tex->DummyMipmapCount, minSize);
	else
		UploadWhite(tex->Texture.get());

	renderer->Stats.Uploads++;
}

void GLUploadManager::UploadTextureRect(GLCachedTexture* tex, const TextureInfo& Info, int x, int y, int w, int h)
{
	GLTextureUploader* uploader = GLTextureUploader::GetUploader(Info.Format);
	if (!uploader || Info.NumMips < 1 || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > Info.Mips[0].Width || y + h > Info.Mips[0].Height || Info.Mips[0].Data.empty())
		return;

	size_t pixelsSize = uploader->GetUploadSize(x, y, w, h);
	pixelsSize = (pixelsSize + 15) / 16 * 16; // memory alignment

	uint8_t* data = GetUploadBuffer(pixelsSize);
	uploader->UploadRect(data, &Info.Mips[0], x, y, w, h, Info.Palette, false);

	// To do: do we need this?
	// int pitch = uploader->GetUploadSize(0, 0, w, 1);
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / bytesPerPixel);

	glBindTexture(GL_TEXTURE_2D, tex->Texture->Handle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, uploader->GetFormat(), uploader->GetType(), data);
	ThrowIfGLError("UploadTextureRect failed");

	renderer->Stats.RectUploads++;
}

void GLUploadManager::UploadData(GLTexture2D* image, const TextureInfo& Info, bool masked, GLTextureUploader* uploader, int dummyMipmapCount, int minSize)
{
	for (int level = 0; level < Info.NumMips; level++)
	{
		UnrealMipmap* Mip = &Info.Mips[level];
		if (!Mip->Data.empty())
		{
			uint32_t mipwidth = std::max(Mip->Width, minSize);
			uint32_t mipheight = std::max(Mip->Height, minSize);

			int mipsize = uploader->GetUploadSize(0, 0, mipwidth, mipheight);
			mipsize = (mipsize + 15) / 16 * 16; // memory alignment

			auto data = (uint32_t*)GetUploadBuffer(mipsize);
			uploader->UploadRect(data, Mip, 0, 0, mipwidth, mipheight, Info.Palette, masked);

			// To do: do we need this?
			// int pitch = uploader->GetUploadSize(0, 0, mipwidth, 1);
			// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			// glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / bytesPerPixel);

			glBindTexture(GL_TEXTURE_2D, image->Handle);
			glTexSubImage2D(GL_TEXTURE_2D, level + dummyMipmapCount, 0, 0, mipwidth, mipheight, uploader->GetFormat(), uploader->GetType(), data);
			ThrowIfGLError("UploadData failed");
		}
	}
}

void GLUploadManager::UploadWhite(GLTexture2D* image)
{
	auto data = (uint32_t*)GetUploadBuffer(sizeof(uint32_t));
	data[0] = 0xffffffff;
	glBindTexture(GL_TEXTURE_2D, image->Handle);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	ThrowIfGLError("UploadWhite failed");
}

uint8_t* GLUploadManager::GetUploadBuffer(size_t size)
{
	// Using uint32_t vector for dword alignment
	size = (size + 3) / 4;
	if (UploadBuffer.size() < size)
		UploadBuffer.resize(size);
	return (uint8_t*)UploadBuffer.data();
}
