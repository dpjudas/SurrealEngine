#pragma once

#include "UKeypoint.h"

class UInterpolationPoint : public UKeypoint
{
public:
	using UKeypoint::UKeypoint;

	float& FovModifier() { return Value<float>(PropOffsets_InterpolationPoint.FovModifier); }
	float& GameSpeedModifier() { return Value<float>(PropOffsets_InterpolationPoint.GameSpeedModifier); }
	UInterpolationPoint*& Next() { return Value<UInterpolationPoint*>(PropOffsets_InterpolationPoint.Next); }
	int& Position() { return Value<int>(PropOffsets_InterpolationPoint.Position); }
	UInterpolationPoint*& Prev() { return Value<UInterpolationPoint*>(PropOffsets_InterpolationPoint.Prev); }
	float& RateModifier() { return Value<float>(PropOffsets_InterpolationPoint.RateModifier); }
	vec3& ScreenFlashFog() { return Value<vec3>(PropOffsets_InterpolationPoint.ScreenFlashFog); }
	float& ScreenFlashScale() { return Value<float>(PropOffsets_InterpolationPoint.ScreenFlashScale); }
	BitfieldBool bEndOfPath() { return BoolValue(PropOffsets_InterpolationPoint.bEndOfPath); }
	BitfieldBool bSkipNextPath() { return BoolValue(PropOffsets_InterpolationPoint.bSkipNextPath); }
};
