
#include "Precomp.h"
#include "DecalRender.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/ULevel.h"
#include "Math/hsb.h"
#include "Engine.h"
#include "Window/Window.h"
#include "UTRenderer.h"

void DecalRender::DrawDecals(FSceneNode* frame)
{
	RenderDevice* device = engine->window->GetRenderDevice();

	for (auto& leveldecal : engine->Level->Decals)
	{
		if (leveldecal->Decal->Texture())
		{
			UTexture* texture = leveldecal->Decal->Texture()->GetAnimTexture();

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)texture;
			texinfo.Texture = texture;
			texinfo.Format = texinfo.Texture->ActualFormat;
			texinfo.Mips = texinfo.Texture->Mipmaps.data();
			texinfo.NumMips = (int)texinfo.Texture->Mipmaps.size();
			texinfo.USize = texinfo.Texture->USize();
			texinfo.VSize = texinfo.Texture->VSize();
			if (texinfo.Texture->Palette())
				texinfo.Palette = (FColor*)texinfo.Texture->Palette()->Colors.data();

			GouraudVertex points[4];
			for (int i = 0; i < 4; i++)
			{
				points[i].Light = vec3(1.0f);
				points[i].Point = leveldecal->Positions[i];
				points[i].UV = leveldecal->UVs[i];
			}

			/*int style = leveldecal->Decal->Style();
			uint32_t renderflags = PF_TwoSided;
			if (style == 3)
				renderflags |= PF_Translucent;
			else if (style == 4)
				renderflags |= PF_Modulated;
			if (leveldecal->Decal->bNoSmooth())
				renderflags |= PF_NoSmooth;
			if (texture->bMasked())
				renderflags |= PF_Masked;*/

			device->DrawGouraudPolygon(frame, texinfo, points, 4, PF_Modulated);
		}
	}
}
