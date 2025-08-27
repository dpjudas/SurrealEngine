#pragma once

#include "VisibleNode.h"

class BspNode;
class USkyZoneInfo;
class UWarpZoneInfo;

class VisiblePortal
{
public:
	Array<VisibleNode> Nodes;
	USkyZoneInfo* SkyZone = nullptr;
	UWarpZoneInfo* WarpZone = nullptr;
};
