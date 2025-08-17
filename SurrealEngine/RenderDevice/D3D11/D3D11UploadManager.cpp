
#include "Precomp.h"
#include "D3D11UploadManager.h"
#include "D3D11RenderDevice.h"
#include "D3D11CachedTexture.h"

D3D11UploadManager::D3D11UploadManager(D3D11RenderDevice* renderer) : renderer(renderer)
{
}

D3D11UploadManager::~D3D11UploadManager()
{
}

bool D3D11UploadManager::SupportsTextureFormat(TextureFormat Format) const
{
	return D3D11TextureUploader::GetUploader(Format);
}

void D3D11UploadManager::UploadTexture(D3D11CachedTexture* tex, const FTextureInfo& Info, bool masked)
{
	int width = Info.USize;
	int height = Info.VSize;
	int mipcount = Info.NumMips;

	D3D11TextureUploader* uploader = D3D11TextureUploader::GetUploader(Info.Format);

	if ((uint32_t)Info.USize > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || (uint32_t)Info.VSize > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || !uploader)
	{
		width = 1;
		height = 1;
		mipcount = 1;
		uploader = nullptr;
	}

	DXGI_FORMAT format = uploader ? uploader->GetDxgiFormat() : DXGI_FORMAT_R8G8B8A8_UNORM;

	// Base texture must use complete 4x4 compression blocks in Direct3D 11 or some drivers crash.
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

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = mipcount;
		texDesc.ArraySize = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		//texDesc.MiscFlags = tex->DummyMipmapCount > 0 ? D3D11_RESOURCE_MISC_RESOURCE_CLAMP : 0;
		HRESULT result = renderer->Device->CreateTexture2D(&texDesc, nullptr, tex->Texture.TypedInitPtr());
		ThrowIfFailed(result, "CreateTexture2D(GameTexture) failed");

		//if (tex->DummyMipmapCount > 0)
		//	renderer->Context->SetResourceMinLOD(tex->Texture, (float)tex->DummyMipmapCount);

		//D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		//viewDesc.Format = format;
		//viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		//viewDesc.Texture2D.MostDetailedMip = tex->DummyMipmapCount;
		//viewDesc.Texture2D.MipLevels = -1;
		result = renderer->Device->CreateShaderResourceView(tex->Texture, nullptr/*&viewDesc*/, tex->View.TypedInitPtr());
		ThrowIfFailed(result, "CreateShaderResourceView(GameTexture) failed");
	}

	if (uploader)
		UploadData(tex->Texture, Info, masked, uploader, tex->DummyMipmapCount, minSize);
	else
		UploadWhite(tex->Texture);

	renderer->Stats.Uploads++;
}

void D3D11UploadManager::UploadTextureRect(D3D11CachedTexture* tex, const FTextureInfo& Info, int x, int y, int w, int h)
{
	D3D11TextureUploader* uploader = D3D11TextureUploader::GetUploader(Info.Format);
	if (!uploader || Info.NumMips < 1 || x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > Info.Mips[0].Width || y + h > Info.Mips[0].Height || Info.Mips[0].Data.empty())
		return;

	size_t pixelsSize = uploader->GetUploadSize(x, y, w, h);
	pixelsSize = (pixelsSize + 15) / 16 * 16; // memory alignment

	uint8_t* data = GetUploadBuffer(pixelsSize);
	uploader->UploadRect(data, &Info.Mips[0], x, y, w, h, Info.Palette, false);

	UINT pitch = uploader->GetUploadSize(0, 0, w, 1);

	D3D11_BOX box = {};
	box.left = x;
	box.top = y;
	box.right = x + w;
	box.bottom = y + h;
	box.back = 1;
	renderer->Context->UpdateSubresource(tex->Texture, D3D11CalcSubresource(0, 0, Info.NumMips), &box, data, pitch, 0);

	renderer->Stats.RectUploads++;
}

void D3D11UploadManager::UploadData(ID3D11Texture2D* image, const FTextureInfo& Info, bool masked, D3D11TextureUploader* uploader, int dummyMipmapCount, INT minSize)
{
	for (INT level = 0; level < Info.NumMips; level++)
	{
		UnrealMipmap* Mip = &Info.Mips[level];
		if (!Mip->Data.empty())
		{
			uint32_t mipwidth = std::max(Mip->Width, minSize);
			uint32_t mipheight = std::max(Mip->Height, minSize);

			INT mipsize = uploader->GetUploadSize(0, 0, mipwidth, mipheight);
			mipsize = (mipsize + 15) / 16 * 16; // memory alignment

			auto data = (uint32_t*)GetUploadBuffer(mipsize);
			uploader->UploadRect(data, Mip, 0, 0, mipwidth, mipheight, Info.Palette, masked);

			UINT pitch = uploader->GetUploadSize(0, 0, mipwidth, 1);

			D3D11_BOX box = {};
			box.right = mipwidth;
			box.bottom = mipheight;
			box.back = 1;

			UINT subresource = D3D11CalcSubresource(level + dummyMipmapCount, 0, Info.NumMips + dummyMipmapCount);
			renderer->Context->UpdateSubresource(image, subresource, &box, data, pitch, 0);
		}
	}
}

void D3D11UploadManager::UploadWhite(ID3D11Texture2D* image)
{
	auto data = (uint32_t*)GetUploadBuffer(sizeof(uint32_t));
	data[0] = 0xffffffff;

	D3D11_BOX box = {};
	box.right = 1;
	box.bottom = 1;
	box.back = 1;
	renderer->Context->UpdateSubresource(image, D3D11CalcSubresource(0, 0, 1), &box, data, 4, 0);
}

uint8_t* D3D11UploadManager::GetUploadBuffer(size_t size)
{
	// Using uint32_t vector for dword alignment
	size = (size + 3) / 4;
	if (UploadBuffer.size() < size)
		UploadBuffer.resize(size);
	return (uint8_t*)UploadBuffer.data();
}
