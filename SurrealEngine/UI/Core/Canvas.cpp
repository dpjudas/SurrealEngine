
#include "Precomp.h"
#include "Canvas.h"
#include "Rect.h"
#include "Colorf.h"
#include "File.h"
#include "UTF8Reader.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"
#include <schrift/schrift.h>

class CanvasTexture
{
public:
	TextureFormat format;
	UnrealMipmap mipmap;
};

class CanvasGlyph
{
public:
	SFT_Glyph id;
	SFT_GMetrics metrics;

	double u = 0.0;
	double v = 0.0;
	double uvwidth = 0.0f;
	double uvheight = 0.0f;
	std::shared_ptr<CanvasTexture> texture;
};

class CanvasFont
{
public:
	CanvasFont(const std::string& fontname, double height) : fontname(fontname), height(height)
	{
		data = File::read_all_bytes("C:\\Windows\\Fonts\\segoeui.ttf");
		loadFont(data.data(), data.size());

		try
		{
			if (sft_lmetrics(&sft, &textmetrics) < 0)
				throw std::runtime_error("Could not get truetype font metrics");
		}
		catch (...)
		{
			sft_freefont(sft.font);
			throw;
		}
	}

	~CanvasFont()
	{
		sft_freefont(sft.font);
		sft.font = nullptr;
	}

	CanvasGlyph* getGlyph(uint32_t utfchar)
	{
		auto& glyph = glyphs[utfchar];
		if (glyph)
			return glyph.get();

		glyph = std::make_unique<CanvasGlyph>();

		if (sft_lookup(&sft, utfchar, &glyph->id) < 0)
			return glyph.get();

		if (sft_gmetrics(&sft, glyph->id, &glyph->metrics) < 0)
			return glyph.get();

		glyph->metrics.advanceWidth /= 3.0;
		glyph->metrics.leftSideBearing /= 3.0;

		if (glyph->metrics.minWidth <= 0 || glyph->metrics.minHeight <= 0)
			return glyph.get();

		int w = (glyph->metrics.minWidth + 3) & ~3;
		int h = glyph->metrics.minHeight;

		int destwidth = (w + 2) / 3;

		auto texture = std::make_shared<CanvasTexture>();
		texture->format = TextureFormat::BGRA8;
		texture->mipmap.Width = destwidth;
		texture->mipmap.Height = h;
		texture->mipmap.Data.resize(destwidth * h * 4);
		uint32_t* dest = (uint32_t*)texture->mipmap.Data.data();

		std::unique_ptr<uint8_t[]> grayscalebuffer(new uint8_t[w * h]);
		uint8_t* grayscale = grayscalebuffer.get();

		SFT_Image img = {};
		img.width = w;
		img.height = h;
		img.pixels = grayscale;
		if (sft_render(&sft, glyph->id, img) < 0)
			return glyph.get();

		for (int y = 0; y < h; y++)
		{
			uint8_t* sline = grayscale + y * w;
			uint32_t* dline = dest + y * destwidth;
			for (int x = 2; x < w; x += 3)
			{
				uint32_t red = sline[x - 2];
				uint32_t green = sline[x - 1];
				uint32_t blue = sline[x];
				uint32_t alpha = (red | green | blue) ? 255 : 0;

				uint32_t maxval = std::max(std::max(red, green), blue);
				red = std::max(red, maxval / 5);
				green = std::max(green, maxval / 5);
				blue = std::max(blue, maxval / 5);

				dline[x / 3] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
			if (w % 3 == 1)
			{
				uint32_t red = sline[w - 1];
				uint32_t green = 0;
				uint32_t blue = 0;
				uint32_t alpha = (red | green | blue) ? 255 : 0;

				uint32_t maxval = std::max(std::max(red, green), blue);
				red = std::max(red, maxval / 5);
				green = std::max(green, maxval / 5);
				blue = std::max(blue, maxval / 5);

				dline[(w - 1) / 3] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
			else if (w % 3 == 2)
			{
				uint32_t red = sline[w - 2];
				uint32_t green = sline[w - 1];
				uint32_t blue = 0;
				uint32_t alpha = (red | green | blue) ? 255 : 0;

				uint32_t maxval = std::max(std::max(red, green), blue);
				red = std::max(red, maxval / 5);
				green = std::max(green, maxval / 5);
				blue = std::max(blue, maxval / 5);

				dline[(w - 1) / 3] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
		}

		glyph->u = 0.0;
		glyph->v = 0.0;
		glyph->uvwidth = destwidth;
		glyph->uvheight = h;
		glyph->texture = std::move(texture);

		return glyph.get();
	}

	std::string fontname;
	double height = 0.0;

	SFT_LMetrics textmetrics = {};
	std::unordered_map<uint32_t, std::unique_ptr<CanvasGlyph>> glyphs;

private:
	void loadFont(const void* data, size_t size)
	{
		sft.xScale = height * 3;
		sft.yScale = height;
		sft.flags = SFT_DOWNWARD_Y;
		sft.font = sft_loadmem(data, size);
	}

	SFT sft = {};
	std::vector<uint8_t> data;
};

class RenderDeviceCanvas : public Canvas
{
public:
	RenderDeviceCanvas(RenderDevice* renderDevice);
	~RenderDeviceCanvas();

	void begin(const Colorf& color) override;
	void end() override;

	void begin3d() override;
	void end3d() override;

	Point getOrigin() override;
	void setOrigin(const Point& origin) override;

	void pushClip(const Rect& box) override;
	void popClip() override;

	void fillRect(const Rect& box, const Colorf& color) override;
	void line(const Point& p0, const Point& p1, const Colorf& color) override;

	void drawText(const Point& pos, const Colorf& color, const std::string& text) override;
	Rect measureText(const std::string& text) override;
	VerticalTextPosition verticalTextAlign() override;

	void drawLineUnclipped(const Point& p0, const Point& p1, const Colorf& color);
	void drawTile(CanvasTexture* texture, double x, double y, double width, double height, double u, double v, double uvwidth, double uvheight, double z, vec4 color, uint32_t flags);

	std::unique_ptr<CanvasTexture> createTexture(int width, int height, const void* pixels);

	RenderDevice* renderDevice = nullptr;

	std::unique_ptr<CanvasFont> font;
	std::unique_ptr<CanvasTexture> whiteTexture;

	Point origin;
	double uiscale = 1.0f;
	std::vector<Rect> clipStack;
	FSceneNode frame;
};

RenderDeviceCanvas::RenderDeviceCanvas(RenderDevice* renderDevice) : renderDevice(renderDevice)
{
	uint32_t white = 0xffffffff;
	whiteTexture = createTexture(1, 1, &white);
	font = std::make_unique<CanvasFont>("Segoe UI", 13.0*uiscale);
}

RenderDeviceCanvas::~RenderDeviceCanvas()
{
}

void RenderDeviceCanvas::begin(const Colorf& color)
{
	frame.XB = 0;
	frame.YB = 0;
	frame.X = renderDevice->Viewport->GetPixelWidth();
	frame.Y = renderDevice->Viewport->GetPixelHeight();
	frame.FX = (float)renderDevice->Viewport->GetPixelWidth();
	frame.FY = (float)renderDevice->Viewport->GetPixelHeight();
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.ObjectToWorld = mat4::identity();
	frame.WorldToView = mat4::identity();
	frame.FovAngle = 90.0f;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);

	renderDevice->Lock(vec4(0.0f), vec4(0.0f), vec4(color.r, color.g, color.b, 1.0f));
	renderDevice->SetSceneNode(&frame);
}

void RenderDeviceCanvas::end()
{
	renderDevice->Unlock(true);
}

void RenderDeviceCanvas::begin3d()
{
	renderDevice->ClearZ(&frame);
}

void RenderDeviceCanvas::end3d()
{
	renderDevice->SetSceneNode(&frame);
	renderDevice->ClearZ(&frame);
}

Point RenderDeviceCanvas::getOrigin()
{
	return origin;
}

void RenderDeviceCanvas::setOrigin(const Point& newOrigin)
{
	origin = newOrigin;
}

void RenderDeviceCanvas::pushClip(const Rect& box)
{
	if (!clipStack.empty())
	{
		const Rect& clip = clipStack.back();

		double x0 = box.x + origin.x;
		double y0 = box.y + origin.y;
		double x1 = x0 + box.width;
		double y1 = y0 + box.height;

		x0 = std::max(x0, clip.x);
		y0 = std::max(y0, clip.y);
		x1 = std::min(x1, clip.x + clip.width);
		y1 = std::min(y1, clip.y + clip.height);

		if (x0 < x1 && y0 < y1)
			clipStack.push_back(Rect::ltrb(x0, y0, x1, y1));
		else
			clipStack.push_back(Rect::xywh(0.0, 0.0, 0.0, 0.0));
	}
	else
	{
		clipStack.push_back(box);
	}
}

void RenderDeviceCanvas::popClip()
{
	clipStack.pop_back();
}

void RenderDeviceCanvas::fillRect(const Rect& box, const Colorf& color)
{
	vec4 premultcolor = { color.r * color.a, color.g * color.a, color.b * color.a, color.a };
	drawTile(whiteTexture.get(), origin.x + box.x, origin.y + box.y, box.width, box.height, 0.0, 0.0, 1.0, 1.0, 1.0, premultcolor, PF_Highlighted);
}

void RenderDeviceCanvas::line(const Point& p0, const Point& p1, const Colorf& color)
{
	double x0 = origin.x + p0.x;
	double y0 = origin.y + p0.y;
	double x1 = origin.x + p1.x;
	double y1 = origin.y + p1.y;

	if (clipStack.empty())// || (clipStack.back().contains({ x0, y0 }) && clipStack.back().contains({ x1, y1 })))
	{
		drawLineUnclipped({ x0, y0 }, { x1, y1 }, color);
	}
	else
	{
		const Rect& clip = clipStack.back();

		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		if (x1 < clip.x || x0 >= clip.x + clip.width)
			return;

		// Clip left edge
		if (x0 < clip.x)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dx) < 0.0001)
				return;
			y0 = y0 + (clip.x - x0) * dy / dx;
			x0 = clip.x;
		}

		// Clip right edge
		if (x1 > clip.x + clip.width)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dx) < 0.0001)
				return;
			y1 = y1 + (clip.x + clip.width - x1) * dy / dx;
			x1 = clip.x + clip.width;
		}

		if (y0 > y1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		if (y1 < clip.y || y0 >= clip.y + clip.height)
			return;

		// Clip top edge
		if (y0 < clip.y)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dy) < 0.0001)
				return;
			x0 = x0 + (clip.y - y0) * dx / dy;
			y0 = clip.y;
		}

		// Clip bottom edge
		if (y1 > clip.y + clip.height)
		{
			double dx = x1 - x0;
			double dy = y1 - y0;
			if (std::abs(dy) < 0.0001)
				return;
			x1 = x1 + (clip.y + clip.height - y1) * dx / dy;
			y1 = clip.y + clip.height;
		}

		x0 = clamp(x0, clip.x, clip.x + clip.width);
		x1 = clamp(x1, clip.x, clip.x + clip.width);
		y0 = clamp(y0, clip.y, clip.y + clip.height);
		y1 = clamp(y1, clip.y, clip.y + clip.height);

		if (x0 != x1 || y0 != y1)
			drawLineUnclipped({ x0, y0 }, { x1, y1 }, color);
	}
}

void RenderDeviceCanvas::drawLineUnclipped(const Point& p0, const Point& p1, const Colorf& color)
{
	vec4 premultcolor = { color.r * color.a, color.g * color.a, color.b * color.a, color.a };
	renderDevice->Draw2DLine(&frame, premultcolor, vec3((float)p0.x, (float)p0.y, 1.0f), vec3((float)p1.x, (float)p1.y, 1.0f));
}

void RenderDeviceCanvas::drawText(const Point& pos, const Colorf& color, const std::string& text)
{
	double x = origin.x + std::round(pos.x * uiscale);
	double y = origin.y + std::round(pos.y * uiscale);

	vec4 color4(color.r, color.g, color.b, 1.0f);

	UTF8Reader reader(text.data(), text.size());
	while (!reader.is_end())
	{
		CanvasGlyph* glyph = font->getGlyph(reader.character());
		if (!glyph->texture)
		{
			glyph = font->getGlyph(32);
		}

		if (glyph->texture)
		{
			double gx = std::round(x + glyph->metrics.leftSideBearing);
			double gy = std::round(y + glyph->metrics.yOffset);
			drawTile(glyph->texture.get(), std::round(gx) / uiscale, std::round(gy) / uiscale, glyph->uvwidth / uiscale, glyph->uvheight / uiscale, glyph->u, glyph->v, glyph->uvwidth, glyph->uvheight, 1.0f, color4, PF_SubpixelFont);
		}

		x += std::round(glyph->metrics.advanceWidth);
		reader.next();
	}
}

Rect RenderDeviceCanvas::measureText(const std::string& text)
{
	double x = 0.0;
	double y = font->textmetrics.ascender + font->textmetrics.descender;

	UTF8Reader reader(text.data(), text.size());
	while (!reader.is_end())
	{
		CanvasGlyph* glyph = font->getGlyph(reader.character());
		if (!glyph->texture)
		{
			glyph = font->getGlyph(32);
		}

		if (glyph->texture)
		{
			x += glyph->metrics.advanceWidth;
		}

		reader.next();
	}

	return Rect::xywh(0.0, 0.0, x, y);
}

VerticalTextPosition RenderDeviceCanvas::verticalTextAlign()
{
	VerticalTextPosition align;
	align.top = 0.0f;
	align.baseline = font->textmetrics.lineGap * 0.5f + font->textmetrics.ascender;
	align.bottom = font->textmetrics.ascender + font->textmetrics.descender + font->textmetrics.lineGap;
	return align;
}

void RenderDeviceCanvas::drawTile(CanvasTexture* texture, double x, double y, double width, double height, double u, double v, double uvwidth, double uvheight, double z, vec4 color, uint32_t flags)
{
	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)texture;
	texinfo.Format = texture->format;
	texinfo.Mips = &texture->mipmap;
	texinfo.NumMips = 1;
	texinfo.USize = texture->mipmap.Width;
	texinfo.VSize = texture->mipmap.Height;
	renderDevice->DrawTile(&frame, texinfo, (float)(x * uiscale), (float)(y * uiscale), (float)(width * uiscale), (float)(height * uiscale), (float)u, (float)v, (float)uvwidth, (float)uvheight, (float)z, color, vec4(0.0f), flags);
}

std::unique_ptr<CanvasTexture> RenderDeviceCanvas::createTexture(int width, int height, const void* pixels)
{
	auto texture = std::make_unique<CanvasTexture>();
	texture->format = TextureFormat::BGRA8;
	texture->mipmap.Width = width;
	texture->mipmap.Height = height;
	texture->mipmap.Data.resize(width * height * 4);
	memcpy(texture->mipmap.Data.data(), pixels, width * height * 4);
	return texture;
}

/////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Canvas> Canvas::create(RenderDevice* renderDevice)
{
	return std::make_unique<RenderDeviceCanvas>(renderDevice);
}
