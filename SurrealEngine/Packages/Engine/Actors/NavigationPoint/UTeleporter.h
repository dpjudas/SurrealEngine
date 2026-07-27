#pragma once

#include "UNavigationPoint.h"

class UTeleporter : public UNavigationPoint
{
public:
	using UNavigationPoint::UNavigationPoint;

	float& LastFired() { return Value<float>(PropOffsets_Teleporter.LastFired); }
	NameString& ProductRequired() { return Value<NameString>(PropOffsets_Teleporter.ProductRequired); }
	vec3& TargetVelocity() { return Value<vec3>(PropOffsets_Teleporter.TargetVelocity); }
	UActor*& TriggerActor() { return Value<UActor*>(PropOffsets_Teleporter.TriggerActor); }
	UActor*& TriggerActor2() { return Value<UActor*>(PropOffsets_Teleporter.TriggerActor2); }
	std::string& URL() { return Value<std::string>(PropOffsets_Teleporter.URL); }
	BitfieldBool bChangesVelocity() { return BoolValue(PropOffsets_Teleporter.bChangesVelocity); }
	BitfieldBool bChangesYaw() { return BoolValue(PropOffsets_Teleporter.bChangesYaw); }
	BitfieldBool bEnabled() { return BoolValue(PropOffsets_Teleporter.bEnabled); }
	BitfieldBool bReversesX() { return BoolValue(PropOffsets_Teleporter.bReversesX); }
	BitfieldBool bReversesY() { return BoolValue(PropOffsets_Teleporter.bReversesY); }
	BitfieldBool bReversesZ() { return BoolValue(PropOffsets_Teleporter.bReversesZ); }
};
