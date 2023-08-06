
#include "Precomp.h"
#include "Canvas.h"
#include "Rect.h"
#include "RenderDevice/RenderDevice.h"

class RenderDeviceCanvas : public Canvas
{
public:
	RenderDeviceCanvas(RenderDevice* renderDevice);
	~RenderDeviceCanvas();

	Point getOrigin() override;
	void setOrigin(const Point& origin) override;

	void pushClip(const Rect& box) override;
	void popClip() override;

	void fillRect(const Rect& box, const Colorf& color) override;
	void drawText(const Point& pos, const Colorf& color, const std::string& text) override;
	Rect measureText(const std::string& text) override;

	RenderDevice* renderDevice = nullptr;

	Point origin;
	double dpiscale = 1.0f;
	std::vector<Rect> clipStack;
};

RenderDeviceCanvas::RenderDeviceCanvas(RenderDevice* renderDevice) : renderDevice(renderDevice)
{
}

RenderDeviceCanvas::~RenderDeviceCanvas()
{
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
}

void RenderDeviceCanvas::drawText(const Point& pos, const Colorf& color, const std::string& text)
{
}

Rect RenderDeviceCanvas::measureText(const std::string& text)
{
	return Rect();
}
