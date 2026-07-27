#pragma once

#include "UNavigationPoint.h"

class UMover;
class UTrigger;

class ULiftExit : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	float& LastTriggerTime() { return Value<float>(PropOffsets_LiftExit.LastTriggerTime); }
	NameString& LiftTag() { return Value<NameString>(PropOffsets_LiftExit.LiftTag); }
	NameString& LiftTrigger() { return Value<NameString>(PropOffsets_LiftExit.LiftTrigger); }
	UMover*& MyLift() { return Value<UMover*>(PropOffsets_LiftExit.MyLift); }
	UTrigger*& RecommendedTrigger() { return Value<UTrigger*>(PropOffsets_LiftExit.RecommendedTrigger); }
};
