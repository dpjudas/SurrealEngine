
#include "Precomp.h"
#include "RenderDevice.h"
#include "LauncherSettings.h"
#include "Vulkan/VulkanRenderDevice.h"
#ifdef WIN32
#include "D3D11/D3D11RenderDevice.h"
#endif
#include "UObject/ULevel.h"
#include <zwidget/core/colorf.h>
#include <zwidget/core/widget.h>

RenderDevice::RenderDevice()
{
	const auto& settings = LauncherSettings::Get();
	UseVSync = settings.RenderDevice.UseVSync;
	Hdr = settings.RenderDevice.Hdr;
	HdrScale = settings.RenderDevice.HdrScale;
	Bloom = settings.RenderDevice.Bloom;
	BloomAmount = settings.RenderDevice.BloomAmount;
	AntialiasMode = (int)settings.RenderDevice.Antialias;
	GammaMode = (int)settings.RenderDevice.Gamma;
	LightMode = (int)settings.RenderDevice.Light;
	GammaCorrectScreenshots = settings.RenderDevice.GammaCorrectScreenshots;
	UseDebugLayer = settings.RenderDevice.UseDebugLayer;
}

std::unique_ptr<RenderDevice> RenderDevice::Create(Widget* viewport, RenderAPI renderAPI)
{
	if (renderAPI == RenderAPI::Vulkan)
	{
		return std::make_unique<VulkanRenderDevice>(viewport);
	}
#ifdef WIN32
	else if (renderAPI == RenderAPI::D3D11)
	{
		return std::make_unique<D3D11RenderDevice>(viewport);
	}
#endif
	else
	{
		Exception::Throw("Render API not available on this platform");
	}
}

////////////////////////////////////////////////////////////////////////////

RenderDeviceCanvas::RenderDeviceCanvas(RenderDevice* device) : device(device)
{
}

void RenderDeviceCanvas::begin(const Colorf& color)
{
	device->Lock(vec4(0.0f), vec4(0.0f), vec4(color.r, color.g, color.b, color.a), nullptr, nullptr);

	frame.XB = 0;
	frame.YB = 0;
	frame.X = device->Viewport->GetNativePixelWidth();
	frame.Y = device->Viewport->GetNativePixelHeight();
	frame.FX = (float)device->Viewport->GetNativePixelWidth();
	frame.FY = (float)device->Viewport->GetNativePixelHeight();
	frame.FX2 = (float)device->Viewport->GetNativePixelWidth() * 0.5f;
	frame.FY2 = (float)device->Viewport->GetNativePixelHeight() * 0.5f;
	frame.Viewport = device->Viewport;
	frame.FovAngle = 90.0f;
	frame.ObjectToWorld = mat4::identity();
	frame.WorldToView = mat4::identity();
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	device->SetSceneNode(&frame);
}

void RenderDeviceCanvas::end()
{
	device->Unlock(true);
}

void RenderDeviceCanvas::begin3d()
{
	device->ClearZ();
}

void RenderDeviceCanvas::end3d()
{
	device->SetSceneNode(&frame);
	device->ClearZ();
}

std::unique_ptr<CanvasTexture> RenderDeviceCanvas::createTexture(int width, int height, const void* pixels, ImageFormat format)
{
	auto texture = std::make_unique<RenderDeviceTexture>();
	texture->Width = width;
	texture->Height = height;
	texture->Info.CacheID = (uint64_t)(ptrdiff_t)texture.get();
	texture->Info.Format = TextureFormat::BGRA8;
	texture->Info.USize = width;
	texture->Info.VSize = height;
	texture->Info.NumMips = 1;
	texture->Info.Mips = &texture->Mip;
	texture->Mip.Width = width;
	texture->Mip.Height = height;
	texture->Mip.Data.resize(width * height * 4);
	if (format == ImageFormat::B8G8R8A8)
	{
		memcpy(texture->Mip.Data.data(), pixels, width * height * 4);
	}
	else // Convert it to BGRA8
	{
		const uint32_t* src = (const uint32_t*)pixels;
		uint32_t* dest = (uint32_t*)texture->Mip.Data.data();
		int count = width * height;
		for (int i = 0; i < count; i++)
		{
			uint32_t a = (src[i] >> 24) & 0xff;
			uint32_t b = (src[i] >> 16) & 0xff;
			uint32_t g = (src[i] >> 8) & 0xff;
			uint32_t r = src[i] & 0xff;
			dest[i] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}
	return texture;
}

void RenderDeviceCanvas::drawLineAntialiased(float x0, float y0, float x1, float y1, Colorf color)
{
	device->Draw2DLine(&frame, vec4(color.r, color.g, color.b, color.a), 0, vec3(x0, y0, 1.0f), vec3(x1, y1, 1.0f));
}

void RenderDeviceCanvas::fillTile(float x, float y, float width, float height, Colorf color)
{
	device->DrawTile(&frame, static_cast<RenderDeviceTexture*>(whiteTexture.get())->Info, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, vec4(color.r, color.g, color.b, color.a), vec4(0.0f), PF_Highlighted);
}

void RenderDeviceCanvas::drawTile(CanvasTexture* texture, float x, float y, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	device->DrawTile(&frame, static_cast<RenderDeviceTexture*>(texture)->Info, x, y, width, height, u, v, uvwidth, uvheight, 1.0f, vec4(color.r, color.g, color.b, color.a), vec4(0.0f), PF_Highlighted);
}

void RenderDeviceCanvas::drawGlyph(CanvasTexture* texture, float x, float y, float width, float height, float u, float v, float uvwidth, float uvheight, Colorf color)
{
	device->DrawTile(&frame, static_cast<RenderDeviceTexture*>(texture)->Info, x, y, width, height, u, v, uvwidth, uvheight, 1.0f, vec4(color.r, color.g, color.b, color.a), vec4(0.0f), PF_SubpixelFont);
}
