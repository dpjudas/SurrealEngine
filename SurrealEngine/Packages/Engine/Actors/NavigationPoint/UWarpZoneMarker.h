#pragma once

#include "UNavigationPoint.h"

class UWarpZoneInfo;

class UWarpZoneMarker : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_WarpZoneMarker.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_WarpZoneMarker.TriggerActor2); }
	UWarpZoneInfo*& markedWarpZone() { return Value<UWarpZoneInfo*>(PropOffsets_WarpZoneMarker.markedWarpZone); }
};
