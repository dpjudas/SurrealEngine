
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

void RenderSubsystem::DrawScene()
{
	Scene.Clipper.numDrawSpans = 0;
	Scene.Clipper.numSurfs = 0;
	Scene.Clipper.numTris = 0;

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
	Scene.TranslucentNodes.clear();
	Scene.Actors.clear();
	Scene.Coronas.clear(); // To do: don't do this - make them fade out instead if they don't get refreshed
	Scene.FrameCounter++;
	ProcessNode(&engine->Level->Model->Nodes[0]);

	Device->SetSceneNode(&Scene.Frame);
	for (const DrawNodeInfo& nodeInfo : Scene.OpaqueNodes)
		DrawNodeSurface(nodeInfo);
	DrawDecals(&Scene.Frame);
	for (auto it = Scene.TranslucentNodes.rbegin(); it != Scene.TranslucentNodes.rend(); ++it)
		DrawNodeSurface(*it);
	DrawActors();
}

void RenderSubsystem::DrawActors()
{
	for (UActor* actor : Scene.Actors)
	{
		EDrawType dt = (EDrawType)actor->DrawType();
		if (dt == DT_Mesh && actor->Mesh())
		{
			DrawMesh(&Scene.Frame, actor);
		}
		else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (actor->Texture()))
		{
			DrawSprite(&Scene.Frame, actor);
		}
		else if (dt == DT_Brush && actor->Brush())
		{
			DrawBrush(&Scene.Frame, actor);
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

	FTextureInfo texture;
	if (surface.Material)
	{
		UTexture* tex = surface.Material->GetAnimTexture();
		UpdateTexture(tex);
		texture.CacheID = (uint64_t)(ptrdiff_t)tex;
		texture.bRealtimeChanged = tex->TextureModified;
		texture.UScale = tex->DrawScale();
		texture.VScale = tex->DrawScale();
		texture.Pan.x = -(float)surface.PanU;
		texture.Pan.y = -(float)surface.PanV;
		texture.Texture = tex;
		texture.Format = texture.Texture->ActualFormat;
		texture.Mips = texture.Texture->Mipmaps.data();
		texture.NumMips = (int)texture.Texture->Mipmaps.size();
		texture.USize = texture.Texture->USize();
		texture.VSize = texture.Texture->VSize();
		if (texture.Texture->Palette())
			texture.Palette = (FColor*)texture.Texture->Palette()->Colors.data();

		if (surface.Material->TextureModified)
			surface.Material->TextureModified = false;

		if (PolyFlags & PF_AutoUPan) texture.Pan.x -= AutoUV;
		if (PolyFlags & PF_AutoVPan) texture.Pan.y -= AutoUV;
	}

	FTextureInfo detailtex;
	if (surface.Material && surface.Material->DetailTexture())
	{
		UTexture* tex = surface.Material->DetailTexture()->GetAnimTexture();
		UpdateTexture(tex);
		detailtex.CacheID = (uint64_t)(ptrdiff_t)tex;
		detailtex.bRealtimeChanged = false;
		detailtex.UScale = tex->DrawScale();
		detailtex.VScale = tex->DrawScale();
		detailtex.Pan.x = -(float)surface.PanU;
		detailtex.Pan.y = -(float)surface.PanV;
		detailtex.Texture = tex;
		detailtex.Format = detailtex.Texture->ActualFormat;
		detailtex.Mips = detailtex.Texture->Mipmaps.data();
		detailtex.NumMips = (int)detailtex.Texture->Mipmaps.size();
		detailtex.USize = detailtex.Texture->USize();
		detailtex.VSize = detailtex.Texture->VSize();
		if (detailtex.Texture->Palette())
			detailtex.Palette = (FColor*)detailtex.Texture->Palette()->Colors.data();

		if (PolyFlags & PF_AutoUPan) detailtex.Pan.x -= AutoUV;
		if (PolyFlags & PF_AutoVPan) detailtex.Pan.y -= AutoUV;
	}

	FTextureInfo macrotex;
	if (surface.Material && surface.Material->MacroTexture())
	{
		UTexture* tex = surface.Material->MacroTexture()->GetAnimTexture();
		UpdateTexture(tex);
		macrotex.CacheID = (uint64_t)(ptrdiff_t)tex;
		macrotex.bRealtimeChanged = false;
		macrotex.UScale = tex->DrawScale();
		macrotex.VScale = tex->DrawScale();
		macrotex.Pan.x = -(float)surface.PanU;
		macrotex.Pan.y = -(float)surface.PanV;
		macrotex.Format = macrotex.Texture->ActualFormat;
		macrotex.Texture = tex;
		macrotex.Mips = macrotex.Texture->Mipmaps.data();
		macrotex.NumMips = (int)macrotex.Texture->Mipmaps.size();
		macrotex.USize = macrotex.Texture->USize();
		macrotex.VSize = macrotex.Texture->VSize();
		if (macrotex.Texture->Palette())
			macrotex.Palette = (FColor*)macrotex.Texture->Palette()->Colors.data();

		if (PolyFlags & PF_AutoUPan) macrotex.Pan.x -= AutoUV;
		if (PolyFlags & PF_AutoVPan) macrotex.Pan.y -= AutoUV;
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
		UZoneInfo* zoneActor = !model->Zones.empty() ? static_cast<UZoneInfo*>(model->Zones[node->Zone1].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = GetSurfaceLightmap(surface, facet, zoneActor, model);
		fogmap = GetSurfaceFogmap(surface, facet, engine->CameraActor->Region().Zone, model);
	}

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.Texture = surface.Material ? &texture : nullptr;
	surfaceinfo.MacroTexture = surface.Material && surface.Material->MacroTexture() ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = surface.Material && surface.Material->DetailTexture() ? &detailtex : nullptr;
	surfaceinfo.LightMap = lightmap.NumMips != 0 ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.NumMips != 0 ? &fogmap : nullptr;

	Device->DrawComplexSurface(&Scene.Frame, surfaceinfo, facet);
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

			if (!actor->bHidden() && actor != engine->CameraActor)
			{
				EDrawType dt = (EDrawType)actor->DrawType();
				if (dt == DT_Mesh && actor->Mesh())
				{
					// Note: this doesn't take the rotation into account!
					BBox bbox = actor->Mesh()->BoundingBox;
					vec3 Scale = actor->Mesh()->Scale * actor->DrawScale();
					bbox.min = (bbox.min * Scale) + actor->Location();
					bbox.max = (bbox.max * Scale) + actor->Location();
					if (Scene.Clipper.IsAABBVisible(bbox))
					{
						Scene.Actors.push_back(actor);
					}
				}
				else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (actor->Texture()))
				{
					Scene.Actors.push_back(actor);
				}
				else if (dt == DT_Brush && actor->Brush())
				{
					BBox bbox = actor->Brush()->BoundingBox;
					bbox.min += actor->Location();
					bbox.max += actor->Location();
					if (Scene.Clipper.IsAABBVisible(bbox))
					{
						Scene.Actors.push_back(actor);
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
		Scene.TranslucentNodes.push_back(info);
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
