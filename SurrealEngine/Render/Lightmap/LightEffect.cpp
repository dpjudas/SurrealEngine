
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "UObject/UActor.h"

void LightEffect::Run(UActor* light, int width, int height, const vec3* locations, vec3 N, const float* shadowmap, float* result)
{
	int size = width * height;

	float radius = light->WorldLightRadius();
	float invRadius = 1.0f / radius;
	float invRadiusSquared = 1.0f / (radius * radius);

	// To do: implement all the light effects

	switch (light->LightEffect())
	{
	case LE_None:
	case LE_TorchWaver:
	case LE_FireWaver:
	case LE_WateryShimmer:
	case LE_Warp:
	case LE_OmniBumpMap:
	case LE_Interference:
	case LE_SlowWave:
	case LE_FastWave:
	case LE_CloudCast:
	case LE_Shock:
	case LE_Disco:
	case LE_Rotor:
	case LE_Unused:
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			//float dist = dot(L, L) * invRadiusSquared;
			float dist = std::sqrt(dot(L, L)) * invRadius;
			if (dist < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(dist);
				float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
				result[i] = shadowmap[i] * distanceAttenuation * angleAttenuation;
			}
			else
			{
				result[i] = 0.0f;
			}
		}
		break;

	case LE_NonIncidence:
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float dist = std::sqrt(dot(L, L)) * invRadius;
			result[i] = shadowmap[i] * std::max(1.0f - dist, 0.0f);
		}
		break;

	case LE_Cylinder:
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float dist = (L.x * L.x + L.y * L.y) * invRadiusSquared;
			result[i] = shadowmap[i] * std::max(1.0f - dist, 0.0f);
		}
		break;

	case LE_Shell:
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float dist = std::sqrt(dot(L, L)) * invRadius;
			float attenuation = (dist > 0.8f && dist < 1.0f) ? 1.0f - 10.0f * std::abs(dist - 0.9f) : 0.0f;
			result[i] = shadowmap[i] * attenuation;
		}
		break;

	case LE_Searchlight:
	case LE_StaticSpot:
	case LE_Spotlight:
	default:
		// There are some maps where it looks horrible if they are drawn as point lights. Turn off for now.
		for (int i = 0; i < size; i++)
		{
			result[i] = 0.0f;
		}
		break;
	}
}
