#pragma once

#include "UZoneInfo.h"

class UDynamicZoneInfo : public UZoneInfo
{
	using UZoneInfo::UZoneInfo;

	UDynamicZoneInfo*& NextDynamicZone() { return Value<UDynamicZoneInfo*>(PropOffsets_DynamicZoneInfo.NextDynamicZone); }
	EDynZoneInfoType ZoneAreaType() { return static_cast<EDynZoneInfoType>(Value<uint8_t>(PropOffsets_DynamicZoneInfo.ZoneAreaType)); }
	vec3*& BoxMin() { return Value<vec3*>(PropOffsets_DynamicZoneInfo.BoxMin); }
	vec3*& BoxMax() { return Value<vec3*>(PropOffsets_DynamicZoneInfo.BoxMax); }
	float& CylinderSize() { return Value<float>(PropOffsets_DynamicZoneInfo.CylinderSize); }
	float& SphereSize() { return Value<float>(PropOffsets_DynamicZoneInfo.SphereSize); }
	UZoneInfo*& MatchOnlyZone() { return Value<UZoneInfo*>(PropOffsets_DynamicZoneInfo.MatchOnlyZone); }
	BitfieldBool bUseRelativeToRotation() { return BoolValue(PropOffsets_DynamicZoneInfo.bUseRelativeToRotation); }
	BitfieldBool bMovesForceTouchUpdate() { return BoolValue(PropOffsets_DynamicZoneInfo.bMovesForceTouchUpdate); }
	BitfieldBool bUpdateTouchers() { return BoolValue(PropOffsets_DynamicZoneInfo.bUpdateTouchers); }
	vec3*& OldPose() { return Value<vec3*>(PropOffsets_DynamicZoneInfo.OldPose); }
};
