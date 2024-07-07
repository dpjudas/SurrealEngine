
#include "Precomp.h"
#include "RenderDevice.h"
#include "Vulkan/VulkanRenderDevice.h"
#include "UObject/ULevel.h"
#include <zwidget/core/colorf.h>

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
	device->Lock(vec4(0.0f), vec4(0.0f), vec4(color.r, color.g, color.b, color.a));
}

void RenderDeviceCanvas::end()
{
	device->Unlock(true);
}

void RenderDeviceCanvas::begin3d()
{
	CheckFrame();
	device->ClearZ(&frame);
}

void RenderDeviceCanvas::end3d()
{
	CheckFrame();
	device->ClearZ(&frame);
}

std::unique_ptr<CanvasTexture> RenderDeviceCanvas::createTexture(int width, int height, const void* pixels, ImageFormat format)
{
	auto texture = std::make_unique<RenderDeviceTexture>();
	texture->Width = width;
	texture->Height = height;
	texture->Info.CacheID = (uint64_t)(ptrdiff_t)texture.get();
	texture->Info.Format = TextureFormat::ARGB8;
	texture->Info.USize = width;
	texture->Info.VSize = height;
	texture->Info.NumMips = 1;
	texture->Info.Mips = &texture->Mip;
	texture->Mip.Width = width;
	texture->Mip.Height = height;
	texture->Mip.Data.resize(width * height * 4);
	memcpy(texture->Mip.Data.data(), pixels, width * height * 4);
	return texture;
}

void RenderDeviceCanvas::drawLineAntialiased(float x0, float y0, float x1, float y1, Colorf color)
{
	CheckFrame();
	device->Draw2DLine(&frame, vec4(color.r, color.g, color.b, color.a), vec3(x0, y0, 1.0f), vec3(x1, y1, 1.0f));
}

void RenderDeviceCanvas::fillTile(float x, float y, float width, float height, Colorf color)
{
	CheckFrame();
	device->DrawTile(&frame, static_cast<RenderDeviceTexture*>(whiteTexture.get())->Info, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, vec4(color.r, color.g, color.b, color.a), vec4(0.0f), 0);
}

void RenderDeviceCanvas::drawTile(CanvasTexture* texture, float x, float y, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	CheckFrame();
	device->DrawTile(&frame, static_cast<RenderDeviceTexture*>(texture)->Info, x, y, width, height, u, v, uvwidth, uvheight, 1.0f, vec4(color.r, color.g, color.b, color.a), vec4(0.0f), 0);
}

void RenderDeviceCanvas::drawGlyph(CanvasTexture* texture, float x, float y, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	CheckFrame();
	device->DrawTile(&frame, static_cast<RenderDeviceTexture*>(texture)->Info, x, y, width, height, u, v, uvwidth, uvheight, 1.0f, vec4(color.r, color.g, color.b, color.a), vec4(0.0f), PF_SubpixelFont);
}

void RenderDeviceCanvas::CheckFrame()
{
	// To do: check if frame is up to date and update it if not
	// device->SetSceneNode(&frame);
}
