#pragma once

#include <d3d11.h>

struct FTextureInfo;
class UnrealMipmap;
struct FColor;
enum class TextureFormat : uint32_t;

class D3D11TextureUploader
{
public:
	D3D11TextureUploader(DXGI_FORMAT format) : Format(format) { }
	virtual ~D3D11TextureUploader() = default;

	virtual int GetUploadSize(int x, int y, int w, int h) = 0;
	virtual void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) = 0;

	DXGI_FORMAT GetDxgiFormat() const { return Format; }

	static D3D11TextureUploader* GetUploader(TextureFormat format);

private:
	DXGI_FORMAT Format;
};

class D3D11TextureUploader_P8 : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_P8() : D3D11TextureUploader(DXGI_FORMAT_R8G8B8A8_UNORM) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;
};

class D3D11TextureUploader_RGB8 : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_RGB8() : D3D11TextureUploader(DXGI_FORMAT_R8G8B8A8_UNORM) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;
};

class D3D11TextureUploader_BGRA8_LM : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_BGRA8_LM() : D3D11TextureUploader(DXGI_FORMAT_R8G8B8A8_UNORM) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;
};

class D3D11TextureUploader_RGB10A2 : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_RGB10A2() : D3D11TextureUploader(DXGI_FORMAT_R16G16B16A16_UNORM) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;
};

class D3D11TextureUploader_RGB10A2_UI : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_RGB10A2_UI() : D3D11TextureUploader(DXGI_FORMAT_R16G16B16A16_UINT) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;
};

class D3D11TextureUploader_RGB10A2_LM : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_RGB10A2_LM() : D3D11TextureUploader(DXGI_FORMAT_R16G16B16A16_UNORM) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;
};

class D3D11TextureUploader_Simple : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_Simple(DXGI_FORMAT format, int bytesPerPixel) : D3D11TextureUploader(format), BytesPerPixel(bytesPerPixel) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;

private:
	int BytesPerPixel;
};

class D3D11TextureUploader_4x4Block : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_4x4Block(DXGI_FORMAT format, int bytesPerBlock) : D3D11TextureUploader(format), BytesPerBlock(bytesPerBlock) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;

private:
	int BytesPerBlock;
};

class D3D11TextureUploader_2DBlock : public D3D11TextureUploader
{
public:
	D3D11TextureUploader_2DBlock(DXGI_FORMAT format, int blockX, int blockY, int bytesPerBlock) : D3D11TextureUploader(format), BlockX(blockX), BlockY(blockY), BytesPerBlock(bytesPerBlock) { }

	int GetUploadSize(int x, int y, int w, int h) override;
	void UploadRect(void* dst, UnrealMipmap* mip, int x, int y, int w, int h, FColor* palette, bool masked) override;

private:
	int BlockX;
	int BlockY;
	int BytesPerBlock;
};
