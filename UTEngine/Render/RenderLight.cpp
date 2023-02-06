
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "Math/hsb.h"

FTextureInfo RenderSubsystem::GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet, UZoneInfo* zoneActor, UModel* model)
{
	if (surface.LightMap < 0)
		return {};

	auto level = engine->Level;
	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	auto& lmtexture = Light.lmtextures[surface.LightMap];
	if (!lmtexture)
	{
		lmtexture = CreateLightmapTexture(lmindex, surface, zoneActor, model);
	}

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)&surface;
	texinfo.Format = lmtexture->Format;
	texinfo.Mips = &lmtexture->Mip;
	texinfo.NumMips = 1;
	texinfo.USize = texinfo.Mips[0].Width;
	texinfo.VSize = texinfo.Mips[0].Height;
	texinfo.Pan = { lmindex.PanX, lmindex.PanY };
	texinfo.UScale = lmindex.UScale;
	texinfo.VScale = lmindex.VScale;
	return texinfo;
}

std::unique_ptr<LightmapTexture> RenderSubsystem::CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface, UZoneInfo* zoneActor, UModel* model)
{
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;

	UnrealMipmap lmmip;
	lmmip.Width = width;
	lmmip.Height = height;
	lmmip.Data.resize((size_t)lmmip.Width * lmmip.Height * 4);

	if (lmindex.LightActors >= 0)
	{
		const vec3& N = model->Vectors[surface.vNormal];

		FSurfaceFacet facet;
		facet.MapCoords.Origin = model->Points[surface.pBase];
		facet.MapCoords.XAxis = model->Vectors[surface.vTextureU];
		facet.MapCoords.YAxis = model->Vectors[surface.vTextureV];

		float UDot = dot(facet.MapCoords.XAxis, facet.MapCoords.Origin);
		float VDot = dot(facet.MapCoords.YAxis, facet.MapCoords.Origin);
		float LMUPan = UDot + lmindex.PanX - 0.5f * lmindex.UScale;
		float LMVPan = VDot + lmindex.PanY - 0.5f * lmindex.VScale;
		float LMUMult = 1.0f / lmindex.UScale;
		float LMVMult = 1.0f / lmindex.VScale;

		vec3 p[3] =
		{
			facet.MapCoords.Origin,
			facet.MapCoords.Origin + facet.MapCoords.XAxis,
			facet.MapCoords.Origin + facet.MapCoords.YAxis
		};

		vec2 uv[3];
		for (int j = 0; j < 3; j++)
		{
			uv[j] =
			{
				(dot(facet.MapCoords.XAxis, p[j]) - LMUPan) * LMUMult,
				(dot(facet.MapCoords.YAxis, p[j]) - LMVPan) * LMVMult
			};
		}

		float leftDX = uv[2].x - uv[0].x;
		float leftDY = uv[2].y - uv[0].y;
		float leftStep = leftDX / leftDY;
		float rightDX = uv[2].x - uv[1].x;
		float rightDY = uv[2].y - uv[1].y;
		float rightStep = rightDX / rightDY;

		std::vector<vec3> lightcolors;
		lightcolors.resize((size_t)width * height, hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness()) * 0.25f);

		const uint8_t* bits = model->LightBits.data() + lmindex.DataOffset;
		int lmpitch = (width + 7) / 8;

		bool isSpecialLit = (surface.PolyFlags & PF_SpecialLit) == PF_SpecialLit;

		for (UActor** lightpos = &model->Lights[lmindex.LightActors]; *lightpos; lightpos++, bits += lmpitch * height)
		{
			UActor* light = *lightpos;

			if (light->LightType() == LT_None || light->LightBrightness() == 0)
				continue;

			vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), 255) * clamp(light->LightBrightness() * (1.0f / 255.0f), 0.0f, 1.0f) * light->Level()->Brightness();

			switch (light->LightEffect())
			{
			case LE_TorchWaver:
			case LE_FireWaver:
			case LE_WateryShimmer:
			case LE_SlowWave:
			case LE_FastWave:
			case LE_CloudCast:
			case LE_Shock:
			case LE_Disco:
			case LE_Warp:
			case LE_NonIncidence:
			case LE_Shell:
			case LE_OmniBumpMap:
			case LE_Interference:
			case LE_Cylinder:
			case LE_Rotor:
			case LE_Unused:
			case LE_None:
				{
					for (int y = 0; y < height; y++)
					{
						float x0 = uv[0].x + leftStep * (y + 0.5f - uv[0].y) + 0.5f;
						float x1 = uv[1].x + rightStep * (y + 0.5f - uv[1].y) + 0.5f;
						float t0 = (y + 0.5f - uv[0].y) / leftDY;
						float t1 = (y + 0.5f - uv[1].y) / rightDY;
						vec3 p0 = mix(p[0], p[2], t0);
						vec3 p1 = mix(p[1], p[2], t1);
						if (x1 < x0) { std::swap(x0, x1); std::swap(p0, p1); }
						DrawLightmapSpan(&lightcolors[(size_t)y * width], width, x0, x1, p0, p1, light, N, bits + y * lmpitch, lightcolor);
					}
				}
				break;
			case LE_Searchlight:
			case LE_StaticSpot:
			case LE_Spotlight:
				// Let us not draw the spot lights for now (drawing them as point lights just looks horrible)
				break;
			}
		}

		float weights[9] = { 0.125f, 0.25f, 0.125f, 0.25f, 0.50f, 0.25f, 0.125f, 0.25f, 0.125f };

		uint32_t* texels = (uint32_t*)lmmip.Data.data();
		bool isTranslucent = (surface.PolyFlags & PF_Translucent) == PF_Translucent;
		for (int y = 0; y < height; y++)
		{
			vec3* src = &lightcolors[(size_t)y * width];
			for (int x = 0; x < width; x++)
			{
				vec3 color = { 0.0f };
				for (int yy = -1; yy <= 1; yy++)
				{
					int yyy = clamp(y + yy, 0, height - 1) - y;
					for (int xx = -1; xx <= 1; xx++)
					{
						int xxx = clamp(x + xx, 0, width - 1);
						color += src[yyy * width + xxx] * weights[4 + xx + yy * 3];
					}
				}
				color *= 0.5f;

				uint32_t red = (uint32_t)clamp(color.r * 127.0f + 0.5f, 0.0f, 127.0f);
				uint32_t green = (uint32_t)clamp(color.g * 127.0f + 0.5f, 0.0f, 127.0f);
				uint32_t blue = (uint32_t)clamp(color.b * 127.0f + 0.5f, 0.0f, 127.0f);
				uint32_t alpha = 127;

				if (isTranslucent)
				{
					red = 64 + (red >> 2);
					green = 64 + (green >> 2);
					blue = 64 + (blue >> 2);
				}

				texels[x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
			texels += width;
		}
	}
	else
	{
		uint32_t* texels = (uint32_t*)lmmip.Data.data();
		int c = width * height;
		vec3 color = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());
		uint32_t red = (uint32_t)clamp(color.r * 127.0f + 0.5f, 0.0f, 127.0f);
		uint32_t green = (uint32_t)clamp(color.g * 127 + 0.5f, 0.0f, 127.0f);
		uint32_t blue = (uint32_t)clamp(color.b * 127 + 0.5f, 0.0f, 127.0f);
		uint32_t alpha = 127;
		uint32_t ambientcolor = (alpha << 24) | (red << 16) | (green << 8) | blue;
		for (int i = 0; i < c; i++)
		{
			texels[i] = ambientcolor;
		}
	}

	auto lmtexture = std::make_unique<LightmapTexture>();
	lmtexture->Format = TextureFormat::BGRA8_LM;
	lmtexture->Mip = std::move(lmmip);
	return lmtexture;
}

void RenderSubsystem::DrawLightmapSpan(vec3* line, int width, float x0, float x1, vec3 p0, vec3 p1, UActor* light, const vec3& N, const uint8_t* bits, vec3 lightcolor)
{
	int bitpos = 0;
	for (int i = 0; i < width; i++)
	{
		bool shadowtest = (bits[bitpos >> 3] & (1 << (bitpos & 7))) != 0;
		if (shadowtest)
		{
			float t = (i + 0.5f - x0) / (x1 - x0);
			vec3 point = mix(p0, p1, t);

			vec3 L = light->Location() - point;
			float distanceAttenuation = std::max(1.0f - length(L) / (light->LightRadius() * 32.0f), 0.0f);
			float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
			float attenuation = distanceAttenuation * angleAttenuation;
			line[i] += lightcolor * attenuation;
		}

		bitpos++;
	}
}

vec3 RenderSubsystem::FindLightAt(const vec3& location, int zoneIndex)
{
	UZoneInfo* zoneActor = dynamic_cast<UZoneInfo*>(engine->Level->Model->Zones[zoneIndex].ZoneActor);
	if (!zoneActor)
		zoneActor = engine->LevelInfo;

	vec3 color = hsbtorgb(zoneActor->AmbientHue(), zoneActor->AmbientSaturation(), zoneActor->AmbientBrightness());

	for (UActor* light : Light.Lights)
	{
		if (light && !light->bCorona() && !light->bSpecialLit())
		{
			vec3 L = light->Location() - location;
			float dist2 = dot(L, L);
			float lightRadius = light->LightRadius() * 32.0f;
			float lightRadius2 = lightRadius * lightRadius;
			if (dist2 < lightRadius2 && !engine->Level->TraceRayAnyHit(light->Location(), location, nullptr, false, true, true))
			{
				vec3 lightcolor = hsbtorgb(light->LightHue(), light->LightSaturation(), light->LightBrightness());

				float distanceAttenuation = std::max(1.0f - std::sqrt(dist2) / lightRadius, 0.0f);
				float angleAttenuation = 0.75f; // std::max(dot(normalize(L), N), 0.0f);
				float attenuation = distanceAttenuation * angleAttenuation;
				color += lightcolor * attenuation;
			}
		}
	}

	return color;
}
