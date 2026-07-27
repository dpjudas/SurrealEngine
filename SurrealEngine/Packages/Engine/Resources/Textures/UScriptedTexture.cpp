
#include "Precomp.h"
#include "UScriptedTexture.h"
#include "Engine.h"
#include "VM/ScriptCall.h"
#include "Packages/Engine/Resources/UPalette.h"
#include "Packages/Engine/Resources/UFont.h"

void UScriptedTexture::Load(ObjectStream* stream)
{
	UTexture::Load(stream);

	UsedFormat = TextureFormat::P8;
	UsedMipmaps.resize(1);

	int width = GetInt("UClamp");
	int height = GetInt("VClamp");

	UnrealMipmap& mipmap = UsedMipmaps.front();
	mipmap.Width = width;
	mipmap.Height = height;
	mipmap.Data.resize((size_t)mipmap.Width * mipmap.Height);
	uint8_t* pixels = (uint8_t*)mipmap.Data.data();
	memset(pixels, 0, (size_t)width * height);
}

void UScriptedTexture::Save(PackageStreamWriter* stream)
{
	UTexture::Save(stream);
}

void UScriptedTexture::UpdateFrame()
{
	if (TextureModified)
		return;

	if (engine->LaunchInfo.ue1Version < 469)
	{
		double timeSinceLastUpdate = engine->TotalTime - LastUpdate;
		if (timeSinceLastUpdate < 1.0 / 80.0) // Rate limit to about 80 fps updates as 436 unrealscript code breaks if called too often
			return;
		LastUpdate = engine->TotalTime;
	}

	UTexture* sourceTex = SourceTexture();
	if (sourceTex)
	{
		Palette() = sourceTex->Palette();
		DrawTileP8(sourceTex, 0.0f, 0.0f, (float)UsedMipmaps.front().Width, (float)UsedMipmaps.front().Height, 0.0f, 0.0f, (float)sourceTex->UsedMipmaps.front().Width, (float)sourceTex->UsedMipmaps.front().Height);
	}

	UActor* actor = NotifyActor();
	if (actor)
		CallEvent(actor, "RenderTexture", { ExpressionValue::ObjectValue(this) });

	TextureModified = true;
}

void UScriptedTexture::DrawColoredText(float X, float Y, const std::string& Text, UFont* Font, const Color& FontColor)
{
	if (!Font)
		return;

	uint8_t color = Palette()->FindBestColor(FontColor);

	for (char c : Text)
	{
		FontGlyph glyph = Font->GetGlyph(c);
		if (!glyph.Texture)
			continue;

		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;
		DrawTileP8Color(glyph.Texture, X, Y, USize, VSize, StartU, StartV, USize, VSize, color);
		X += USize;
	}
}

void UScriptedTexture::DrawText(float X, float Y, const std::string& Text, UFont* Font)
{
	if (!Font)
		return;

	for (char c : Text)
	{
		FontGlyph glyph = Font->GetGlyph(c);
		if (!glyph.Texture)
			continue;

		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;
		DrawTileP8Masked(glyph.Texture, X, Y, USize, VSize, StartU, StartV, USize, VSize);
		X += USize;
	}
}

void UScriptedTexture::DrawTile(float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UTexture* Tex, bool bMasked)
{
	if (bMasked)
		DrawTileP8Masked(Tex, X, Y, XL, YL, U, V, UL, VL);
	else
		DrawTileP8(Tex, X, Y, XL, YL, U, V, UL, VL);
}

void UScriptedTexture::ReplaceTexture(UTexture* Tex)
{
	// What does this do? Nothing calls it in UT it seems.
}

void UScriptedTexture::TextSize(const std::string& Text, float& XL, float& YL, UFont* Font)
{
	if (!Font)
		return;

	int x = 0;
	int y = 0;
	for (char c : Text)
	{
		FontGlyph glyph = Font->GetGlyph(c);
		x += glyph.USize;
		y = std::max(y, glyph.VSize);
	}

	XL = (float)x;
	YL = (float)y;
}

void UScriptedTexture::DrawTileP8(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight)
{
	if (!texture || width <= 0.0f || height <= 0.0f || UsedFormat != TextureFormat::P8 || texture->UsedFormat != TextureFormat::P8)
		return;

	int swidth = texture->UsedMipmaps.front().Width;
	const uint8_t* src = (const uint8_t*)texture->UsedMipmaps.front().Data.data();

	int dwidth = UsedMipmaps.front().Width;
	int dheight = UsedMipmaps.front().Height;
	uint8_t* dest = (uint8_t*)UsedMipmaps.front().Data.data();

	int x0 = std::max((int)left, 0);
	int x1 = std::min((int)(left + width), dwidth);
	int y0 = std::max((int)top, 0);
	int y1 = std::min((int)(top + height), dheight);
	if (x1 <= x0 || y1 <= y0)
		return;

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint8_t* sline = src + ((int)vpix) * swidth;
		uint8_t* dline = dest + y * dwidth;

		for (int x = x0; x < x1; x++)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint8_t spixel = sline[(int)upix];
			dline[x] = spixel;
		}
	}
}

void UScriptedTexture::DrawTileP8Masked(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight)
{
	if (!texture || width <= 0.0f || height <= 0.0f || UsedFormat != TextureFormat::P8 || texture->UsedFormat != TextureFormat::P8)
		return;

	int swidth = texture->UsedMipmaps.front().Width;
	const uint8_t* src = (const uint8_t*)texture->UsedMipmaps.front().Data.data();

	int dwidth = UsedMipmaps.front().Width;
	int dheight = UsedMipmaps.front().Height;
	uint8_t* dest = (uint8_t*)UsedMipmaps.front().Data.data();

	int x0 = std::max((int)left, 0);
	int x1 = std::min((int)(left + width), dwidth);
	int y0 = std::max((int)top, 0);
	int y1 = std::min((int)(top + height), dheight);
	if (x1 <= x0 || y1 <= y0)
		return;

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint8_t* sline = src + ((int)vpix) * swidth;
		uint8_t* dline = dest + y * dwidth;

		for (int x = x0; x < x1; x++)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint8_t spixel = sline[(int)upix];
			if (spixel != 0)
				dline[x] = spixel;
		}
	}
}

void UScriptedTexture::DrawTileP8Color(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight, uint8_t color)
{
	if (!texture || width <= 0.0f || height <= 0.0f || UsedFormat != TextureFormat::P8 || texture->UsedFormat != TextureFormat::P8)
		return;

	int swidth = texture->UsedMipmaps.front().Width;
	const uint8_t* src = (const uint8_t*)texture->UsedMipmaps.front().Data.data();

	int dwidth = UsedMipmaps.front().Width;
	int dheight = UsedMipmaps.front().Height;
	uint8_t* dest = (uint8_t*)UsedMipmaps.front().Data.data();

	int x0 = std::max((int)left, 0);
	int x1 = std::min((int)(left + width), dwidth);
	int y0 = std::max((int)top, 0);
	int y1 = std::min((int)(top + height), dheight);
	if (x1 <= x0 || y1 <= y0)
		return;

	float uscale = uvwidth / width;
	float vscale = uvheight / height;

	for (int y = y0; y < y1; y++)
	{
		float vpix = v + vscale * (y + 0.5f - top);
		const uint8_t* sline = src + ((int)vpix) * swidth;
		uint8_t* dline = dest + y * dwidth;

		for (int x = x0; x < x1; x++)
		{
			float upix = u + uscale * (x + 0.5f - left);
			uint8_t spixel = sline[(int)upix];
			if (spixel != 0)
				dline[x] = color;
		}
	}
}
