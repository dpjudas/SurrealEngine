
#include "Precomp.h"
#include "VisibleFrame.h"
#include "VisibleCorona.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/UWindow.h"

void VisibleFrame::Process(const ViewSetup& setup)
{
	engine->render->Stats.Frames++;

	Device = engine->render->Device;
	FrameCounter = engine->render->FrameCounter++;
	MirrorFlag = setup.MirrorFlag;
	PortalDepth = setup.PortalDepth;

	SetupSceneFrame(setup);

	static const Array<PortalSpan> noPortalSpans;

	Clipper.numDrawSpans = 0;
	Clipper.numSurfs = 0;
	Clipper.numTris = 0;
	Clipper.Setup(Frame.Projection * Frame.WorldToView * Frame.ObjectToWorld, setup.PortalSpans ? *setup.PortalSpans : noPortalSpans, setup.PortalPlane);

	ViewLocation = vec4(setup.Location, 1.0f);
	ViewZone = FindZoneAt(setup.Location);
	//ViewZoneMask = ViewZone ? 1ULL << ViewZone : -1;
	ViewRotation = setup.ViewRotation;
	HeadCoords = setup.HeadCoords;

	OpaqueNodes.clear();
	Actors.clear();
	Translucents.clear();
	Coronas.clear();
	Portals.clear();

	ProcessNode(&engine->Level->Model->Nodes[0]);
}

// The parts of a view that any frame seen through `portal` inherits from this one: the spans to clip
// against, the recursion depth, and - unchanged all the way down - the VR eye's projection and head
// pose. Callers fill in the rest, which is what actually differs between a sky, a warp zone and a mirror.
VisibleFrame::ViewSetup VisibleFrame::NestedViewSetup(const VisiblePortal& portal) const
{
	ViewSetup setup;
	setup.MirrorFlag = MirrorFlag;
	setup.PortalDepth = PortalDepth + 1;
	setup.PortalSpans = &portal.Spans;
	setup.UseProvidedProjection = Frame.UseProvidedProjection;
	setup.ProvidedProjection = Frame.Projection;
	setup.HeadCoords = HeadCoords;
	return setup;
}

void VisibleFrame::SetupSceneFrame(const ViewSetup& setup)
{
	// UseProvidedProjection is only set when rendering a VR eye (see RenderSubsystem::DrawScene). The eye's
	// render target is sized to the headset's recommended eye resolution, not the desktop window, so the
	// viewport bookkeeping below must be based on that instead of engine->viewport.
	if (setup.UseProvidedProjection && engine->vr)
	{
		Frame.XB = 0;
		Frame.YB = 0;
		Frame.X = engine->vr->GetRecommendedEyeWidth();
		Frame.Y = engine->vr->GetRecommendedEyeHeight();
		Frame.FX = (float)Frame.X;
		Frame.FY = (float)Frame.Y;
	}
	else
	{
		Frame.XB = engine->viewport->ViewportX();
		Frame.YB = engine->viewport->ViewportY();
		Frame.X = engine->viewport->ViewportWidth();
		Frame.Y = engine->viewport->ViewportHeight();
		Frame.FX = (float)engine->viewport->ViewportWidth();
		Frame.FY = (float)engine->viewport->ViewportHeight();

		if (engine->dxRootWindow && engine->dxRootWindow->RenderViewportSet)
		{
			// DeusEX expected a 4:3 monitor.
			// The unrealscript code assumes that certain aspect ratio calculations would produce cinematic black bars.
			// Unfortunately modern monitors are 16:9 (plus ultrawides), so we lie to the unrealscript code by boxing
			// our window tree in a 4:3 area in the center of the screen.
			// The code below calculates how the black bars would then lie within this area.

			float virtualScale = engine->dxRootWindow->GetVirtualScale();
			Frame.XB = (int)std::round((engine->dxRootWindow->UsedX + engine->dxRootWindow->renderX()) * virtualScale);
			Frame.YB = (int)std::round((engine->dxRootWindow->UsedY + engine->dxRootWindow->renderY()) * virtualScale);
			Frame.FX = engine->dxRootWindow->renderWidth() * virtualScale;
			Frame.FY = engine->dxRootWindow->renderHeight() * virtualScale;
			Frame.X = (int)std::round(Frame.FX);
			Frame.Y = (int)std::round(Frame.FY);
		}
	}

	Frame.FX2 = Frame.FX * 0.5f;
	Frame.FY2 = Frame.FY * 0.5f;
	Frame.ObjectToWorld = mat4::identity();
	Frame.WorldToView = setup.WorldToView;
	Frame.FovAngle = engine->CameraFovAngle;
	Frame.UseProvidedProjection = setup.UseProvidedProjection;
	if (setup.UseProvidedProjection)
	{
		Frame.Projection = setup.ProvidedProjection;
	}
	else
	{
		float Aspect = Frame.FY / Frame.FX;
		float RProjZ = (float)std::tan(radians(Frame.FovAngle) * 0.5f);
		float RFX2 = 2.0f * RProjZ / Frame.FX;
		float RFY2 = 2.0f * RProjZ * Aspect / Frame.FY;
		Frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	}
}

void VisibleFrame::ProcessNode(BspNode* node)
{
	// Skip node if it is not part of the portal zones we have seen so far
	//if ((node->ZoneMask & ViewZoneMask) == 0)
	//	return;

	// Skip node if its AABB is not visible
	if (node->RenderBound != -1 && !Clipper.IsAABBVisible(engine->Level->Model->Bounds[node->RenderBound]))
	{
		return;
	}

	// Add bsp node actors to the visible set
	for (UActor* actor = node->ActorList; actor != nullptr; actor = actor->BspInfo.Next)
	{
		if (actor->LastDrawFrame != FrameCounter)
		{
			actor->LastDrawFrame = FrameCounter;

			VisibleActor visactor;
			visactor.Process(this, actor);
		}
	}

	// Decide which side the plane the camera is
	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	bool swapFrontAndBack = dot(ViewLocation, plane) < 0.0f;
	int back = node->Back;
	int front = node->Front;
	if (swapFrontAndBack)
		std::swap(front, back);

	// Recursively divide front space (toward the viewer)
	if (front >= 0)
	{
		ProcessNode(&engine->Level->Model->Nodes[front]);
	}

	// Draw surfaces on this plane
	BspNode* polynode = node;
	while (true)
	{
		ProcessNodeSurface(polynode, swapFrontAndBack);

		if (polynode->Plane < 0) break;
		polynode = &engine->Level->Model->Nodes[polynode->Plane];
	}

	// Possibly divide back space (away from the viewer)
	if (back >= 0)
	{
		ProcessNode(&engine->Level->Model->Nodes[back]);
	}
}

void VisibleFrame::ProcessNodeSurface(BspNode* node, bool front)
{
	if (node->NumVertices < 3 || node->Surf < 0)
		return;

	UModel* model = engine->Level->Model;
	const BspSurface& surface = model->Surfaces[node->Surf];

	int numverts = node->NumVertices;
	vec3* points = engine->render->GetTempVertexBuffer(numverts);
	BspVert* v = &model->Vertices[node->VertPool];
	if (MirrorFlag)
	{
		for (int j = 0; j < numverts; j++)
		{
			points[numverts - 1 - j] = model->Points[v[j].Vertex];
		}
	}
	else
	{
		for (int j = 0; j < numverts; j++)
		{
			points[j] = model->Points[v[j].Vertex];
		}
	}

	uint32_t PolyFlags = surface.PolyFlags;
	UTexture* texture = surface.Material;
	if (!texture)
		texture = engine->LevelInfo->DefaultTexture();

	if (surface.Material)
		PolyFlags |= surface.Material->PolyFlags();

	VisibleNode info;
	info.Node = node;
	info.Front = front;
	info.PolyFlags = PolyFlags;

	if (PortalDepth < 4)
	{
		if ((PolyFlags & (PF_FakeBackdrop | PF_Invisible)) == PF_FakeBackdrop)
		{
			int zone = front ? node->Zone1 : node->Zone0;
			UZoneInfo* zoneInfo = engine->GetZoneActor(zone);
			if (zoneInfo->SkyZone())
			{
				Array<PortalSpan> spans = Clipper.CheckPortal(points, numverts);
				if (!spans.empty())
				{
					USkyZoneInfo* skyZone = zoneInfo->SkyZone();
					for (auto& p : Portals)
					{
						if (p.SkyZone == skyZone)
						{
							p.Nodes.push_back(info);
							p.Spans.insert(p.Spans.end(), spans.begin(), spans.end());
							return;
						}
					}
					VisiblePortal portal;
					portal.Nodes.push_back(info);
					portal.SkyZone = skyZone;
					portal.Spans = std::move(spans);
					Portals.push_back(std::move(portal));
				}
				return;
			}
		}
		else if (PolyFlags & PF_Portal)
		{
			int portalZone = front ? node->Zone1 : node->Zone0;
			if (portalZone > 0)
			{
				UWarpZoneInfo* warpZone = UObject::TryCast<UWarpZoneInfo>(engine->GetZoneActor(portalZone));
				if (warpZone)
				{
					Array<PortalSpan> spans = Clipper.CheckPortal(points, numverts);
					if (!spans.empty())
					{
						for (auto& p : Portals)
						{
							if (p.WarpZone == warpZone)
							{
								p.Nodes.push_back(info);
								p.Spans.insert(p.Spans.end(), spans.begin(), spans.end());
								return;
							}
						}
						VisiblePortal portal;
						portal.Nodes.push_back(info);
						portal.WarpZone = warpZone;
						portal.Spans = std::move(spans);
						Portals.push_back(std::move(portal));
					}
					return;
				}
			}
		}
		else if (PolyFlags & PF_Mirrored)
		{
			if (PortalDepth > 0) // To do: cull backfacing surfaces so we don't need this hack
				return;

			Array<PortalSpan> spans = Clipper.CheckPortal(points, numverts);
			if (!spans.empty())
			{
				for (auto& p : Portals)
				{
					if (!p.WarpZone && !p.SkyZone) // To do: how do we best merge mirrors using the same plane?
					{
						p.Nodes.push_back(info);
						p.Spans.insert(p.Spans.end(), spans.begin(), spans.end());
						return;
					}
				}

				VisiblePortal portal;
				portal.Nodes.push_back(info);
				portal.Spans = std::move(spans);
				Portals.push_back(std::move(portal));
			}
			return;
		}
	}

	if (!Clipper.CheckSurface(points, numverts, (PolyFlags & PF_NoOcclude) == 0))
		return;

	/*if (PolyFlags & PF_Portal)
	{
		ViewZoneMask |= 1ULL << node->Zone0;
		ViewZoneMask |= 1ULL << node->Zone1;
	}*/

	if (PolyFlags & PF_Invisible)
		return;

	if ((PolyFlags & (PF_Translucent | PF_Modulated)) == 0 && (PolyFlags & PF_Occlude) != PF_Occlude)
	{
		OpaqueNodes.push_back(info);
	}
	else
	{
		UModel* model = engine->Level->Model;
		const BspSurface& surface = model->Surfaces[node->Surf];
		vec3 v = model->Points[surface.pBase] - ViewLocation.xyz();
		Translucents.emplace_back(info, dot(v, v));
	}
}

void VisibleFrame::SortTranslucent()
{
	std::sort(Translucents.begin(), Translucents.end(), [](const VisibleTranslucent& a, const VisibleTranslucent& b) { return a.DistSqr < b.DistSqr; });
}

int VisibleFrame::FindZoneAt(const vec3& location)
{
	return FindZoneAt(vec4(location, 1.0f), &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
}

int VisibleFrame::FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes)
{
	while (true)
	{
		vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
		bool swapFrontAndBack = dot(location, plane) < 0.0f;
		int front = node->Front;
		int back = node->Back;
		if (swapFrontAndBack)
			std::swap(front, back);

		if (front >= 0)
		{
			node = nodes + front;
		}
		else
		{
			return swapFrontAndBack ? node->Zone0 : node->Zone1;
		}
	}
}

void VisibleFrame::Draw()
{
	DrawPortals();
	DrawOpaqueNodes();
	DrawOpaqueActors();
	SortTranslucent();
	DrawTranslucent();
}

void VisibleFrame::DrawCoronas()
{
	FSceneNode frame2d = Frame;
	frame2d.ObjectToWorld = mat4::identity();
	frame2d.WorldToView = mat4::identity();

	// VisibleCorona::Draw() below already projects the light's world position into screen-space x/y itself,
	// using the real (possibly asymmetric, in VR) Frame.Projection. It then hands that x/y to DrawTile(),
	// whose vertex math (RFX2/RFY2 in VulkanRenderDevice::DrawTile) assumes a plain symmetric, screen-centered
	// projection. Reusing the real asymmetric VR projection here would apply that asymmetry a second time on
	// top of the already-correct x/y, shifting the corona sprite - by a different amount per eye, since each
	// eye's asymmetry differs, breaking stereo alignment. Rebuild a symmetric frustum matching what DrawTile expects.
	float aspect = frame2d.FY / frame2d.FX;
	float halfFov = (float)std::tan(radians(frame2d.FovAngle) * 0.5);
	frame2d.Projection = mat4::frustum(-halfFov, halfFov, -aspect * halfFov, aspect * halfFov, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);

	Device->SetSceneNode(&frame2d);

	for (VisibleCorona &corona : Coronas)
		corona.Draw(this);
}

void VisibleFrame::DrawOpaqueNodes()
{
	Device->SetSceneNode(&Frame);
	for (VisibleNode& node : OpaqueNodes)
		node.Draw(this);
}

void VisibleFrame::DrawOpaqueActors()
{
	Device->SetSceneNode(&Frame);
	for (VisibleActor& actor : Actors)
		actor.DrawOpaque(this);
}

void VisibleFrame::DrawTranslucent()
{
	Device->SetSceneNode(&Frame);
	for (VisibleTranslucent& translucent : Translucents)
		translucent.Draw(this);
}

void VisibleFrame::DrawPortals()
{
	for (VisiblePortal& portal : Portals)
	{
		// BspClipper requires the visible spans list to be sorted
		std::sort(portal.Spans.begin(), portal.Spans.end(), [](const PortalSpan& a, const PortalSpan& b) { return a.y != b.y ? a.y < b.y: a.x0 < b.x0; });

		if (portal.SkyZone)
		{
			// ViewRotation never has the VR head pose baked in (see HeadCoords) - insert it here, locally,
			// as a separate matrix term (mirroring exactly how the main view formula combines head and camera
			// rotation - see RenderScene.cpp), so the skybox rotates with head movement like the main view does.
			// Composing HeadCoords into a single Coords with ViewRotation instead would pull the head's
			// *position* into the result too (Coords multiplication combines origins), which a sky - meant to
			// be infinitely far away and rotation-only - must never pick up.
			// The pure ViewRotation (without head) is still what gets passed down for any further nested
			// portals inside the sky, so the head pose only ever gets combined in once per level instead of
			// accumulating.
			mat4 skyToView =
				Coords::ViewToRenderDev().ToMatrix() *
				HeadCoords.Inverse().ToMatrix() *
				ViewRotation.Inverse().ToMatrix() *
				Coords::Rotation(portal.SkyZone->Rotation()).ToMatrix() *
				Coords::Location(portal.SkyZone->Location()).ToMatrix();

			ViewSetup setup = NestedViewSetup(portal);
			setup.Location = portal.SkyZone->Location();
			setup.WorldToView = skyToView;
			setup.ViewRotation = ViewRotation * Coords::Rotation(portal.SkyZone->Rotation());

			VisibleFrame skyframe;
			skyframe.Process(setup);
			Device->SetSceneNode(&skyframe.Frame);
			skyframe.Draw();
			Device->ClearZ();
		}
		else if (portal.WarpZone)
		{
			// Warp camera
			vec3 newLocation = WarpLocationToOtherSide(portal.WarpZone, ViewLocation.xyz());
			Coords rotation = WarpRotationToOtherSide(portal.WarpZone, ViewRotation);
			// Same head-pose handling as the sky case above: combine as a separate matrix term for this level's
			// view matrix (not by composing Coords, which would also pull in the head's position), keeping the
			// pure (head-free) `rotation` for what gets passed down to nested portals.
			mat4 worldToView = Coords::ViewToRenderDev().ToMatrix() * HeadCoords.Inverse().ToMatrix() * rotation.Inverse().ToMatrix() * mat4::translate(-newLocation);

			// Find clipping plane for portal and warp it
			auto node = portal.Nodes.front().Node;
			vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
			bool isFrontfacing = dot(ViewLocation, plane) < 0.0f;
			vec3 p = WarpLocationToOtherSide(portal.WarpZone, plane.xyz() * -plane.w);
			vec3 n = WarpNormalToOtherSide(portal.WarpZone, vec3(node->PlaneX, node->PlaneY, node->PlaneZ));
			vec4 portalPlane(n, -dot(p,n));
			if (!isFrontfacing)
				portalPlane = -portalPlane;

			ViewSetup setup = NestedViewSetup(portal);
			setup.Location = newLocation;
			setup.WorldToView = worldToView;
			setup.ViewRotation = rotation;
			setup.PortalPlane = portalPlane;

			VisibleFrame portalframe;
			portalframe.Process(setup);
			Device->SetSceneNode(&portalframe.Frame);
			portalframe.Draw();
			Device->ClearZ();
		}
		else // Mirror
		{
			UModel* model = engine->Level->Model;
			const BspSurface& surface = model->Surfaces[portal.Nodes.front().Node->Surf];
			vec3 v = model->Points[surface.pBase];
			vec3 n = model->Vectors[surface.vNormal];
			mat4 mirrorRotation = mat4::mirror(n);
			// Frame.WorldToView already has the head pose baked in for the current level (it's either the
			// top-level VR view matrix, or a sky/portal view matrix built above) - no separate head handling needed here.
			mat4 mirrorToView = Frame.WorldToView * mat4::translate(v) * mirrorRotation * mat4::translate(-v);

			ViewSetup setup = NestedViewSetup(portal);
			setup.Location = ViewLocation.xyz();
			setup.WorldToView = mirrorToView;
			setup.ViewRotation = ViewRotation * Coords::FromMatrix(mirrorRotation).Inverse();
			setup.MirrorFlag = !MirrorFlag;

			VisibleFrame mirrorframe;
			mirrorframe.Process(setup);
			Device->SetSceneNode(&mirrorframe.Frame);
			mirrorframe.Draw();
			Device->ClearZ();
		}
	}

	// Seal the portals
	Device->SetSceneNode(&Frame);
	for (VisiblePortal& portal : Portals)
	{
		for (const auto& info : portal.Nodes)
		{
			VisibleNode visnode(info);
			visnode.PolyFlags |= PF_Occlude | PF_Invisible;
			visnode.Front = true;
			visnode.Draw(this);

			if (info.PolyFlags & PF_NoOcclude)
			{
				if ((info.PolyFlags & (PF_Translucent | PF_Modulated)) == 0)
				{
					OpaqueNodes.push_back(info);
				}
				else
				{
					UModel* model = engine->Level->Model;
					const BspSurface& surface = model->Surfaces[info.Node->Surf];
					vec3 v = model->Points[surface.pBase] - ViewLocation.xyz();
					Translucents.emplace_back(info, dot(v, v));
				}
			}
		}
	}
}

vec3 VisibleFrame::WarpLocationToOtherSide(UWarpZoneInfo* warpZone, vec3 p)
{
	// Transform to warp space:
	{
		vec3 origin = warpZone->WarpCoords().Origin;
		mat3 rotate = warpZone->WarpCoords().ToMatrix();
		//mat3 invrotate = mat3::transpose(rotate);
		p = rotate * (p - origin);
	}

	// Transform from warp space:
	{
		vec3 origin = warpZone->OtherSideActor()->WarpCoords().Origin;
		mat3 rotate = warpZone->OtherSideActor()->WarpCoords().ToMatrix();
		mat3 invrotate = mat3::transpose(rotate);
		p = (invrotate * p) + origin;
	}

	return p;
}

vec3 VisibleFrame::WarpNormalToOtherSide(UWarpZoneInfo* warpZone, vec3 n)
{
	// Transform to warp space:
	{
		mat3 rotate = warpZone->WarpCoords().ToMatrix();
		mat3 invrotate = mat3::transpose(rotate);
		n = invrotate * n;
	}

	// Transform from warp space:
	{
		mat3 rotate = warpZone->OtherSideActor()->WarpCoords().ToMatrix();
		//mat3 invrotate = mat3::transpose(rotate);
		n = rotate * n;
	}

	return n;
}

Coords VisibleFrame::WarpRotationToOtherSide(UWarpZoneInfo* warpZone, Coords rotation)
{
	mat3 newRotation = rotation.ToMatrix();

	// Transform to warp space:
	{
		vec3 origin = warpZone->WarpCoords().Origin;
		mat3 rotate = warpZone->WarpCoords().ToMatrix();
		//mat3 invrotate = mat3::transpose(rotate);
		newRotation = rotate * newRotation;
	}

	// Transform from warp space:
	{
		vec3 origin = warpZone->OtherSideActor()->WarpCoords().Origin;
		mat3 rotate = warpZone->OtherSideActor()->WarpCoords().ToMatrix();
		mat3 invrotate = mat3::transpose(rotate);
		newRotation = invrotate * newRotation;
	}

	rotation.XAxis = vec3(newRotation[0], newRotation[1], newRotation[2]);
	rotation.YAxis = vec3(newRotation[3], newRotation[4], newRotation[5]);
	rotation.ZAxis = vec3(newRotation[6], newRotation[7], newRotation[8]);
	return rotation;
}
