
#include "Precomp.h"
#include "VisibleCorona.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"

void VisibleCorona::Draw(VisibleFrame* frame)
{
	if (light->bCorona() && light->Skin() && !engine->Level->Collision.TraceAnyHit(light->Location(), engine->CameraLocation, nullptr, false, true, true))
	{
		vec4 pos = frame->Frame.WorldToView * frame->Frame.ObjectToWorld * vec4(light->Location(), 1.0f);
		if (pos.z >= 1.0f)
		{
			vec4 clip = frame->Frame.Projection * pos;

			float x = frame->Frame.FX2 + clip.x / clip.w * frame->Frame.FX2;
			float y = frame->Frame.FY2 + clip.y / clip.w * frame->Frame.FY2;
			float z = 2.0f;

			float width = (float)light->Skin()->UsedMipmaps.front().Width;
			float height = (float)light->Skin()->UsedMipmaps.front().Height;
			float size = light->DrawScale() * frame->Frame.FX * 0.8f;

			vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), 255/*light->LightBrightness()*/);

			engine->render->UpdateTexture(light->Skin());

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)light->Skin();
			texinfo.Texture = light->Skin()->GetAnimTexture();
			texinfo.Format = texinfo.Texture->UsedFormat;
			texinfo.Mips = texinfo.Texture->UsedMipmaps.data();
			texinfo.NumMips = (int)texinfo.Texture->UsedMipmaps.size();
			texinfo.USize = texinfo.Texture->USize();
			texinfo.VSize = texinfo.Texture->VSize();
			if (texinfo.Texture->Palette())
				texinfo.Palette = (FColor*)texinfo.Texture->Palette()->Colors.data();

			engine->render->UpdateTexture(texinfo.Texture);

			frame->Device->DrawTile(&frame->Frame, texinfo, x - size * 0.5f, y - size * 0.5f, size, size, 0.0f, 0.0f, width, height, z, vec4(lightcolor, 1.0f), vec4(0.0f), PF_Translucent);
		}
	}
}
