
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Math/coords.h"

LightEffect::EffectFunc LightEffect::Effects[LE_Unused + 1] =
{
	&LightEffect::NoneEffect, // LE_None
	&LightEffect::SlowWaveEffect, // LE_TorchWaver - to do: needs implementation
	&LightEffect::SlowWaveEffect, // LE_FireWaver - to do: needs implementation
	&LightEffect::SlowWaveEffect, // LE_WateryShimmer - to do: needs implementation
	&LightEffect::SearchlightEffect, // LE_Searchlight
	&LightEffect::SlowWaveEffect, // LE_SlowWave
	&LightEffect::FastWaveEffect, // LE_FastWave
	&LightEffect::NoneEffect, // LE_CloudCast
	&LightEffect::SpotlightEffect, // LE_StaticSpot
	&LightEffect::SlowWaveEffect, // LE_Shock - to do: needs implementation
	&LightEffect::SlowWaveEffect, // LE_Disco - to do: needs implementation
	&LightEffect::NoneEffect, // LE_Warp
	&LightEffect::SpotlightEffect, // LE_Spotlight
	&LightEffect::NonIncidenceEffect, // LE_NonIncidence
	&LightEffect::ShellEffect, // LE_Shell
	&LightEffect::OmniBumpMapEffect, // LE_OmniBumpMap
	&LightEffect::SlowWaveEffect, // LE_Interference - to do: needs implementation
	&LightEffect::CylinderEffect, // LE_Cylinder
	&LightEffect::SlowWaveEffect, // LE_Rotor - to do: needs implementation
	&LightEffect::NoneEffect, // LE_Unused
};

void LightEffect::Run(UActor* light, int width, int height, const vec3* locations, vec3 base, vec3 N, const float* shadowmap, float* result)
{
	LightEffectArgs args;
	args.light = light;
	args.size = width * height;
	args.locations = locations;
	args.N = N;
	args.shadowmap = shadowmap;
	args.result = result;
	args.LightLocation = light->Location();
	args.radius = light->WorldLightRadius();
	args.invRadius = 1.0f / args.radius;
	args.invRadiusSquared = args.invRadius * args.invRadius;

	uint8_t effect = light->LightEffect();
	if (effect <= LE_Unused)
		(this->*Effects[effect])(&args);
	else
		NoneEffect(&args);
}

static float LightDistanceFalloff(float distsqr)
{
	float v = std::sqrt(distsqr + 0.0001f);
	float v2 = v * v;
	float v3 = v2 * v;
	return std::min((1.0f + 2.0f * v3 - 3.0f * v2) / v, 1.0f);
}

void LightEffect::NoneEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const vec3 N = args->N;
	const float invRadiusSquared = args->invRadiusSquared;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
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
}

void LightEffect::NonIncidenceEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const float invRadius = args->invRadius;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
		result[i] = shadowmap[i] * std::max(1.0f - length(L) * invRadius, 0.0f);
	}
}

void LightEffect::CylinderEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const float invRadiusSquared = args->invRadiusSquared;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
		float distsqr = (L.x * L.x + L.y * L.y) * invRadiusSquared;
		result[i] = shadowmap[i] * std::max(1.0f - distsqr, 0.0f);
	}
}

void LightEffect::SlowWaveEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const vec3 N = args->N;
	const float invRadiusSquared = args->invRadiusSquared;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	float timeOffset = args->light->Level()->TimeSeconds() * 2.0f;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
		float waveAttenuation = 0.6f + 0.4f * std::sin(length(L) * 0.04f + timeOffset);
		float angleAttenuation = std::abs(dot(normalize(L), N));
		float distsqr = dot(L, L) * invRadiusSquared;
		if (distsqr < 1.0f)
		{
			float distanceAttenuation = LightDistanceFalloff(distsqr);
			result[i] = shadowmap[i] * distanceAttenuation * angleAttenuation * waveAttenuation;
		}
		else
		{
			result[i] = 0.0f;
		}
	}
}

void LightEffect::FastWaveEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const vec3 N = args->N;
	const float invRadiusSquared = args->invRadiusSquared;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	float timeOffset = args->light->Level()->TimeSeconds() * 4.0f;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
		float waveAttenuation = 0.6f + 0.4f * std::sin(length(L) * 0.04f + timeOffset);
		float angleAttenuation = std::abs(dot(normalize(L), N));
		float distsqr = dot(L, L) * invRadiusSquared;
		if (distsqr < 1.0f)
		{
			float distanceAttenuation = LightDistanceFalloff(distsqr);
			result[i] = shadowmap[i] * distanceAttenuation * angleAttenuation * waveAttenuation;
		}
		else
		{
			result[i] = 0.0f;
		}
	}
}

void LightEffect::ShellEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const float invRadius = args->invRadius;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
		float dist = length(L) * invRadius;
		float attenuation = (dist > 0.8f && dist < 1.0f) ? 1.0f - 10.0f * std::abs(dist - 0.9f) : 0.0f;
		result[i] = shadowmap[i] * attenuation;
	}
}

void LightEffect::SpotlightEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const vec3 N = args->N;
	const float invRadiusSquared = args->invRadiusSquared;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;
	vec3 tmp0, tmp1, tmp2;
	Coords::Rotation(args->light->Rotation()).GetAxes(tmp0, tmp1, tmp2);
	vec3 spotDir = -tmp0;
	float lightCosOuterAngle = 1.0f - args->light->LightCone() * (1.0f / 255.0f);
	float lightCosInnerAngle = 1.0f;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
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
}

void LightEffect::SearchlightEffect(LightEffectArgs* args)
{
	// The lightbeam will rotate, like a searchlight.
	// If LightPeriod is 0, the SearchLight will not rotate.
	// 
	// Like the cylinder light effect, the Z component is ignored.

	const int size = args->size;
	const vec3 lightLocation = args->LightLocation;
	const vec3 N = args->N;
	const float invRadiusSquared = args->invRadiusSquared;
	const vec3* locations = args->locations;
	const float* shadowmap = args->shadowmap;
	float* result = args->result;

	constexpr float phaseScale = (1.0f / 255.0f);
	constexpr float periodSpeed = 40.0f;
	float turns = args->light->LightPhase() * phaseScale;
	if (args->light->LightPeriod() != 0)
		turns += args->light->Level()->TimeSeconds() * periodSpeed / args->light->LightPeriod();
	float spotAngle = turns * (2.0f * 3.14159265359f);
	float lightCone = 0.5f;

	vec3 spotDir(std::sin(spotAngle), std::cos(spotAngle), 0.0f);
	float lightCosOuterAngle = 1.0f - lightCone * (1.0f / 255.0f);
	float lightCosInnerAngle = 1.0f;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
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
}

void LightEffect::OmniBumpMapEffect(LightEffectArgs* args)
{
	// Can be used to create blacklights. This is not what the name suggests, so this is
	// probably an old remainder of the pre-release version of Unreal.
	const int size = args->size;
	float* result = args->result;
	for (int i = 0; i < size; i++)
	{
		result[i] = 0.0f;
	}
}
