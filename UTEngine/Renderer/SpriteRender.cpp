
#include "Precomp.h"
#include "SpriteRender.h"
#include "UObject/UTexture.h"
#include "UObject/ULevel.h"
#include "RenderDevice/RenderDevice.h"
#include "Math/quaternion.h"
#include "Engine.h"
#include "Viewport/Viewport.h"
#include "UTRenderer.h"

void SpriteRender::DrawSprite(FSceneNode* frame, UTexture* texture, const vec3& location, const Rotator& rotation, float drawscale)
{
	auto device = engine->viewport->GetRenderDevice();

	engine->renderer->Textures.insert(texture);

	FTextureInfo texinfo;
	texinfo.Texture = texture;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;
	texinfo.bRealtimeChanged = texture->TextureModified;
	if (texture->TextureModified)
		texture->TextureModified = false;

	float texwidth = (float)texture->Mipmaps.front().Width;
	float texheight = (float)texture->Mipmaps.front().Height;

	drawscale *= 0.5f;

	vec3 xaxis = { texwidth * drawscale, 0.0f, 0.0f };
	vec3 yaxis = { 0.0f, 0.0f, texheight * drawscale };

	quaternion viewrotation = inverse(normalize(quaternion::euler(radians(-engine->Camera.Pitch), radians(-engine->Camera.Roll), radians(-engine->Camera.Yaw), EulerOrder::yxz)));
	xaxis = viewrotation * xaxis;
	yaxis = viewrotation * yaxis;

	GouraudVertex vertices[4];
	vertices[0].Point = location - xaxis - yaxis;
	vertices[0].UV = { 0.0f, 0.0f };
	vertices[0].Light = { 1.0f };
	vertices[1].Point = location + xaxis - yaxis;
	vertices[1].UV = { texwidth, 0.0f };
	vertices[1].Light = { 1.0f };
	vertices[2].Point = location + xaxis + yaxis;
	vertices[2].UV = { texwidth, texheight };
	vertices[2].Light = { 1.0f };
	vertices[3].Point = location - xaxis + yaxis;
	vertices[3].UV = { 0.0f, texheight };
	vertices[3].Light = { 1.0f };
	device->DrawGouraudPolygon(frame, texinfo.Texture ? &texinfo : nullptr, vertices, 4, PF_Translucent); // To do: use the Style property for the polyflags
}
