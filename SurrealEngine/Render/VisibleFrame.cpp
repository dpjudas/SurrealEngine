
#include "Precomp.h"
#include "VisibleFrame.h"
#include "VisibleCorona.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"

void VisibleFrame::Process(const vec3& location, const mat4& worldToView)
{
	Device = engine->render->Device;
	FrameCounter = engine->render->FrameCounter++;

	SetupSceneFrame(worldToView);

	Clipper.numDrawSpans = 0;
	Clipper.numSurfs = 0;
	Clipper.numTris = 0;
	Clipper.Setup(Frame.Projection * Frame.WorldToView * Frame.ObjectToWorld);

	ViewLocation = vec4(location, 1.0f);
	ViewZone = FindZoneAt(location);
	ViewZoneMask = ViewZone ? 1ULL << ViewZone : -1;
	ViewRotation = Coords::Rotation(engine->CameraRotation);

	OpaqueNodes.clear();
	Actors.clear();
	Translucents.clear();
	Coronas.clear();
	Portals.clear();

	ProcessNode(&engine->Level->Model->Nodes[0]);
}

void VisibleFrame::SetupSceneFrame(const mat4& worldToView)
{
	Frame.XB = engine->ViewportX;
	Frame.YB = engine->ViewportY;
	Frame.X = engine->ViewportWidth;
	Frame.Y = engine->ViewportHeight;
	Frame.FX = (float)engine->ViewportWidth;
	Frame.FY = (float)engine->ViewportHeight;
	Frame.FX2 = Frame.FX * 0.5f;
	Frame.FY2 = Frame.FY * 0.5f;
	Frame.ObjectToWorld = mat4::identity();
	Frame.WorldToView = worldToView;
	Frame.FovAngle = engine->CameraFovAngle;
	float Aspect = Frame.FY / Frame.FX;
	float RProjZ = (float)std::tan(radians(Frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / Frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / Frame.FY;
	Frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
}

void VisibleFrame::ProcessNode(BspNode* node)
{
	// Skip node if it is not part of the portal zones we have seen so far
	if ((node->ZoneMask & ViewZoneMask) == 0)
		return;

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

			if (actor->bCorona())
				Coronas.push_back(actor);

			if (actor != engine->CameraActor && !actor->bHidden() && (!actor->bOwnerNoSee() || actor->Owner() != engine->CameraActor))
			{
				if (Clipper.IsAABBVisible(actor->BspInfo.BoundingBox))
				{
					VisibleActor visactor;
					visactor.Process(this, actor);
				}
			}
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
	if (node->NumVertices <= 0 || node->Surf < 0)
		return;

	UModel* model = engine->Level->Model;
	const BspSurface& surface = model->Surfaces[node->Surf];

	int numverts = node->NumVertices;
	vec3* points = engine->render->GetTempVertexBuffer(numverts);
	BspVert* v = &model->Vertices[node->VertPool];
	for (int j = 0; j < numverts; j++)
	{
		points[j] = model->Points[v[j].Vertex];
	}

	uint32_t PolyFlags = surface.PolyFlags;
	UTexture* texture = surface.Material;
	if (!texture)
		texture = engine->LevelInfo->DefaultTexture();

	if (surface.Material)
		PolyFlags |= surface.Material->PolyFlags();

	VisibleNode info;
	info.Node = node;
	info.PolyFlags = PolyFlags;

	if (PolyFlags & PF_FakeBackdrop)
	{
		int zone = front ? node->Zone0 : node->Zone1;
		if (UZoneInfo* zoneInfo = UObject::TryCast<UZoneInfo>(model->Zones[zone].ZoneActor))
		{
			if (zoneInfo->SkyZone())
			{
				if (Clipper.CheckSurface(points, numverts, true))
				{
					USkyZoneInfo* skyZone = zoneInfo->SkyZone();
					for (auto& p : Portals)
					{
						if (p.SkyZone == skyZone)
						{
							p.Nodes.push_back(info);
							return;
						}
					}
					VisiblePortal portal;
					portal.Nodes.push_back(info);
					portal.SkyZone = skyZone;
					Portals.push_back(portal);
					return;
				}
			}
		}
	}
	else if (PolyFlags & PF_Portal)
	{
		int portalZone = front ? node->Zone1 : node->Zone0;
		if (portalZone > 0)
		{
			UWarpZoneInfo* warpZone = UObject::TryCast<UWarpZoneInfo>(model->Zones[portalZone].ZoneActor);
			if (warpZone)
			{
				if (Clipper.CheckSurface(points, numverts, true))
				{
					for (auto& p : Portals)
					{
						if (p.WarpZone == warpZone)
						{
							p.Nodes.push_back(info);
							return;
						}
					}
					VisiblePortal portal;
					portal.Nodes.push_back(info);
					portal.WarpZone = warpZone;
					Portals.push_back(portal);
					return;
				}
			}
		}
	}
	else if (PolyFlags & PF_Mirrored)
	{
		if (Clipper.CheckSurface(points, numverts, true))
		{
			// To do: how to merge mirror surfaces? Use the plane?
			VisiblePortal portal;
			portal.Nodes.push_back(info);
			Portals.push_back(portal);
			return;
		}
	}

	if (!Clipper.CheckSurface(points, numverts, (PolyFlags & PF_NoOcclude) == 0))
		return;

	if (PolyFlags & PF_Portal)
	{
		ViewZoneMask |= 1ULL << node->Zone0;
		ViewZoneMask |= 1ULL << node->Zone1;
	}

	if (PolyFlags & PF_Invisible)
		return;

	if ((PolyFlags & (PF_Translucent | PF_Modulated)) == 0)
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
		if (portal.SkyZone)
		{
			mat4 skyToView =
				Coords::ViewToRenderDev().ToMatrix() *
				Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() *
				Coords::Rotation(portal.SkyZone->Rotation()).ToMatrix() *
				Coords::Location(portal.SkyZone->Location()).ToMatrix();

			VisibleFrame skyframe;
			skyframe.Process(portal.SkyZone->Location(), skyToView);
			Device->SetSceneNode(&skyframe.Frame);
			skyframe.Draw();
			Device->ClearZ();
		}
		else if (portal.WarpZone)
		{
		}
		else // Mirror
		{
		}
	}

	// Seal the portals
	Device->SetSceneNode(&Frame);
	for (VisiblePortal& portal : Portals)
	{
		for (auto info : portal.Nodes)
		{
			info.PolyFlags = PF_Occlude | PF_Invisible;

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
