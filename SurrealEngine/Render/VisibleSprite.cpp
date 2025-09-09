
#include "Precomp.h"
#include "VisibleSprite.h"
#include "UObject/UClass.h"
#include "UObject/UTexture.h"
#include "UObject/UActor.h"
#include "Engine.h"
#include "RenderSubsystem.h"

void VisibleSprite::Draw(VisibleFrame* frame, UActor* actor)
{
	UTexture* texture = actor->Texture();
	const vec3& location = actor->Location();
	float drawscale = actor->DrawScale();
	int style = actor->Style();
	bool noSmooth = actor->bNoSmooth();

	engine->render->UpdateTexture(texture);

	if (actor->DrawType() == DT_SpriteAnimOnce)
	{
		float t = (1.0f - actor->LifeSpan() / actor->Class->GetDefaultObject<UActor>()->LifeSpan());
		int count = texture->GetAnimTextureCount();
		int index = (int)std::floor(clamp(t, 0.0f, 1.0f) * count);
		for (int i = 0; i < index; i++)
			texture = texture->AnimNext();
	}
	else
	{
		texture = texture->GetAnimTexture();
	}
	if (!texture)
		return;

	engine->render->UpdateTexture(texture);

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

	vec3 sideAxis = frame->ViewRotation.YAxis * (texwidth * drawscale);
	vec3 upAxis = frame->ViewRotation.ZAxis * (texheight * drawscale);

	vec3 color = clamp(actor->ScaleGlow(), 0.0f, 1.0f);

	GouraudVertex vertices[4];
	vertices[0].Point = location - sideAxis - upAxis;
	vertices[0].UV = { 0.0f, 0.0f };
	vertices[0].Light = color;
	vertices[0].Fog = { 0.0f };
	vertices[1].Point = location + sideAxis - upAxis;
	vertices[1].UV = { texwidth, 0.0f };
	vertices[1].Light = color;
	vertices[1].Fog = { 0.0f };
	vertices[2].Point = location + sideAxis + upAxis;
	vertices[2].UV = { texwidth, texheight };
	vertices[2].Light = color;
	vertices[2].Fog = { 0.0f };
	vertices[3].Point = location - sideAxis + upAxis;
	vertices[3].UV = { 0.0f, texheight };
	vertices[3].Light = color;
	vertices[3].Fog = { 0.0f };

	frame->Device->DrawGouraudPolygon(&frame->Frame, texinfo, vertices, 4, renderflags);

	engine->render->Stats.Actors++;
}
