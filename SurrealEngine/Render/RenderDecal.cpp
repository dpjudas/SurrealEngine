
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"

void RenderSubsystem::DrawDecals(FSceneNode* frame, BspNode* node)
{
	for (auto& leveldecal : node->Decals)
	{
		if (leveldecal.Decal->Texture())
		{
			UpdateTexture(leveldecal.Decal->Texture());

			UTexture* texture = leveldecal.Decal->Texture()->GetAnimTexture();
			UpdateTexture(texture);

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
				points[i].Point = leveldecal.Positions[i];
				points[i].UV = leveldecal.UVs[i];
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

			Device->DrawGouraudPolygon(frame, texinfo, points, 4, PF_Modulated);
		}
	}
}
