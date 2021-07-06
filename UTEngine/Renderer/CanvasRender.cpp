
#include "Precomp.h"
#include "CanvasRender.h"
#include "Engine.h"
#include "Window/Window.h"
#include "Package/PackageManager.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"

CanvasRender::CanvasRender()
{
	bigfont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "BigFont"));
	largefont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "LargeFont"));
	medfont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "MedFont"));
	smallfont = UObject::Cast<UFont>(engine->packages->GetPackage("Engine")->GetUObject("Font", "SmallFont"));
}

void CanvasRender::DrawTile(FSceneNode* frame, UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags)
{
	RenderDevice* device = engine->window->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)Tex;
	texinfo.Texture = Tex;

	device->DrawTile(frame, texinfo, x, y, XL, YL, U, V, UL, VL, Z, color, fog, flags);
}

void CanvasRender::DrawFontTextWithShadow(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment)
{
	DrawFontText(frame, font, vec4(0.0f, 0.0f, 0.0f, color.a), x + 2 * 4, y + 2 * 4, text, alignment);
	DrawFontText(frame, font, color, x, y, text, alignment);
}

void CanvasRender::DrawFontText(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment)
{
	if (alignment != TextAlignment::left)
	{
		ivec2 textsize = GetFontTextSize(font, text);
		if (alignment == TextAlignment::center)
			x -= textsize.x / 2;
		else
			x -= textsize.x;
	}

	RenderDevice* device = engine->window->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)font->pages.front().Texture;
	texinfo.Texture = font->pages.front().Texture;

	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);

		int width = glyph.USize * 8;
		int height = glyph.VSize * 8;
		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;

		device->DrawTile(frame, texinfo, (float)x, (float)y, (float)width, (float)height, StartU, StartV, USize, VSize, 1.5f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

		x += width;
	}
}

ivec2 CanvasRender::GetFontTextSize(UFont* font, const std::string& text)
{
	int x = 0;
	int y = 0;
	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);

		x += glyph.USize * 8;
		y = std::max(y, glyph.VSize * 8);
	}
	return { x, y };
}
