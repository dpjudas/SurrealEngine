#pragma once

#include "Packages/Engine/Actors/UActor.h"

class UNavigationPoint : public UActor
{
public:
	using UActor::UActor;

	// Paths() is an array of LevelReachSpec indexes to navigation points that can be reached from this one.
	// upstreamPaths() is the same as Paths(), except this is in reverse order (when searching from a goal back to initially reachable points).
	// PrunedPaths() are reachable points that have been removed from Paths() as they could already be reached via a different path.
	// If -1 is encountered in any of those arrays it means the end of the list.

	int& ExtraCost() { return Value<int>(PropOffsets_NavigationPoint.ExtraCost); }
	FixedArrayView<int, 16> Paths() { return FixedArray<int, 16>(PropOffsets_NavigationPoint.Paths); }
	FixedArrayView<int, 16> PrunedPaths() { return FixedArray<int, 16>(PropOffsets_NavigationPoint.PrunedPaths); }
	UActor*& RouteCache() { return Value<UActor*>(PropOffsets_NavigationPoint.RouteCache); }
	FixedArrayView<UNavigationPoint*, 16> VisNoReachPaths() { return FixedArray<UNavigationPoint*, 16>(PropOffsets_NavigationPoint.VisNoReachPaths); }
	BitfieldBool bAutoBuilt() { return BoolValue(PropOffsets_NavigationPoint.bAutoBuilt); }
	BitfieldBool bEndPoint() { return BoolValue(PropOffsets_NavigationPoint.bEndPoint); }
	BitfieldBool bEndPointOnly() { return BoolValue(PropOffsets_NavigationPoint.bEndPointOnly); }
	BitfieldBool bNeverUseStrafing() { return BoolValue(PropOffsets_NavigationPoint.bNeverUseStrafing); }
	BitfieldBool bOneWayPath() { return BoolValue(PropOffsets_NavigationPoint.bOneWayPath); }
	BitfieldBool bPlayerOnly() { return BoolValue(PropOffsets_NavigationPoint.bPlayerOnly); }
	BitfieldBool bSpecialCost() { return BoolValue(PropOffsets_NavigationPoint.bSpecialCost); }
	BitfieldBool bTwoWay() { return BoolValue(PropOffsets_NavigationPoint.bTwoWay); }
	int& bestPathWeight() { return Value<int>(PropOffsets_NavigationPoint.bestPathWeight); }
	int& cost() { return Value<int>(PropOffsets_NavigationPoint.cost); }
	UNavigationPoint*& nextNavigationPoint() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.nextNavigationPoint); }
	UNavigationPoint*& nextOrdered() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.nextOrdered); }
	NameString& ownerTeam() { return Value<NameString>(PropOffsets_NavigationPoint.ownerTeam); }
	UNavigationPoint*& prevOrdered() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.prevOrdered); }
	UNavigationPoint*& previousPath() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.previousPath); }
	UNavigationPoint*& startPath() { return Value<UNavigationPoint*>(PropOffsets_NavigationPoint.startPath); }
	BitfieldBool taken() { return BoolValue(PropOffsets_NavigationPoint.taken); }
	FixedArrayView<int, 16> upstreamPaths() { return FixedArray<int, 16>(PropOffsets_NavigationPoint.upstreamPaths); }
	int& visitedWeight() { return Value<int>(PropOffsets_NavigationPoint.visitedWeight); }
};
