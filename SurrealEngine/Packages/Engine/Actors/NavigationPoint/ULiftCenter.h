#pragma once

#include "UNavigationPoint.h"

class UMover;
class UTrigger;

class ULiftCenter : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	float& LastTriggerTime() { return Value<float>(PropOffsets_LiftCenter.LastTriggerTime); }
	vec3& LiftOffset() { return Value<vec3>(PropOffsets_LiftCenter.LiftOffset); }
	NameString& LiftTag() { return Value<NameString>(PropOffsets_LiftCenter.LiftTag); }
	NameString& LiftTrigger() { return Value<NameString>(PropOffsets_LiftCenter.LiftTrigger); }
	float& MaxDist2D() { return Value<float>(PropOffsets_LiftCenter.MaxDist2D); }
	float& MaxZDiffAdd() { return Value<float>(PropOffsets_LiftCenter.MaxZDiffAdd); }
	UMover*& MyLift() { return Value<UMover*>(PropOffsets_LiftCenter.MyLift); }
	UTrigger*& RecommendedTrigger() { return Value<UTrigger*>(PropOffsets_LiftCenter.RecommendedTrigger); }
};
