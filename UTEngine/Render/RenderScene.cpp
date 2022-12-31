
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

void RenderSubsystem::DrawScene()
{
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
		mat4 rotate = mat4::rotate(radians(engine->CameraRotation.RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->CameraRotation.PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->CameraRotation.YawDegrees()), 0.0f, 0.0f, -1.0f);
		mat4 skyrotate = skyZone->Rotation().ToMatrix();
		mat4 translate = mat4::translate(vec3(0.0f) - skyZone->Location());
		mat4 worldToView = CoordsMatrix() * rotate * skyrotate * translate;
		DrawFrame(skyZone->Location(), worldToView);
		Device->ClearZ(&Scene.Frame);
	}

	mat4 rotate = mat4::rotate(radians(engine->CameraRotation.RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->CameraRotation.PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->CameraRotation.YawDegrees() - 90.0f), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - engine->CameraLocation);
	mat4 worldToView = CoordsMatrix() * rotate * translate;
	DrawFrame(engine->CameraLocation, worldToView);
	DrawCoronas(&Scene.Frame);
}

void RenderSubsystem::DrawFrame(const vec3& location, const mat4& worldToView)
{
	SetupSceneFrame(worldToView);
	Scene.FrustumClip = FrustumPlanes(Scene.Frame.Projection * Scene.Frame.WorldToView * Scene.Frame.ObjectToWorld);
	Scene.ViewLocation = vec4(location, 1.0f);
	Scene.ViewZone = FindZoneAt(location);
	Scene.ViewZoneMask = 1ULL << Scene.ViewZone;
	Scene.OpaqueNodes.clear();
	Scene.TranslucentNodes.clear();
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
	Corona.Lights.clear(); // To do: don't do this - make them fade out instead if they don't get refreshed

	// To do: only draw the actors currently visible

	for (UActor* actor : engine->Level->Actors)
	{
		if (!actor)
			continue;

		if (actor->bCorona())
			Corona.Lights.push_back(actor);

		if (!actor->bHidden() && actor != engine->CameraActor)
		{
			if (!actor->lightsCalculated)
			{
				actor->lightsCalculated = true;
				if (!actor->bUnlit())
					actor->light = FindLightAt(actor->Location(), actor->Region().ZoneNumber);
				else
					actor->light = vec3(1.0f);
			}

			ActorDrawType dt = (ActorDrawType)actor->DrawType();
			if (dt == ActorDrawType::Mesh && actor->Mesh())
			{
				// Note: this doesn't take the rotation into account!
				BBox bbox = actor->Mesh()->BoundingBox;
				bbox.min = bbox.min * actor->Mesh()->Scale + actor->Location();
				bbox.max = bbox.max * actor->Mesh()->Scale + actor->Location();
				if (Scene.FrustumClip.test(bbox) != IntersectionTestResult::outside)
				{
					DrawMesh(&Scene.Frame, actor);
				}
			}
			else if ((dt == ActorDrawType::Sprite || dt == ActorDrawType::SpriteAnimOnce) && (actor->Texture() || actor->Sprite()))
			{
				DrawSprite(&Scene.Frame, actor);
			}
			else if (dt == ActorDrawType::Brush && actor->Brush())
			{
				BBox bbox = actor->Brush()->BoundingBox;
				bbox.min += actor->Location();
				bbox.max += actor->Location();
				if (Scene.FrustumClip.test(bbox) != IntersectionTestResult::outside)
				{
					DrawBrush(&Scene.Frame, actor);
				}
			}
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

	FTextureInfo texture;
	if (surface.Material)
	{
		UTexture* tex = surface.Material->GetAnimTexture();
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

		if (PolyFlags & PF_AutoUPan) texture.Pan.x += AutoUV;
		if (PolyFlags & PF_AutoVPan) texture.Pan.y += AutoUV;
	}

	FTextureInfo detailtex;
	if (surface.Material && surface.Material->DetailTexture())
	{
		UTexture* tex = surface.Material->DetailTexture()->GetAnimTexture();
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

		if (PolyFlags & PF_AutoUPan) detailtex.Pan.x += AutoUV;
		if (PolyFlags & PF_AutoVPan) detailtex.Pan.y += AutoUV;
	}

	FTextureInfo macrotex;
	if (surface.Material && surface.Material->MacroTexture())
	{
		UTexture* tex = surface.Material->MacroTexture()->GetAnimTexture();
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

		if (PolyFlags & PF_AutoUPan) macrotex.Pan.x += AutoUV;
		if (PolyFlags & PF_AutoVPan) macrotex.Pan.y += AutoUV;
	}

	BspVert* v = &model->Vertices[node->VertPool];

	FSurfaceFacet facet;
	facet.MapCoords.Origin = Base;
	facet.MapCoords.XAxis = UVec;
	facet.MapCoords.YAxis = VVec;

	std::vector<vec3> points;
	int numverts = node->NumVertices;
	for (int j = 0; j < numverts; j++)
	{
		points.push_back(model->Points[v[j].Vertex]);
	}
	facet.Polys.push_back(std::move(points));

	FTextureInfo lightmap;
	FTextureInfo fogmap;
	if ((PolyFlags & PF_Unlit) == 0)
	{
		UZoneInfo* zoneActor = !model->Zones.empty() ? dynamic_cast<UZoneInfo*>(model->Zones[node->Zone1].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = GetSurfaceLightmap(surface, facet, zoneActor, model);
		//fogmap = GetSurfaceFogmap(surface, facet, cameraZoneActor, model);
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
	if ((node->ZoneMask & Scene.ViewZoneMask) == 0)
		return;

	// Skip node if its AABB is not visible
	if (node->RenderBound != -1)
	{
		const BBox& bbox = engine->Level->Model->Bounds[node->RenderBound];
		if (Scene.FrustumClip.test(bbox) == IntersectionTestResult::outside)
			return;
	}

	// Decide which side the view point is on
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

void RenderSubsystem::ProcessNodeSurface(BspNode* node, bool swapFrontAndBack)
{
	if (node->NumVertices <= 0 || node->Surf < 0)
		return;

	UModel* model = engine->Level->Model;
	const BspSurface& surface = model->Surfaces[node->Surf];

	uint32_t PolyFlags = surface.PolyFlags;
	if (surface.Material)
		PolyFlags |= GetTexturePolyFlags(surface.Material);

	if (PolyFlags & PF_Portal)
	{
		int zone = swapFrontAndBack ? node->Zone1 : node->Zone0;
		Scene.ViewZoneMask |= 1ULL << zone;
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

	bool opaqueSurface = (PolyFlags & PF_NoOcclude) == 0;
	if (opaqueSurface)
	{
		Scene.OpaqueNodes.push_back(info);
	}
	else
	{
		Scene.TranslucentNodes.push_back(info);
	}
}

uint32_t RenderSubsystem::GetTexturePolyFlags(UTexture* t)
{
	// To do: implement packed booleans in the VM so that this can be done as a single uint32_t
	uint32_t PolyFlags = ((uint32_t)t->bInvisible()) << 0;
	PolyFlags |= ((uint32_t)t->bMasked()) << 1;
	PolyFlags |= ((uint32_t)t->bTransparent()) << 2;
	PolyFlags |= ((uint32_t)t->bNotSolid()) << 3;
	PolyFlags |= ((uint32_t)t->bEnvironment()) << 4;
	PolyFlags |= ((uint32_t)t->bSemisolid()) << 5;
	PolyFlags |= ((uint32_t)t->bModulate()) << 6;
	PolyFlags |= ((uint32_t)t->bFakeBackdrop()) << 7;
	PolyFlags |= ((uint32_t)t->bTwoSided()) << 8;
	PolyFlags |= ((uint32_t)t->bAutoUPan()) << 9;
	PolyFlags |= ((uint32_t)t->bAutoVPan()) << 10;
	PolyFlags |= ((uint32_t)t->bNoSmooth()) << 11;
	PolyFlags |= ((uint32_t)t->bBigWavy()) << 12;
	PolyFlags |= ((uint32_t)t->bSmallWavy()) << 13;
	PolyFlags |= ((uint32_t)t->bWaterWavy()) << 14;
	PolyFlags |= ((uint32_t)t->bLowShadowDetail()) << 15;
	PolyFlags |= ((uint32_t)t->bNoMerge()) << 16;
	PolyFlags |= ((uint32_t)t->bCloudWavy()) << 17;
	PolyFlags |= ((uint32_t)t->bDirtyShadows()) << 18;
	PolyFlags |= ((uint32_t)t->bHighLedge()) << 19;
	PolyFlags |= ((uint32_t)t->bSpecialLit()) << 20;
	PolyFlags |= ((uint32_t)t->bGouraud()) << 21;
	PolyFlags |= ((uint32_t)t->bUnlit()) << 22;
	PolyFlags |= ((uint32_t)t->bHighShadowDetail()) << 23;
	PolyFlags |= ((uint32_t)t->bPortal()) << 24;
	PolyFlags |= ((uint32_t)t->bMirrored()) << 25;
	PolyFlags |= ((uint32_t)t->bX2()) << 26;
	PolyFlags |= ((uint32_t)t->bX3()) << 27;
	PolyFlags |= ((uint32_t)t->bX4()) << 28;
	PolyFlags |= ((uint32_t)t->bX5()) << 29;
	PolyFlags |= ((uint32_t)t->bX6()) << 30;
	PolyFlags |= ((uint32_t)t->bX7()) << 31;
	return PolyFlags;
}

void RenderSubsystem::SetupSceneFrame(const mat4& worldToView)
{
	Scene.Frame.XB = 0;
	Scene.Frame.YB = 0;
	Scene.Frame.X = engine->window->SizeX;
	Scene.Frame.Y = engine->window->SizeY;
	Scene.Frame.FX = (float)engine->window->SizeX;
	Scene.Frame.FY = (float)engine->window->SizeY;
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

mat4 RenderSubsystem::CoordsMatrix()
{
	FCoords coords;
	coords.XAxis = vec3(-1.0f, 0.0f, 0.0f);
	coords.YAxis = vec3(0.0f, 0.0f, 1.0f);
	coords.ZAxis = vec3(0.0f, -1.0f, 0.0f);

	mat4 coordsmatrix = mat4::null();
	coordsmatrix[0] = coords.XAxis[0];
	coordsmatrix[1] = coords.XAxis[1];
	coordsmatrix[2] = coords.XAxis[2];
	coordsmatrix[4] = coords.YAxis[0];
	coordsmatrix[5] = coords.YAxis[1];
	coordsmatrix[6] = coords.YAxis[2];
	coordsmatrix[8] = coords.ZAxis[0];
	coordsmatrix[9] = coords.ZAxis[1];
	coordsmatrix[10] = coords.ZAxis[2];
	coordsmatrix[15] = 1.0f;
	return coordsmatrix;
}
