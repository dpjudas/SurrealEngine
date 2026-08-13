#pragma once

#include "GLHandles.h"

struct TextureInfo;
class UnrealMipmap;
struct TextureColor;
enum class TextureFormat : uint32_t;

class GLTextureUploader
{
public:
	GLTextureUploader(GLint internalformat, GLenum format, GLenum type) : Internalformat(internalformat), Format(format), Type(type) { }
	virtual ~GLTextureUploader() = default;

	virtual int GetUploadSize(int x, int y, int w, int h) = 0;
	virtual void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) = 0;

	GLint GetInternalformat() const { return Format; }
	GLenum GetFormat() const { return Format; }
	GLenum GetType() const { return Format; }

	static GLTextureUploader* GetUploader(TextureFormat format);

private:
	GLint Internalformat;
	GLenum Format;
	GLenum Type;
};

class GLTextureUploader_P8 : public GLTextureUploader
{
public:
	GLTextureUploader_P8() : GLTextureUploader(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;
};

class GLTextureUploader_RGB8 : public GLTextureUploader
{
public:
	GLTextureUploader_RGB8() : GLTextureUploader(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;
};

class GLTextureUploader_BGRA8_LM : public GLTextureUploader
{
public:
	GLTextureUploader_BGRA8_LM() : GLTextureUploader(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;
};

class GLTextureUploader_RGB10A2 : public GLTextureUploader
{
public:
	GLTextureUploader_RGB10A2() : GLTextureUploader(GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;
};

class GLTextureUploader_RGB10A2_UI : public GLTextureUploader
{
public:
	GLTextureUploader_RGB10A2_UI() : GLTextureUploader(GL_RGBA16UI, GL_RGBA, GL_UNSIGNED_SHORT) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;
};

class GLTextureUploader_RGB10A2_LM : public GLTextureUploader
{
public:
	GLTextureUploader_RGB10A2_LM() : GLTextureUploader(GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;
};

class GLTextureUploader_Simple : public GLTextureUploader
{
public:
	GLTextureUploader_Simple(GLint internalformat, GLenum format, GLenum type, int bytesPerPixel) : GLTextureUploader(internalformat, format, type), BytesPerPixel(bytesPerPixel) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;

private:
	int BytesPerPixel;
};

class GLTextureUploader_4x4Block : public GLTextureUploader
{
public:
	GLTextureUploader_4x4Block(GLint internalformat, GLenum format, GLenum type, int bytesPerBlock) : GLTextureUploader(internalformat, format, type), BytesPerBlock(bytesPerBlock) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;

private:
	int BytesPerBlock;
};

class GLTextureUploader_2DBlock : public GLTextureUploader
{
public:
	GLTextureUploader_2DBlock(GLint internalformat, GLenum format, GLenum type, int blockX, int blockY, int bytesPerBlock) : GLTextureUploader(internalformat, format, type), BlockX(blockX), BlockY(blockY), BytesPerBlock(bytesPerBlock) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, TextureColor* palette, bool masked) override;

private:
	int BlockX;
	int BlockY;
	int BytesPerBlock;
};
