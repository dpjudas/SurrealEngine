
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "UObject/UActor.h"
#include "Math/coords.h"

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
				float distanceAttenuation = clamp(LightDistanceFalloff(dist), 0.0f, 1.0f);
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

			//float dist = dot(L, L) * invRadiusSquared;
			float dist = std::sqrt(dot(L, L)) * invRadius;
			if (dist < 1.0f && lightCosOuterAngle < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(dist);
				float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
				float cosDir = dot(normalize(L), spotDir);
#if 0
				float spotAttenuation = smoothstep(lightCosOuterAngle, lightCosInnerAngle, cosDir);
#else
				float spotAttenuation = 1.0f - std::min((1.0f - cosDir) / (1.0f - lightCosOuterAngle), 1.0f);
				spotAttenuation = spotAttenuation * spotAttenuation;
#endif
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

float LightEffect::VertexLight(UActor* light, const vec3& location, const vec3& N)
{
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
		{
			vec3 L = light->Location() - location;
			//float dist = dot(L, L) * invRadiusSquared;
			float dist = std::sqrt(dot(L, L)) * invRadius;
			if (dist < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(dist);
				float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
				return distanceAttenuation * angleAttenuation;
			}
			else
			{
				return 0.0f;
			}
		}
		break;

	case LE_NonIncidence:
		{
			vec3 L = light->Location() - location;
			float dist = std::sqrt(dot(L, L)) * invRadius;
			return std::max(1.0f - dist, 0.0f);
		}
		break;

	case LE_Cylinder:
		{
			vec3 L = light->Location() - location;
			float dist = (L.x * L.x + L.y * L.y) * invRadiusSquared;
			return std::max(1.0f - dist, 0.0f);
		}
		break;

	case LE_Shell:
		{
			vec3 L = light->Location() - location;
			float dist = std::sqrt(dot(L, L)) * invRadius;
			float attenuation = (dist > 0.8f && dist < 1.0f) ? 1.0f - 10.0f * std::abs(dist - 0.9f) : 0.0f;
			return attenuation;
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
		{
			vec3 L = light->Location() - location;

			//float dist = dot(L, L) * invRadiusSquared;
			float dist = std::sqrt(dot(L, L)) * invRadius;
			if (dist < 1.0f && lightCosOuterAngle < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(dist);
				float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
				float cosDir = dot(normalize(L), spotDir);
#if 0
				float spotAttenuation = smoothstep(lightCosOuterAngle, lightCosInnerAngle, cosDir);
#else
				float spotAttenuation = 1.0f - std::min((1.0f - cosDir) / (1.0f - lightCosOuterAngle), 1.0f);
				spotAttenuation = spotAttenuation * spotAttenuation;
#endif
				return distanceAttenuation * angleAttenuation * spotAttenuation;
			}
			else
			{
				return 0.0f;
			}
		}
		break;
	}

	case LE_Searchlight:
	default:
		return 0.0f;
	}
}
