#pragma once

#include "VisibleNode.h"
#include "BspClipper.h"

class BspNode;
class USkyZoneInfo;
class UWarpZoneInfo;

class VisiblePortal
{
public:
	Array<VisibleNode> Nodes;
	Array<PortalSpan> Spans;
	USkyZoneInfo* SkyZone = nullptr;
	UWarpZoneInfo* WarpZone = nullptr;
};
