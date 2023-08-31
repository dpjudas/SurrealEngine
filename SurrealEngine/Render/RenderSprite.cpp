
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "UObject/UClass.h"

void RenderSubsystem::DrawSprite(FSceneNode* frame, UActor* actor)
{
	UTexture* texture = actor->Texture();
	const vec3& location = actor->Location();
	float drawscale = actor->DrawScale();
	int style = actor->Style();
	bool noSmooth = actor->bNoSmooth();

	UpdateTexture(texture);

	if (actor->DrawType() == DT_SpriteAnimOnce)
	{
		float t = (1.0f - actor->LifeSpan() / static_cast<UActor*>(actor->Class->GetDefaultObject())->LifeSpan());
		int count = texture->GetAnimTextureCount();
		int index = (int)std::floor(clamp(t, 0.0f, 1.0f) * count);
		for (int i = 0; i < index; i++)
			texture = texture->AnimNext();
	}
	else
	{
		texture = texture->GetAnimTexture();
	}

	UpdateTexture(texture);

	FTextureInfo texinfo;
	texinfo.Texture = texture;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;
	texinfo.bRealtimeChanged = texture->TextureModified;
	if (texture->TextureModified)
		texture->TextureModified = false;
	texinfo.Format = texinfo.Texture->ActualFormat;
	texinfo.Mips = texinfo.Texture->Mipmaps.data();
	texinfo.NumMips = (int)texinfo.Texture->Mipmaps.size();
	texinfo.USize = texinfo.Texture->USize();
	texinfo.VSize = texinfo.Texture->VSize();
	if (texinfo.Texture->Palette())
		texinfo.Palette = (FColor*)texinfo.Texture->Palette()->Colors.data();

	float texwidth = (float)texture->Mipmaps.front().Width;
	float texheight = (float)texture->Mipmaps.front().Height;

	uint32_t renderflags = PF_TwoSided;
	if (style == 3)
		renderflags |= PF_Translucent;
	else if (style == 4)
		renderflags |= PF_Modulated;
	if (noSmooth)
		renderflags |= PF_NoSmooth;
	if (texture->bMasked())
		renderflags |= PF_Masked;

	renderflags |= texture->PolyFlags();
	if (renderflags & PF_Invisible)
		return;

	drawscale *= 0.5f;

	vec3 sideAxis = Scene.ViewRotation.YAxis * (texwidth * drawscale);
	vec3 upAxis = Scene.ViewRotation.ZAxis * (texheight * drawscale);
	vec3 offsetlocation = location - Scene.ViewRotation.XAxis * 30.0f;

	vec3 color = clamp(actor->ScaleGlow(), 0.0f, 1.0f);

	GouraudVertex vertices[4];
	vertices[0].Point = offsetlocation - sideAxis - upAxis;
	vertices[0].UV = { 0.0f, 0.0f };
	vertices[0].Light = color;
	vertices[0].Fog = { 0.0f };
	vertices[1].Point = offsetlocation + sideAxis - upAxis;
	vertices[1].UV = { texwidth, 0.0f };
	vertices[1].Light = color;
	vertices[1].Fog = { 0.0f };
	vertices[2].Point = offsetlocation + sideAxis + upAxis;
	vertices[2].UV = { texwidth, texheight };
	vertices[2].Light = color;
	vertices[2].Fog = { 0.0f };
	vertices[3].Point = offsetlocation - sideAxis + upAxis;
	vertices[3].UV = { 0.0f, texheight };
	vertices[3].Light = color;
	vertices[3].Fog = { 0.0f };

	Device->DrawGouraudPolygon(frame, texinfo, vertices, 4, renderflags);
}
