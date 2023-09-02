
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "UObject/UActor.h"
#include "Math/coords.h"

void LightEffect::Run(UActor* light, int width, int height, const vec3* locations, vec3 base, vec3 N, const float* shadowmap, float* result)
{
	int size = width * height;

	float radius = light->WorldLightRadius();
	float invRadius = 1.0f / radius;
	float invRadiusSquared = 1.0f / (radius * radius);

	// UE1 uses a single angle attenuation for the entire surface
//#ifndef NOSSE
//	__m128d base = _mm_
//	float angleAttenuation = 
//#else
	float angleAttenuation = std::abs(dot(light->Location() - base, N) * invRadius);
//#endif

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
			float distsqr = (L.x * L.x + L.y * L.y) * invRadiusSquared;
			result[i] = shadowmap[i] * std::max(1.0f - distsqr, 0.0f);
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

float LightEffect::VertexLight(UActor* light, const vec3& location, const vec3& N)
{
	float radius = light->WorldLightRadius();
	float invRadius = 1.0f / radius;
	float invRadiusSquared = 1.0f / (radius * radius);

	float angleAttenuation = std::abs(dot(light->Location() - location, N) * invRadius);

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
			float distsqr = dot(L, L) * invRadiusSquared;
			if (distsqr < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(distsqr);
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
			float distsqr = (L.x * L.x + L.y * L.y) * invRadiusSquared;
			return std::max(1.0f - distsqr, 0.0f);
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

			float distsqr = dot(L, L) * invRadiusSquared;
			if (distsqr < 1.0f && lightCosOuterAngle < 1.0f)
			{
				float distanceAttenuation = LightDistanceFalloff(distsqr);
				float cosDir = dot(normalize(L), spotDir);
				float spotAttenuation = 1.0f - std::min((1.0f - cosDir) / (1.0f - lightCosOuterAngle), 1.0f);
				spotAttenuation = spotAttenuation * spotAttenuation;
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
