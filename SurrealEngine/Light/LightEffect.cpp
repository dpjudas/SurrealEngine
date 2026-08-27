
#include "Precomp.h"
#include "LightEffect.h"
#include "Shadowmap.h"
#include "Packages/Engine/Actors/UActor.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Math/coords.h"

#ifdef USE_SSE2
#include <immintrin.h>
#endif
#ifdef USE_NEON
#include <arm_neon.h>
#endif

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
	if (!TablesInitialized)
		InitTables();

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

void LightEffect::NoneEffect(LightEffectArgs* args)
{
	const int size = args->size;
	const float lightLocationX = args->LightLocation.x;
	const float lightLocationY = args->LightLocation.y;
	const float lightLocationZ = args->LightLocation.z;
	const float Nx = args->N.x;
	const float Ny = args->N.y;
	const float Nz = args->N.z;
	const float invRadius = args->invRadius;
	const float invRadiusSquared = args->invRadiusSquared;
	const float* locations = (float*)args->locations; // vec3
	const float* shadowmap = args->shadowmap;
	float* result = args->result;

#ifdef USE_SSE2
	__m128 mmLightLocationX = _mm_set_ps1(lightLocationX);
	__m128 mmLightLocationY = _mm_set_ps1(lightLocationY);
	__m128 mmLightLocationZ = _mm_set_ps1(lightLocationZ);
	__m128 mmInvRadius = _mm_set_ps1(invRadius);
	__m128 mmInvRadiusSquared = _mm_set_ps1(invRadiusSquared);
	__m128 mmNx = _mm_set_ps1(Nx);
	__m128 mmNy = _mm_set_ps1(Ny);
	__m128 mmNz = _mm_set_ps1(Nz);
	int sse_size = size / 4 * 4;
	for (int i = 0; i < sse_size; i += 4)
	{
		// To do: memory align buffers
		//__m128 locationX = _mm_loadu_ps(locations);
		//__m128 locationY = _mm_loadu_ps(locations + 4);
		//__m128 locationZ = _mm_loadu_ps(locations + 8);
		// To do: transpose this
		__m128 locationX = _mm_setr_ps(locations[0], locations[3], locations[6], locations[9]);
		__m128 locationY = _mm_setr_ps(locations[1], locations[4], locations[7], locations[10]);
		__m128 locationZ = _mm_setr_ps(locations[2], locations[5], locations[8], locations[11]);
		__m128 Lx = _mm_sub_ps(mmLightLocationX, locationX); // float Lx = lightLocationX - locations[0];
		__m128 Ly = _mm_sub_ps(mmLightLocationY, locationY); // float Ly = lightLocationY - locations[1];
		__m128 Lz = _mm_sub_ps(mmLightLocationZ, locationZ); // float Lz = lightLocationZ - locations[2];
		__m128 lensqr = _mm_add_ps(_mm_add_ps(_mm_mul_ps(Lx, Lx), _mm_mul_ps(Ly, Ly)), _mm_mul_ps(Lz, Lz)); // float lensqr = Lx * Lx + Ly * Ly + Lz * Lz;
		__m128 unitdistsqr = _mm_mul_ps(lensqr, mmInvRadiusSquared); // float unitdistsqr = lensqr * invRadiusSquared;
		__m128 cmpdistmask = _mm_cmplt_ps(unitdistsqr, _mm_set_ps1(1.0f)); // if (unitdistsqr < 1.0f)
		if (_mm_movemask_ps(cmpdistmask) != 0)
		{
			__m128 len = _mm_sqrt_ps(lensqr); // float len = std::sqrt(lensqr);
			__m128 rcpdist = _mm_rcp_ps(len); // float rcpdist = 1.0f / len;
			Lx = _mm_mul_ps(Lx, rcpdist); // Lx *= rcpdist;
			Ly = _mm_mul_ps(Ly, rcpdist); // Ly *= rcpdist;
			Lz = _mm_mul_ps(Lz, rcpdist); // Lz *= rcpdist;
			__m128 angleAttenuation = _mm_add_ps(_mm_add_ps(_mm_mul_ps(Lx, mmNx), _mm_mul_ps(Ly, mmNy)), _mm_mul_ps(Lz, mmNz)); // float angleAttenuation = Lx * Nx + Ly * Ny + Lz * Nz;
			__m128 cmpmask = _mm_cmpge_ps(angleAttenuation, _mm_setzero_ps());
			angleAttenuation = _mm_or_ps(_mm_and_ps(cmpmask, angleAttenuation), _mm_andnot_ps(cmpmask, _mm_sub_ps(_mm_setzero_ps(), angleAttenuation))); // angleAttenuation = angleAttenuation >= 0.0f ? angleAttenuation : -angleAttenuation;

			//float distanceAttenuation = LightDistanceFalloff(unitdistsqr);
			__m128 v = _mm_mul_ps(len, mmInvRadius); // float v = len * invRadius;
			__m128 v2 = _mm_mul_ps(v, v); // float v2 = v * v;
			__m128 v3 = _mm_mul_ps(v2, v); // float v3 = v2 * v;
			__m128 distanceAttenuation = _mm_div_ps(_mm_sub_ps(_mm_add_ps(_mm_set_ps1(1.0f), _mm_mul_ps(_mm_set_ps1(2.0f), v3)), _mm_mul_ps(_mm_set_ps1(3.0f), v2)), v); // float distanceAttenuation = (1.0f + 2.0f * v3 - 3.0f * v2) / v;
			distanceAttenuation = _mm_min_ps(distanceAttenuation, _mm_set_ps1(1.0f)); // distanceAttenuation = std::min(distanceAttenuation, 1.0f);

			__m128 value = _mm_mul_ps(_mm_loadu_ps(shadowmap), _mm_mul_ps(distanceAttenuation, angleAttenuation));
			value = _mm_or_ps(_mm_and_ps(cmpdistmask, value), _mm_andnot_ps(cmpdistmask, _mm_setzero_ps()));
			_mm_storeu_ps(result, value);
		}
		else
		{
			_mm_storeu_ps(result, _mm_setzero_ps());
		}
		locations += 3 * 4;
		shadowmap += 4;
		result += 4;
	}
#elif defined(USE_NEON)
	float32x4_t vLx0 = vdupq_n_f32(lightLocationX);
	float32x4_t vLy0 = vdupq_n_f32(lightLocationY);
	float32x4_t vLz0 = vdupq_n_f32(lightLocationZ);
	float32x4_t vNx = vdupq_n_f32(Nx);
	float32x4_t vNy = vdupq_n_f32(Ny);
	float32x4_t vNz = vdupq_n_f32(Nz);
	float32x4_t vInvR = vdupq_n_f32(invRadius);
	float32x4_t vInvR2 = vdupq_n_f32(invRadiusSquared);
	float32x4_t one = vdupq_n_f32(1.0f);
	float32x4_t two = vdupq_n_f32(2.0f);
	float32x4_t three = vdupq_n_f32(3.0f);

	int sse_size = size / 4 * 4;
	for (int i = 0; i < sse_size; i += 4)
	{
		float32x4x3_t P = vld3q_f32(locations);
		float32x4_t Lx = vsubq_f32(vLx0, P.val[0]);
		float32x4_t Ly = vsubq_f32(vLy0, P.val[1]);
		float32x4_t Lz = vsubq_f32(vLz0, P.val[2]);

		float32x4_t lensqr = vfmaq_f32(vfmaq_f32(vmulq_f32(Lx, Lx), Ly, Ly), Lz, Lz);
		uint32x4_t inRange = vcltq_f32(vmulq_f32(lensqr, vInvR2), one);

		if (vmaxvq_u32(inRange))
		{
			float32x4_t len = vsqrtq_f32(lensqr);
			float32x4_t rcpdist = vdivq_f32(one, len);

			Lx = vmulq_f32(Lx, rcpdist);
			Ly = vmulq_f32(Ly, rcpdist);
			Lz = vmulq_f32(Lz, rcpdist);

			float32x4_t angle = vfmaq_f32(vfmaq_f32(vmulq_f32(Lx, vNx), Ly, vNy), Lz, vNz);
			angle = vabsq_f32(angle);

			float32x4_t v = vmulq_f32(len, vInvR);
			float32x4_t v2 = vmulq_f32(v, v);
			float32x4_t v3 = vmulq_f32(v2, v);
			float32x4_t num = vsubq_f32(vfmaq_f32(one, two, v3), vmulq_f32(three, v2));
			float32x4_t dist = vminq_f32(vdivq_f32(num, v), one);

			float32x4_t value = vmulq_f32(vld1q_f32(shadowmap), vmulq_f32(dist, angle));
			value = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(value), inRange));
			vst1q_f32(result, value);
		}
		else
		{
			vst1q_f32(result, vdupq_n_f32(0.0f));
		}

		locations += 3 * 4;
		shadowmap += 4;
		result += 4;
	}
#else
	int sse_size = 0;
#endif

	for (int i = sse_size; i < size; i++)
	{
		float Lx = lightLocationX - locations[0];
		float Ly = lightLocationY - locations[1];
		float Lz = lightLocationZ - locations[2];
		float lensqr = Lx * Lx + Ly * Ly + Lz * Lz;
		float unitdistsqr = lensqr * invRadiusSquared;
		if (unitdistsqr < 1.0f)
		{
#ifdef USE_SSE2
			float len = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(lensqr)));
#else
			float len = std::sqrt(lensqr);
#endif
			float rcpdist = 1.0f / len;
			Lx *= rcpdist;
			Ly *= rcpdist;
			Lz *= rcpdist;
			float angleAttenuation = Lx * Nx + Ly * Ny + Lz * Nz;
			angleAttenuation = angleAttenuation >= 0.0f ? angleAttenuation : -angleAttenuation;

			//float distanceAttenuation = LightDistanceFalloff(unitdistsqr);
			float v = len * invRadius;
			float v2 = v * v;
			float v3 = v2 * v;
			float distanceAttenuation = (1.0f + 2.0f * v3 - 3.0f * v2) / v;
			distanceAttenuation = distanceAttenuation <= 1.0f ? distanceAttenuation : 1.0f;

			*result = (*shadowmap) * distanceAttenuation * angleAttenuation;
		}
		else
		{
			*result = 0.0f;
		}
		locations += 3;
		shadowmap++;
		result++;
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
		float lensqr = dot(L, L);
		float distsqr = lensqr * invRadiusSquared;
		if (distsqr < 1.0f)
		{
#ifdef USE_SSE2
			float len = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(lensqr)));
#else
			float len = std::sqrt(lensqr);
#endif
			L *= 1.0f / len;
			float waveAttenuation = 0.6f + 0.4f * Sin(len * 0.04f + timeOffset);
			float angleAttenuation = std::abs(dot(L, N));
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
		float lensqr = dot(L, L);
		float distsqr = lensqr * invRadiusSquared;
		if (distsqr < 1.0f)
		{
#ifdef USE_SSE2
			float len = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(lensqr)));
#else
			float len = std::sqrt(lensqr);
#endif
			L *= 1.0f / len;
			float waveAttenuation = 0.6f + 0.4f * Sin(len * 0.04f + timeOffset);
			float angleAttenuation = std::abs(dot(L, N));
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
	if (lightCosOuterAngle < 1.0f)
	{
		for (int i = 0; i < size; i++)
		{
			vec3 L = lightLocation - locations[i];
			float lensqr = dot(L, L);
			float distsqr = lensqr * invRadiusSquared;
			if (distsqr < 1.0f)
			{
#ifdef USE_SSE2
				float rcplen = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(lensqr)));
#else
				float rcplen = 1.0f / std::sqrt(lensqr);
#endif
				L *= rcplen;
				float angleAttenuation = std::abs(dot(L, N));
				float distanceAttenuation = LightDistanceFalloff(distsqr);
				float cosDir = dot(L, spotDir);
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
	else
	{
		for (int i = 0; i < size; i++)
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

	vec3 spotDir(Sin(spotAngle), Cos(spotAngle), 0.0f);
	float lightCosOuterAngle = 1.0f - lightCone * (1.0f / 255.0f);
	float lightCosInnerAngle = 1.0f;
	for (int i = 0; i < size; i++)
	{
		vec3 L = lightLocation - locations[i];
		float lensqr = dot(L, L);
		float distsqr = lensqr * invRadiusSquared;
		if (distsqr < 1.0f && lightCosOuterAngle < 1.0f)
		{
#ifdef USE_SSE2
			float rcplen = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(lensqr)));
#else
			float rcplen = 1.0f / std::sqrt(lensqr);
#endif
			L *= rcplen;
			float angleAttenuation = std::abs(dot(L, N));
			float distanceAttenuation = LightDistanceFalloff(distsqr);
			float cosDir = dot(L, spotDir);
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

void LightEffect::InitTables()
{
	for (int i = 0; i < SinTableSize; i++)
	{
		SinTable[i] = (float)std::sin(i * ((2.0 * 3.14159265359) / (float)SinTableSize));
		CosTable[i] = (float)std::cos(i * ((2.0 * 3.14159265359) / (float)SinTableSize));
	}

	for (int i = 0; i < FalloffTableSize; i++)
	{
		float v = std::sqrt(i + 0.0001f);
		float v2 = v * v;
		float v3 = v2 * v;
		float falloff = std::min((1.0f + 2.0f * v3 - 3.0f * v2) / v, 1.0f);
		FalloffTable[i] = CalcLightDistanceFalloff(i / (float)FalloffTableSize);
	}

	TablesInitialized = true;
}

float LightEffect::Sin(float v)
{
	int x = (int)(v * ((SinTableSize >> 1) / 3.14159265359f));
	x &= SinTableSize - 1;
	return SinTable[x];
}

float LightEffect::Cos(float v)
{
	int x = (int)(v * ((SinTableSize >> 1) / 3.14159265359f));
	x &= SinTableSize - 1;
	return CosTable[x];
}

float LightEffect::CalcLightDistanceFalloff(float distsqr)
{
	float v = std::sqrt(distsqr + 0.0001f);
	float v2 = v * v;
	float v3 = v2 * v;
	return std::min((1.0f + 2.0f * v3 - 3.0f * v2) / v, 1.0f);
}

float LightEffect::LightDistanceFalloff(float distsqr)
{
	int x = (int)(distsqr * ((float)FalloffTableSize - 1.0f));
	x &= FalloffTableSize - 1;
	return FalloffTable[x];
}

bool LightEffect::TablesInitialized;
float LightEffect::SinTable[SinTableSize];
float LightEffect::CosTable[SinTableSize];
float LightEffect::FalloffTable[FalloffTableSize];
