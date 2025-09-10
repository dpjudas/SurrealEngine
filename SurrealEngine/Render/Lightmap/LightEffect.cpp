
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "UObject/UActor.h"
#include "Math/coords.h"

static float LightDistanceFalloff(float distsqr)
{
	float v = std::sqrt(distsqr + 0.0001f);
	float v2 = v * v;
	float v3 = v2 * v;
	return std::min((1.0f + 2.0f * v3 - 3.0f * v2) / v, 1.0f);
}

void LightEffect::Run(UActor* light, int width, int height, const vec3* locations, vec3 base, vec3 N, const float* shadowmap, float* result)
{
	int size = width * height;

	float radius = light->WorldLightRadius();
	float invRadius = 1.0f / radius;
	float invRadiusSquared = invRadius * invRadius;

	// UE1 uses a single angle attenuation for the entire surface
	//float angleAttenuation = std::abs(dot(normalize(light->Location() - base), N));

	N = normalize(N); // the normal isn't normalized!

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
	{
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float angleAttenuation = std::abs(dot(normalize(L), N));
			float distsqr = dot(L, L) * invRadiusSquared;
			if (distsqr < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(distsqr);
				result[i] = shadowmap[i] * distanceAttenuation * angleAttenuation;
			}
			else
			{
				result[i] = 0.0f;
			}
		}
		break;
	}

	case LE_NonIncidence: // linear falloff
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			result[i] = shadowmap[i] * std::max(1.0f - length(L) * invRadius, 0.0f);
		}
		break;

	case LE_Cylinder: // squared falloff
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float distsqr = (L.x * L.x + L.y * L.y) * invRadiusSquared;
			result[i] = shadowmap[i] * std::max(1.0f - distsqr, 0.0f);
		}
		break;

	case LE_Shell:
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float dist = length(L) * invRadius;
			float attenuation = (dist > 0.8f && dist < 1.0f) ? 1.0f - 10.0f * std::abs(dist - 0.9f) : 0.0f;
			result[i] = shadowmap[i] * attenuation;
		}
		break;

	case LE_Spotlight:
	case LE_StaticSpot:
	{
		vec3 tmp0, tmp1, tmp2;
		Coords::Rotation(light->Rotation()).GetAxes(tmp0, tmp1, tmp2);
		vec3 spotDir = -tmp0;
		float lightCosOuterAngle = 1.0f - light->LightCone() * (1.0f / 255.0f);
		float lightCosInnerAngle = 1.0f;
		for (int i = 0; i < size; i++)
		{
			vec3 L = light->Location() - locations[i];
			float angleAttenuation = std::abs(dot(normalize(L), N));

			float distsqr = dot(L, L) * invRadiusSquared;
			if (distsqr < 1.0f && lightCosOuterAngle < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(distsqr);
				float cosDir = dot(normalize(L), spotDir);
				float spotAttenuation = 1.0f - std::min((1.0f - cosDir) / (1.0f - lightCosOuterAngle), 1.0f);
				spotAttenuation = spotAttenuation * spotAttenuation;
				result[i] = shadowmap[i] * distanceAttenuation * angleAttenuation * spotAttenuation;
			}
			else
			{
				result[i] = 0.0f;
			}
		}
		break;
	}

	case LE_Searchlight:
	default:
		for (int i = 0; i < size; i++)
		{
			result[i] = 0.0f;
		}
		break;
	}
}
