
#include "Precomp.h"
#include "VisibleDecal.h"
#include "VisibleFrame.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"

void VisibleDecal::DrawDecals(VisibleFrame* frame, BspNode* node)
{
	for (auto& leveldecal : node->Decals)
	{
		if (leveldecal.Decal->Texture())
		{
			engine->render->UpdateTexture(leveldecal.Decal->Texture());

			UTexture* texture = leveldecal.Decal->Texture()->GetAnimTexture();
			engine->render->UpdateTexture(texture);

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

			vec3 depthOffset = -frame->ViewRotation.XAxis;

			int count = (int)leveldecal.Positions.size();
			GouraudVertex* points = engine->render->GetTempGouraudVertexBuffer(count);
			for (int i = 0; i < count; i++)
			{
				points[i].Light = vec3(1.0f);
				points[i].Point = leveldecal.Positions[i] + depthOffset;
				points[i].UV = leveldecal.UVs[i];
			}

			frame->Device->DrawGouraudPolygon(&frame->Frame, texinfo, points, count, PF_Modulated);
		}
	}
}
