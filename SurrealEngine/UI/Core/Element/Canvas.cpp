
#include "Precomp.h"
#include "Canvas.h"
#include "Rect.h"
#include "Colorf.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"

class CanvasTexture
{
public:
	TextureFormat format;
	UnrealMipmap mipmap;
};

class RenderDeviceCanvas : public Canvas
{
public:
	RenderDeviceCanvas(RenderDevice* renderDevice);
	~RenderDeviceCanvas();

	void begin() override;
	void end() override;

	Point getOrigin() override;
	void setOrigin(const Point& origin) override;

	void pushClip(const Rect& box) override;
	void popClip() override;

	void fillRect(const Rect& box, const Colorf& color) override;
	void drawText(const Point& pos, const Colorf& color, const std::string& text) override;
	Rect measureText(const std::string& text) override;

	void drawTile(CanvasTexture* texture, double x, double y, double width, double height, double u, double v, double uvwidth, double uvheight, double z, vec4 color, uint32_t flags);

	std::unique_ptr<CanvasTexture> createTexture(int width, int height, const void* pixels);

	RenderDevice* renderDevice = nullptr;

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
}

RenderDeviceCanvas::~RenderDeviceCanvas()
{
}

void RenderDeviceCanvas::begin()
{
	frame.XB = 0;
	frame.YB = 0;
	frame.X = renderDevice->Viewport->SizeX;
	frame.Y = renderDevice->Viewport->SizeY;
	frame.FX = (float)renderDevice->Viewport->SizeX;
	frame.FY = (float)renderDevice->Viewport->SizeY;
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

	renderDevice->Lock(vec4(0.0f), vec4(0.0f), vec4(0.0f, 0.0f, 0.2f, 1.0f));
	renderDevice->SetSceneNode(&frame);
}

void RenderDeviceCanvas::end()
{
	renderDevice->Unlock(true);
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
	clipStack.push_back(box);
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

void RenderDeviceCanvas::drawText(const Point& pos, const Colorf& color, const std::string& text)
{
}

Rect RenderDeviceCanvas::measureText(const std::string& text)
{
	return Rect::xywh(0.0, 0.0, text.length() * 10.0, 18.0);
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
