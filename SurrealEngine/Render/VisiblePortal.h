#pragma once

#include "VisibleNode.h"
#include "BspClipper.h"

class BspNode;
class USkyZoneInfo;
class UWarpZoneInfo;
class VisibleFrame;
enum class StencilPolygonType;

class VisiblePortal
{
public:
	Array<VisibleNode> Nodes;
	Array<PortalSpan> Spans;
	USkyZoneInfo* SkyZone = nullptr;
	UWarpZoneInfo* WarpZone = nullptr;

	void DrawStencil(VisibleFrame* frame, StencilPolygonType type);
};
