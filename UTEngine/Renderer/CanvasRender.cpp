
#include "Precomp.h"
#include "CanvasRender.h"
#include "Engine.h"
#include "Window/Window.h"
#include "Package/PackageManager.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"
#include "UTRenderer.h"

CanvasRender::CanvasRender()
{
	bigfont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "BigFont"));
	largefont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "LargeFont"));
	medfont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "MedFont"));
	smallfont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "SmallFont"));
}

void CanvasRender::DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags)
{
	RenderDevice* device = engine->window->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)Tex;
	texinfo.Texture = Tex;

	int frameSizeX = (int)(engine->window->SizeX / (float)engine->renderer->uiscale);
	int frameSizeY = (int)(engine->window->SizeY / (float)engine->renderer->uiscale);

	if (XL <= 0.0f || YL <= 0.0f || x + XL <= 0.0f || y + YL <= 0.0f || x >= frameSizeX || y >= frameSizeY)
		return;

	if (x < 0.f)
	{
		float t = x * UL / XL;
		U -= t;
		UL += t;
		XL += x;
		x = 0.0f;
	}

	if (y < 0.0f)
	{
		float t = y * VL / YL;
		V -= t;
		VL += t;
		YL += y;
		y = 0.0f;
	}

	if (XL > frameSizeX - x)
	{
		UL += (frameSizeX - x - XL) * UL / XL;
		XL = frameSizeX - x;
	}

	if (YL > frameSizeY - y)
	{
		VL += (frameSizeY - y - YL) * VL / YL;
		YL = frameSizeY - y;
	}

	int uiscale = engine->renderer->uiscale;
	device->DrawTile(&SceneFrame, texinfo, x * uiscale, y * uiscale, XL * uiscale, YL * uiscale, U, V, UL, VL, Z, color, fog, flags);
}

void CanvasRender::DrawFontTextWithShadow(UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment)
{
	DrawFontText(font, vec4(0.0f, 0.0f, 0.0f, color.a), x + 1, y + 1, text, alignment);
	DrawFontText(font, color, x, y, text, alignment);
}

void CanvasRender::DrawFontText(UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment)
{
	if (alignment != TextAlignment::left)
	{
		ivec2 textsize = GetTextSize(font, text);
		if (alignment == TextAlignment::center)
			x -= textsize.x / 2;
		else
			x -= textsize.x;
	}

	RenderDevice* device = engine->window->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)font->pages.front().Texture;
	texinfo.Texture = font->pages.front().Texture;

	int uiscale = engine->renderer->uiscale;

	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);

		int width = glyph.USize;
		int height = glyph.VSize;
		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;

		device->DrawTile(&SceneFrame, texinfo, (float)x * uiscale, (float)y * uiscale, (float)width * uiscale, (float)height * uiscale, StartU, StartV, USize, VSize, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

		x += width;
	}
}

void CanvasRender::DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t flags)
{
	RenderDevice* device = engine->window->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)font->pages.front().Texture;
	texinfo.Texture = font->pages.front().Texture;

	int uiscale = engine->renderer->uiscale;

	FontCharacter uglyph = font->GetGlyph('_');
	int uwidth = uglyph.USize;
	int uheight = uglyph.VSize;
	float uStartU = (float)uglyph.StartU;
	float uStartV = (float)uglyph.StartV;
	float uUSize = (float)uglyph.USize;
	float uVSize = (float)uglyph.VSize;

	for (char c : text)
	{
		// To do: word wrap
		// To do: SpaceX and SpaceY also affects DrawText

		if (c == '\n')
		{
			curX = 0;
			curY += curYL;
			curYL = 0;
		}
		else
		{
			FontCharacter glyph = font->GetGlyph(c);

			int width = glyph.USize;
			int height = glyph.VSize;
			float StartU = (float)glyph.StartU;
			float StartV = (float)glyph.StartV;
			float USize = (float)glyph.USize;
			float VSize = (float)glyph.VSize;

			device->DrawTile(&SceneFrame, texinfo, (orgX + curX) * uiscale, (float)(orgY + curY) * uiscale, (float)width * uiscale, (float)height * uiscale, StartU, StartV, USize, VSize, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

			curX += width;
			curYL = std::max(curYL, (float)glyph.VSize);
		}
	}

	if (newlineAtEnd)
	{
		curX = 0;
		curY += curYL;
		curYL = 0;
	}
}

void CanvasRender::DrawTextClipped(UFont* font, vec4 color, int x, int y, const std::string& text, uint32_t flags, bool checkHotKey, float clipX)
{
	RenderDevice* device = engine->window->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)font->pages.front().Texture;
	texinfo.Texture = font->pages.front().Texture;

	int uiscale = engine->renderer->uiscale;

	FontCharacter uglyph = font->GetGlyph('_');
	int uwidth = uglyph.USize;
	int uheight = uglyph.VSize;
	float uStartU = (float)uglyph.StartU;
	float uStartV = (float)uglyph.StartV;
	float uUSize = (float)uglyph.USize;
	float uVSize = (float)uglyph.VSize;

	bool foundAmpersand = false;
	int maxY = 0;
	for (char c : text)
	{
		if (checkHotKey && c == '&' && !foundAmpersand)
		{
			foundAmpersand = true;
		}
		else if (foundAmpersand && c != '&')
		{
			foundAmpersand = false;

			FontCharacter glyph = font->GetGlyph(c);
			if (x + glyph.USize > (int)clipX)
				break;
			int width = glyph.USize;
			int height = glyph.VSize;
			float StartU = (float)glyph.StartU;
			float StartV = (float)glyph.StartV;
			float USize = (float)glyph.USize;
			float VSize = (float)glyph.VSize;
			device->DrawTile(&SceneFrame, texinfo, (float)x * uiscale, (float)y * uiscale, (float)width * uiscale, (float)height * uiscale, StartU, StartV, USize, VSize, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);
			device->DrawTile(&SceneFrame, texinfo, (float)(x + (width - uwidth) / 2) * uiscale, (float)y * uiscale, (float)uwidth * uiscale, (float)uheight * uiscale, uStartU, uStartV, uUSize, uVSize, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

			x += std::max(width, uwidth);
			maxY = std::max(maxY, glyph.VSize);
		}
		else
		{
			foundAmpersand = false;

			FontCharacter glyph = font->GetGlyph(c);
			if (x + glyph.USize > (int)clipX)
				break;

			int width = glyph.USize;
			int height = glyph.VSize;
			float StartU = (float)glyph.StartU;
			float StartV = (float)glyph.StartV;
			float USize = (float)glyph.USize;
			float VSize = (float)glyph.VSize;

			device->DrawTile(&SceneFrame, texinfo, (float)x * uiscale, (float)y * uiscale, (float)width * uiscale, (float)height * uiscale, StartU, StartV, USize, VSize, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

			x += width;
			maxY = std::max(maxY, glyph.VSize);
		}
	}
}

ivec2 CanvasRender::GetTextClippedSize(UFont* font, const std::string& text, float clipX)
{
	int x = 0;
	int y = 0;
	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);
		if (x + glyph.USize > (int)clipX)
			break;
		x += glyph.USize;
		y = std::max(y, glyph.VSize);
	}
	return { x, y };
}

ivec2 CanvasRender::GetTextSize(UFont* font, const std::string& text)
{
	int x = 0;
	int y = 0;
	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);
		x += glyph.USize;
		y = std::max(y, glyph.VSize);
	}
	return { x, y };
}
