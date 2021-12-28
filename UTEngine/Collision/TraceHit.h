#pragma once

class UActor;

class TraceHit
{
public:
	TraceHit() = default;
	TraceHit(float fraction, vec3 normal) : Fraction(fraction), Normal(normal) { }

	float Fraction = 1.0;
	vec3 Normal = vec3(0.0);
};

class SweepHit
{
public:
	SweepHit() = default;
	SweepHit(float fraction, vec3 normal, UActor* actor) : Fraction(fraction), Normal(normal), Actor(actor) { }

	float Fraction = 1.0;
	vec3 Normal = vec3(0.0);
	UActor* Actor = nullptr;
};
