#pragma once

#include <cmath>
#include "Math/vec.h"
#include "Packages/Engine/Actors/UActor.h"

class UActor;

struct LightEffectArgs
{
	UActor* light;
	int size;
	const vec3* locations;
	float* result;
	vec3 LightLocation;
	float radius;
	float invRadius;
	float invRadiusSquared;
	vec3 N;
	const float* shadowmap;
};

class LightEffect
{
public:
	void Run(UActor* light, int width, int height, const vec3* locations, vec3 base, vec3 normal, const float* shadowmap, float* result);

private:
	void NoneEffect(LightEffectArgs* args);
	void NonIncidenceEffect(LightEffectArgs* args);
	void CylinderEffect(LightEffectArgs* args);
	void SlowWaveEffect(LightEffectArgs* args);
	void FastWaveEffect(LightEffectArgs* args);
	void ShellEffect(LightEffectArgs* args);
	void SpotlightEffect(LightEffectArgs* args);
	void SearchlightEffect(LightEffectArgs* args);
	void OmniBumpMapEffect(LightEffectArgs* args);

	typedef void (LightEffect::* EffectFunc)(LightEffectArgs* args);
	static EffectFunc Effects[LE_Unused + 1];
};
