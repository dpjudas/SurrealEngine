#pragma once

#include "UZoneInfo.h"

class UWarpZoneInfo : public UZoneInfo
{
public:
	using UZoneInfo::UZoneInfo;

	void Warp(vec3& Loc, vec3& Vel, Rotator& R);
	void UnWarp(vec3& Loc, vec3& Vel, Rotator& R); // Warp but in reverse?

	std::string& Destinations() { return Value<std::string>(PropOffsets_WarpZoneInfo.Destinations); }
	UWarpZoneInfo*& OtherSideActor() { return Value<UWarpZoneInfo*>(PropOffsets_WarpZoneInfo.OtherSideActor); }
	UObject*& OtherSideLevel() { return Value<UObject*>(PropOffsets_WarpZoneInfo.OtherSideLevel); }
	std::string& OtherSideURL() { return Value<std::string>(PropOffsets_WarpZoneInfo.OtherSideURL); }
	NameString& ThisTag() { return Value<NameString>(PropOffsets_WarpZoneInfo.ThisTag); }
	Coords& WarpCoords() { return Value<Coords>(PropOffsets_WarpZoneInfo.WarpCoords); }
	BitfieldBool bNoTeleFrag() { return BoolValue(PropOffsets_WarpZoneInfo.bNoTeleFrag); }
	int& iWarpZone() { return Value<int>(PropOffsets_WarpZoneInfo.iWarpZone); }
	int& numDestinations() { return Value<int>(PropOffsets_WarpZoneInfo.numDestinations); }
};
