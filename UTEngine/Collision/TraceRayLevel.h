#pragma once

#include "UObject/ULevel.h"

class TraceRayLevel
{
public:
	bool TraceAnyHit(ULevel* level, vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly);

private:
	ULevel* Level = nullptr;
};
