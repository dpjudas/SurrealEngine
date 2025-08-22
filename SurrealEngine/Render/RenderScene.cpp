
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include <ranges>

void RenderSubsystem::DrawScene()
{
	Scene.Clipper.numDrawSpans = 0;
	Scene.Clipper.numSurfs = 0;
	Scene.Clipper.numTris = 0;

	if (!engine->Level)
		return;

	// Make sure all actors are at the right location in the BSP
	for (UActor* actor : engine->Level->Actors)
	{
		if (actor)
			actor->UpdateBspInfo();
	}

	// To do: use the zone specified in the surface with the PF_FakeBackdrop PolyFlags
	UZoneInfo* skyZone = nullptr;
	for (const auto& zone : engine->Level->Model->Zones)
	{
		UZoneInfo* zoneInfo = UObject::TryCast<UZoneInfo>(zone.ZoneActor);
		if (zoneInfo && zoneInfo->SkyZone())
		{
			skyZone = zoneInfo->SkyZone();
			break;
		}
	}

	if (skyZone)
	{
		mat4 skyToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Rotation(skyZone->Rotation()).ToMatrix() * Coords::Location(skyZone->Location()).ToMatrix();
		DrawFrame(skyZone->Location(), skyToView);
		Device->ClearZ(&Scene.Frame);
	}

	mat4 worldToView = Coords::ViewToRenderDev().ToMatrix() * Coords::Rotation(engine->CameraRotation).Inverse().ToMatrix() * Coords::Location(engine->CameraLocation).ToMatrix();
	DrawFrame(engine->CameraLocation, worldToView);
	DrawCoronas(&Scene.Frame);
}

void RenderSubsystem::DrawFrame(const vec3& location, const mat4& worldToView)
{
	SetupSceneFrame(worldToView);
	Scene.Clipper.Setup(Scene.Frame.Projection * Scene.Frame.WorldToView * Scene.Frame.ObjectToWorld);
	Scene.ViewLocation = vec4(location, 1.0f);
	Scene.ViewZone = FindZoneAt(location);
	Scene.ViewZoneMask = Scene.ViewZone ? 1ULL << Scene.ViewZone : -1;
	Scene.ViewRotation = Coords::Rotation(engine->CameraRotation);
	Scene.OpaqueNodes.clear();
	Scene.TranslucentNodesAndActors.clear();
	Scene.Actors.clear();
	Scene.Coronas.clear(); // To do: don't do this - make them fade out instead if they don't get refreshed
	Scene.FrameCounter++;
	ProcessNode(&engine->Level->Model->Nodes[0]);
	ProcessTranslucentNodesAndActors();

	Device->SetSceneNode(&Scene.Frame);
	for (const DrawNodeInfo& nodeInfo : Scene.OpaqueNodes)
		DrawNodeSurface(nodeInfo);

	DrawOpaqueActors();

	for (auto& info : Scene.TranslucentNodesAndActors)
	{
		if (info.Actor)
		{
			EDrawType dt = (EDrawType)info.Actor->DrawType();
			if (dt == DT_Mesh && info.Actor->Mesh())
			{
				DrawMesh(&Scene.Frame, info.Actor, false, true);
			}
			else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (info.Actor->Texture()))
			{
				DrawSprite(&Scene.Frame, info.Actor);
			}
			else if (dt == DT_Brush && info.Actor->Brush())
			{
				DrawBrush(&Scene.Frame, info.Actor);
			}
		}
		else
			DrawNodeSurface(info.TranslucentNodeInfo);
	}
}

void RenderSubsystem::ProcessTranslucentNodesAndActors()
{
	std::ranges::sort(Scene.TranslucentNodesAndActors,
	[&](const TranslucentNodeAndActorInfo& info1, const TranslucentNodeAndActorInfo& info2)
		{
		  return length(Scene.ViewLocation - vec4(info1.Location(), 1.0f))
		      > length(Scene.ViewLocation - vec4(info2.Location(), 1.0f));
		}
	);
}

void RenderSubsystem::DrawOpaqueActors()
{
	for (UActor* actor : Scene.Actors)
	{
		EDrawType dt = (EDrawType)actor->DrawType();
		if (dt == DT_Mesh && actor->Mesh())
		{
			// Draw opaque mesh surfaces. If there are any translucent surfaces found, add mesh to the translucent pass
			if (DrawMesh(&Scene.Frame, actor, false, false))
			{
				Scene.TranslucentNodesAndActors.push_back(TranslucentNodeAndActorInfo{ .Actor = actor });
			}
		}
		else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (actor->Texture()))
		{
			// Assume all sprites are translucent for now

			// DrawSprite(&Scene.Frame, actor);
			Scene.TranslucentNodesAndActors.push_back(TranslucentNodeAndActorInfo{ .Actor = actor });
		}
		else if (dt == DT_Brush && actor->Brush())
		{
			// Assume brushes are opaque for now.

			DrawBrush(&Scene.Frame, actor);
			//Scene.TranslucentNodesAndActors.push_back(TranslucentNodeAndActorInfo{ .Actor = actor });
		}
	}
}

void RenderSubsystem::DrawNodeSurface(const DrawNodeInfo& nodeInfo)
{
	UModel* model = engine->Level->Model;
	BspNode* node = nodeInfo.Node;
	BspSurface& surface = model->Surfaces[node->Surf];
	uint32_t PolyFlags = nodeInfo.PolyFlags;

	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	UpdateTexture(surface.Material);

	// Try to find the Zone the surface is in using the corresponding node, to obtain its ZoneInfo actor.
	// Checking for Zone1 first seems to work better, as otherwise the clouds in CTF-LavaGiant remain fast.
	// Might return NULL if there is no corresponding ZoneInfo actor for the given Zone.
	auto zoneInfo = UObject::Cast<UZoneInfo>(model->Zones[node->Zone1].ZoneActor);
	if (!zoneInfo)
		zoneInfo = UObject::Cast<UZoneInfo>(model->Zones[node->Zone0].ZoneActor);

	// If no ZoneInfo is found, use the values from LevelInfo instead.
	float ZoneUPanSpeed = zoneInfo ? zoneInfo->TexUPanSpeed() : engine->LevelInfo->TexUPanSpeed();
	float ZoneVPanSpeed = zoneInfo ? zoneInfo->TexVPanSpeed() : engine->LevelInfo->TexVPanSpeed();

	FTextureInfo texture;
	if (surface.Material)
	{
		UTexture* tex = surface.Material->GetAnimTexture();
		UpdateTexture(tex);
		UpdateTextureInfo(texture, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
	}

	FTextureInfo detailtex;
	if (surface.Material && surface.Material->DetailTexture())
	{
		UTexture* tex = surface.Material->DetailTexture()->GetAnimTexture();
		UpdateTexture(tex);
		UpdateTextureInfo(detailtex, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
	}

	FTextureInfo macrotex;
	if (surface.Material && surface.Material->MacroTexture())
	{
		UTexture* tex = surface.Material->MacroTexture()->GetAnimTexture();
		UpdateTexture(tex);
		UpdateTextureInfo(macrotex, surface, tex, ZoneUPanSpeed, ZoneVPanSpeed);
	}

	int numverts = node->NumVertices;
	vec3* points = GetTempVertexBuffer(numverts);

	BspVert* v = &model->Vertices[node->VertPool];
	for (int j = 0; j < numverts; j++)
	{
		points[j] = model->Points[v[j].Vertex];
	}

	FSurfaceFacet facet;
	facet.MapCoords.Origin = Base;
	facet.MapCoords.XAxis = UVec;
	facet.MapCoords.YAxis = VVec;
	facet.Vertices = points;
	facet.VertexCount = numverts;

	FTextureInfo lightmap;
	FTextureInfo fogmap;
	if ((PolyFlags & PF_Unlit) == 0)
	{
		UZoneInfo* zoneActor = !model->Zones.empty() ? UObject::TryCast<UZoneInfo>(model->Zones[node->Zone1].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = GetSurfaceLightmap(surface, facet, zoneActor, model);
		fogmap = GetSurfaceFogmap(surface, facet, engine->CameraActor->Region().Zone, model);
	}

	if (PolyFlags & PF_Mirrored)
	{
		// We don't support mirrored surfaces right now. Force them to be rendered as opaque
		PolyFlags = 0;
	}

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.Texture = surface.Material ? &texture : nullptr;
	surfaceinfo.MacroTexture = surface.Material && surface.Material->MacroTexture() ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = surface.Material && surface.Material->DetailTexture() ? &detailtex : nullptr;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	Device->DrawComplexSurface(&Scene.Frame, surfaceinfo, facet);

	DrawDecals(&Scene.Frame, node);
}

int RenderSubsystem::FindZoneAt(const vec3& location)
{
	return FindZoneAt(vec4(location, 1.0f), &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
}

int RenderSubsystem::FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes)
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

void RenderSubsystem::ProcessNode(BspNode* node)
{
	// Skip node if it is not part of the portal zones we have seen so far
	if ((node->ZoneMask & Scene.ViewZoneMask) == 0)
		return;

	// Skip node if its AABB is not visible
	if (node->RenderBound != -1 && !Scene.Clipper.IsAABBVisible(engine->Level->Model->Bounds[node->RenderBound]))
	{
		return;
	}

	// Add bsp node actors to the visible set
	for (UActor* actor = node->ActorList; actor != nullptr; actor = actor->BspInfo.Next)
	{
		if (actor->LastDrawFrame != Scene.FrameCounter)
		{
			actor->LastDrawFrame = Scene.FrameCounter;

			if (actor->bCorona())
				Scene.Coronas.push_back(actor);

			if (actor != engine->CameraActor && !actor->bHidden() && (!actor->bOwnerNoSee() || actor->Owner() != engine->CameraActor))
			{
				EDrawType dt = (EDrawType)actor->DrawType();
				if (dt == DT_Mesh && actor->Mesh())
				{
					UMesh* mesh = actor->Mesh();

					Coords rotation = Coords::Rotation(actor->Rotation());
					mat4 objectToWorld = mat4::translate(actor->Location() + actor->PrePivot()) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::scale(actor->DrawScale());
					mat4 meshToWorld = objectToWorld * mesh->meshToObject;
					BBox bbox = mesh->BoundingBox.transform(meshToWorld);
					if (Scene.Clipper.IsAABBVisible(bbox))
					{
						Scene.Actors.push_back(actor);
					}
				}
				else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (actor->Texture()))
				{
					Scene.Actors.push_back(actor);
				}
				else if (dt == DT_Brush && actor->Brush() && actor->Brush()->Nodes.size() > 0)
				{
					UModel* brush = actor->Brush();
					if (UMover* mover = UObject::TryCast<UMover>(actor))
					{
						mat4 objectToWorld = mat4::translate(actor->Location()) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-actor->PrePivot());
						BBox bbox = brush->BoundingBox.transform(objectToWorld);
						if (Scene.Clipper.IsAABBVisible(bbox))
						{
							Scene.Actors.push_back(actor);
						}
					}
				}
			}
		}
	}

	// Decide which side the plane the camera is
	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	bool swapFrontAndBack = dot(Scene.ViewLocation, plane) < 0.0f;
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
		ProcessNodeSurface(polynode);

		if (polynode->Plane < 0) break;
		polynode = &engine->Level->Model->Nodes[polynode->Plane];
	}

	// Possibly divide back space (away from the viewer)
	if (back >= 0)
	{
		ProcessNode(&engine->Level->Model->Nodes[back]);
	}
}

void RenderSubsystem::ProcessNodeSurface(BspNode* node)
{
	if (node->NumVertices <= 0 || node->Surf < 0)
		return;

	UModel* model = engine->Level->Model;
	const BspSurface& surface = model->Surfaces[node->Surf];

	int numverts = node->NumVertices;
	vec3* points = GetTempVertexBuffer(numverts);
	BspVert* v = &model->Vertices[node->VertPool];
	for (int j = 0; j < numverts; j++)
	{
		points[j] = model->Points[v[j].Vertex];
	}

	uint32_t PolyFlags = surface.PolyFlags;
	UTexture* texture = surface.Material;
	if (!texture)
		texture = engine->LevelInfo->DefaultTexture();

	bool opaqueSurface = ((PolyFlags & PF_NoOcclude) == 0) &&
		!texture->bMasked() && !texture->bTransparent() && !texture->bModulate();

	if (!Scene.Clipper.CheckSurface(points, numverts, opaqueSurface))
		return;

	if (surface.Material)
		PolyFlags |= surface.Material->PolyFlags();

	if (PolyFlags & PF_Portal)
	{
		Scene.ViewZoneMask |= 1ULL << node->Zone0;
		Scene.ViewZoneMask |= 1ULL << node->Zone1;
	}

	if (PolyFlags & PF_FakeBackdrop)
	{
		PolyFlags |= PF_Invisible;
	}
	else if (PolyFlags & PF_Invisible)
	{
		return;
	}

	DrawNodeInfo info;
	info.Node = node;
	info.PolyFlags = PolyFlags;

	if (opaqueSurface)
	{
		Scene.OpaqueNodes.push_back(info);
	}
	else
	{
		//Scene.TranslucentNodes.push_back(info);
		Scene.TranslucentNodesAndActors.push_back(TranslucentNodeAndActorInfo{
				.Actor = nullptr,
				.TranslucentNodeInfo = info
			}
		);
	}
}

void RenderSubsystem::SetupSceneFrame(const mat4& worldToView)
{
	Scene.Frame.XB = engine->ViewportX;
	Scene.Frame.YB = engine->ViewportY;
	Scene.Frame.X = engine->ViewportWidth;
	Scene.Frame.Y = engine->ViewportHeight;
	Scene.Frame.FX = (float)engine->ViewportWidth;
	Scene.Frame.FY = (float)engine->ViewportHeight;
	Scene.Frame.FX2 = Scene.Frame.FX * 0.5f;
	Scene.Frame.FY2 = Scene.Frame.FY * 0.5f;
	Scene.Frame.ObjectToWorld = mat4::identity();
	Scene.Frame.WorldToView = worldToView;
	Scene.Frame.FovAngle = engine->CameraFovAngle;
	float Aspect = Scene.Frame.FY / Scene.Frame.FX;
	float RProjZ = (float)std::tan(radians(Scene.Frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / Scene.Frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / Scene.Frame.FY;
	Scene.Frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
}
