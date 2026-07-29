#include "Packages/Engine/Actors/Pawn/PawnVision.h"

#include <iostream>
#include <limits>

namespace
{
	int failures = 0;

	void Expect(bool condition, const char* name)
	{
		if (!condition)
		{
			std::cerr << "FAILED: " << name << '\n';
			failures++;
		}
	}
}

int main()
{
	const vec3 origin(0.0f);
	const vec3 forward(1.0f, 0.0f, 0.0f);

	Expect(PawnVision::IsWithinVisionCone(origin, vec3(10.0f, 0.0f, 0.0f), forward, 0.5f), "target ahead");
	Expect(!PawnVision::IsWithinVisionCone(origin, vec3(-10.0f, 0.0f, 0.0f), forward, 0.5f), "target behind");

	const vec3 target(3.0f, 4.0f, 0.0f);
	const vec3 translation(-1000.0f, 250.0f, 75.0f);
	const bool untranslated = PawnVision::IsWithinVisionCone(origin, target, forward, 0.5f);
	const bool translated = PawnVision::IsWithinVisionCone(origin + translation, target + translation, forward, 0.5f);
	Expect(untranslated, "untranslated encounter");
	Expect(translated, "translated encounter");
	Expect(untranslated == translated, "translation invariance");

	const vec3 rearQuarter(-0.5f, std::sqrt(0.75f), 0.0f);
	Expect(PawnVision::IsWithinVisionCone(origin, rearQuarter, forward, -0.6f), "negative threshold widens cone");
	Expect(!PawnVision::IsWithinVisionCone(origin, vec3(-1.0f, 0.0f, 0.0f), forward, -0.6f), "negative threshold still filters behind");

	Expect(PawnVision::IsWithinVisionCone(origin, target, forward, 0.6f), "inclusive angular boundary");
	Expect(!PawnVision::IsWithinVisionCone(origin, target, forward, 0.6001f), "outside angular boundary");
	Expect(PawnVision::IsWithinVisionCone(origin, vec3(0.0f, 1.0f, 0.0f), forward, 0.0f), "perpendicular boundary");
	Expect(PawnVision::IsWithinVisionCone(origin, vec3(1.0f, 0.0f, 0.0f), forward, 1.0f), "narrowest threshold boundary");
	Expect(PawnVision::IsWithinVisionCone(origin, vec3(-1.0f, 0.0f, 0.0f), forward, -1.0f), "widest threshold boundary");

	Expect(!PawnVision::IsWithinVisionCone(origin, origin, forward, 0.0f), "coincident observer and target");
	Expect(!PawnVision::IsWithinVisionCone(origin, target, vec3(0.0f), 0.0f), "zero view direction");
	Expect(!PawnVision::IsWithinVisionCone(origin, vec3(std::numeric_limits<float>::infinity(), 0.0f, 0.0f), forward, 0.0f), "non-finite target");
	Expect(!PawnVision::IsWithinVisionCone(origin, target, forward, std::numeric_limits<float>::quiet_NaN()), "non-finite threshold");
	const float largest = std::numeric_limits<float>::max();
	Expect(PawnVision::IsWithinVisionCone(vec3(largest, 0.0f, 0.0f), vec3(-largest, 0.0f, 0.0f), -forward, 1.0f), "large finite coordinates");

	return failures == 0 ? 0 : 1;
}
