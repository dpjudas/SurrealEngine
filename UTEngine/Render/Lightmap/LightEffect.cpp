
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "UObject/UActor.h"

void LightEffect::Run(UActor* light, int width, int height, const vec3* locations, vec3 N, const float* shadowmap, float* result)
{
	int size = width * height;

	float radius = light->WorldLightRadius();
	float invRadiusSquared = 1.0f / (radius * radius);

	// To do: implement all the light effects

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
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float dist = dot(L, L) * invRadiusSquared;
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
