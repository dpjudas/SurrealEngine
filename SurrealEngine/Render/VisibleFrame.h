#pragma once

#include "Engine.h"
#include "VisibleNode.h"
#include "VisibleTranslucent.h"
#include "VisibleCorona.h"
#include "VisibleActor.h"
#include "VisiblePortal.h"
#include "BspClipper.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"
#include "UObject/UClient.h"

class VisibleFrame
{
public:
	// Everything needed to build one view of the level. Sky, warp zone and mirror frames recurse by
	// filling one of these in from the frame they are nested inside (see DrawPortals()).
	struct ViewSetup
	{
		vec3 Location = vec3(0.0f);
		mat4 WorldToView = mat4::identity();
		Coords ViewRotation = {};
		bool MirrorFlag = false;
		int PortalDepth = 0;
		// The spans of the portal this frame is seen through, to clip against. Null for a view that
		// isn't nested inside a portal, i.e. the player's own. Borrowed, not owned: it only has to
		// outlive the Process() call.
		const Array<PortalSpan>* PortalSpans = nullptr;
		vec4 PortalPlane = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// VR only. An asymmetric per-eye frustum that replaces the FOV-derived one, plus the head pose
		// relative to the camera. Left unset outside VR. See RenderSubsystem::DrawScene().
		bool UseProvidedProjection = false;
		mat4 ProvidedProjection = mat4::identity();
		Coords HeadCoords = Coords::Identity();
	};

	void Process(const ViewSetup& setup);
	void Draw();
	void DrawCoronas();

	RenderDevice* Device = nullptr;

	FSceneNode Frame;
	BspClipper Clipper;
	vec4 ViewLocation = vec4(0.0f);
	Coords ViewRotation = {};
	// The VR head pose relative to the camera (identity outside VR). ViewRotation never has this baked
	// in - it stays the plain camera-derived rotation chain so recursive sky/portal/mirror frames can each
	// combine it with the head pose exactly once when building their own view matrix (see DrawPortals()).
	Coords HeadCoords = Coords::Identity();
	int ViewZone = 0;
	//uint64_t ViewZoneMask = 0;
	int FrameCounter = 0;
	bool MirrorFlag = false;
	int PortalDepth = 0;

	Array<VisibleNode> OpaqueNodes;
	Array<VisibleActor> Actors;
	Array<VisibleTranslucent> Translucents;
	Array<VisibleCorona> Coronas;
	Array<VisiblePortal> Portals;

private:
	ViewSetup NestedViewSetup(const VisiblePortal& portal) const;
	void SetupSceneFrame(const ViewSetup& setup);
	void ProcessNode(BspNode* node);
	void ProcessNodeSurface(BspNode* node, bool front);
	void SortTranslucent();

	void DrawOpaqueNodes();
	void DrawOpaqueActors();
	void DrawTranslucent();
	void DrawPortals();

	int FindZoneAt(const vec3& location);
	int FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes);

	vec3 WarpLocationToOtherSide(UWarpZoneInfo* warpZone, vec3 p);
	vec3 WarpNormalToOtherSide(UWarpZoneInfo* warpZone, vec3 n);
	Coords WarpRotationToOtherSide(UWarpZoneInfo* warpZone, Coords rotation);
};
