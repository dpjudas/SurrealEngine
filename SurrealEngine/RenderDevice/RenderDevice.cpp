
#include "Precomp.h"
#include "RenderDevice.h"
#include "Vulkan/VulkanRenderDevice.h"

std::unique_ptr<RenderDevice> RenderDevice::Create(GameWindow* viewport, std::shared_ptr<VulkanSurface> surface)
{
	return std::make_unique<VulkanRenderDevice>(viewport, surface);
}

////////////////////////////////////////////////////////////////////////////

RenderDeviceCanvas::RenderDeviceCanvas(RenderDevice* device) : device(device)
{
}

void RenderDeviceCanvas::begin(const Colorf& color)
{
}

void RenderDeviceCanvas::end()
{
}

void RenderDeviceCanvas::begin3d()
{
}

void RenderDeviceCanvas::end3d()
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
}

void RenderDeviceCanvas::popClip()
{
}

void RenderDeviceCanvas::fillRect(const Rect& box, const Colorf& color)
{
}

void RenderDeviceCanvas::line(const Point& p0, const Point& p1, const Colorf& color)
{
}

void RenderDeviceCanvas::drawText(const Point& pos, const Colorf& color, const std::string& text)
{
}

Rect RenderDeviceCanvas::measureText(const std::string& text)
{
	return Rect();
}

VerticalTextPosition RenderDeviceCanvas::verticalTextAlign()
{
	return VerticalTextPosition();
}

void RenderDeviceCanvas::drawText(const std::shared_ptr<Font>& font, const Point& pos, const std::string& text, const Colorf& color)
{
}

void RenderDeviceCanvas::drawTextEllipsis(const std::shared_ptr<Font>& font, const Point& pos, const Rect& clipBox, const std::string& text, const Colorf& color)
{
}

Rect RenderDeviceCanvas::measureText(const std::shared_ptr<Font>& font, const std::string& text)
{
	return Rect();
}

FontMetrics RenderDeviceCanvas::getFontMetrics(const std::shared_ptr<Font>& font)
{
	return FontMetrics();
}

int RenderDeviceCanvas::getCharacterIndex(const std::shared_ptr<Font>& font, const std::string& text, const Point& hitPoint)
{
	return 0;
}

void RenderDeviceCanvas::drawImage(const std::shared_ptr<Image>& image, const Point& pos)
{
}
