
#include "Precomp.h"
#include "SceneRender.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "Engine.h"
#include "Window/Window.h"
#include "Math/FrustumPlanes.h"
#include "Renderer/UTRenderer.h"

void SceneRender::DrawScene()
{
	RenderDevice* device = engine->window->GetRenderDevice();
	auto level = engine->Level;

	engine->renderer->canvas.SceneFrame = CreateSceneFrame();

	// Find the skyzone the wrong way: (to do: do a proper visibility analysis on the BSP and then render what we can see)
	UZoneInfo* skyZone = nullptr;
	for (const auto& zone : level->Model->Zones)
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
		cameraZoneActor = skyZone;

		engine->renderer->sceneDrawNumber++;
		FSceneNode frame = CreateSkyFrame(skyZone);
		device->SetSceneNode(&frame);

		FrustumPlanes clip(frame.Projection * frame.Modelview);

		int zone = FindZoneAt(frame.ViewLocation);
		uint64_t zonemask = FindRenderZoneMask(&frame, level->Model->Nodes.front(), clip, zone);

		DrawNode(&frame, level->Model->Nodes[0], clip, zonemask, 0);
		DrawNode(&frame, level->Model->Nodes[0], clip, zonemask, 1);

		device->ClearZ(&frame);
	}

	auto& SceneFrame = engine->renderer->canvas.SceneFrame;
	FrustumPlanes clip(SceneFrame.Projection * SceneFrame.Modelview);

	int zone = FindZoneAt(SceneFrame.ViewLocation);
	uint64_t zonemask = FindRenderZoneMask(&SceneFrame, level->Model->Nodes.front(), clip, zone);

	cameraZoneActor = !level->Model->Zones.empty() ? dynamic_cast<UZoneInfo*>(level->Model->Zones[zone].ZoneActor) : nullptr;
	if (!cameraZoneActor)
		cameraZoneActor = engine->LevelInfo;

	engine->renderer->sceneDrawNumber++;
	device->SetSceneNode(&engine->renderer->canvas.SceneFrame);

	DrawNode(&SceneFrame, level->Model->Nodes[0], clip, zonemask, 0);

	for (UActor* actor : level->Actors)
	{
		if (actor && !actor->bHidden())
		{
			int actorZone = actor->actorZone;
			if (actorZone == -1)
			{
				actorZone = FindZoneAt(actor->Location());
				actor->actorZone = actorZone;
				actor->light = engine->renderer->light.FindLightAt(actor->Location(), actorZone);

			}
			if (((uint64_t)1 << actorZone) & zonemask)
			{
				ActorDrawType dt = (ActorDrawType)actor->DrawType();
				if (dt == ActorDrawType::Mesh && actor->Mesh())
				{
					BBox bbox = actor->Mesh()->BoundingBox;
					bbox.min += actor->Location();
					bbox.max += actor->Location();
					if (clip.test(bbox) != IntersectionTestResult::outside)
					{
						engine->renderer->mesh.DrawMesh(&SceneFrame, actor);
					}
				}
				else if ((dt == ActorDrawType::Sprite || dt == ActorDrawType::SpriteAnimOnce) && actor->Texture())
				{
					engine->renderer->sprite.DrawSprite(&SceneFrame, actor);
				}
				else if (dt == ActorDrawType::Brush && actor->Brush())
				{
					BBox bbox = actor->Brush()->BoundingBox;
					bbox.min += actor->Location();
					bbox.max += actor->Location();
					if (clip.test(bbox) != IntersectionTestResult::outside)
					{
						engine->renderer->brush.DrawBrush(&SceneFrame, actor);
					}
				}
			}
		}
	}

	DrawNode(&SceneFrame, level->Model->Nodes[0], clip, zonemask, 1);

	engine->renderer->corona.DrawCoronas(&SceneFrame);
}

void SceneRender::DrawTimedemoStats()
{
	engine->renderer->framesDrawn++;
	if (engine->renderer->startFPSTime == 0 || engine->lastTime - engine->renderer->startFPSTime >= 1'000'000)
	{
		engine->renderer->fps = engine->renderer->framesDrawn;
		engine->renderer->startFPSTime = engine->lastTime;
		engine->renderer->framesDrawn = 0;
	}

	if (engine->renderer->showTimedemoStats)
	{
		std::string text = std::to_string(engine->renderer->fps) + " FPS";
		float curX = engine->window->SizeX / (float)engine->renderer->uiscale - engine->renderer->canvas.GetTextSize(engine->renderer->canvas.medfont, text).x - 64;
		float curY = 180;
		float curYL = 0.0f;
		engine->renderer->canvas.DrawText(engine->renderer->canvas.largefont, vec4(1.0f), 0.0f, 0.0f, curX, curY, curYL, false, text, PF_NoSmooth | PF_Masked, false);
	}
}

void SceneRender::DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass)
{
	if ((node.ZoneMask & zonemask) == 0)
		return;

	if (node.RenderBound != -1)
	{
		const BBox& bbox = engine->Level->Model->Bounds[node.RenderBound];
		if (clip.test(bbox) == IntersectionTestResult::outside)
			return;
	}

	vec4 plane = { node.PlaneX, node.PlaneY, node.PlaneZ, -node.PlaneW };
	float cameraSide = dot(vec4(frame->ViewLocation, 1.0f), plane);

	int back = node.Back;
	int front = node.Front;

	// Render pass 0 (opaque) front to back and pass 1 (translucent) back to front
	if ((pass == 0 && cameraSide < 0.0f) || (pass == 1 && cameraSide > 0.0f))
		std::swap(front, back);

	if (front >= 0)
	{
		DrawNode(frame, engine->Level->Model->Nodes[front], clip, zonemask, pass);
	}

	const BspNode* polynode = &node;
	while (true)
	{
		int backzone = polynode->Zone0;
		int frontzone = polynode->Zone1;

#if 0 // This only works for 99% of the surfaces :(
		if (cameraSide < 0.0f)
			std::swap(frontzone, backzone);
		if (((uint64_t)1 << frontzone) & zonemask)
			DrawNodeSurface(frame, *polynode, pass);
#else
		if ((((uint64_t)1 << frontzone) & zonemask) || (((uint64_t)1 << backzone) & zonemask))
			DrawNodeSurface(frame, engine->Level->Model, *polynode, pass);
#endif

		if (polynode->Plane < 0) break;
		polynode = &engine->Level->Model->Nodes[polynode->Plane];
	}

	if (back >= 0)
	{
		DrawNode(frame, engine->Level->Model->Nodes[back], clip, zonemask, pass);
	}
}

void SceneRender::DrawNodeSurface(FSceneNode* frame, UModel* model, const BspNode& node, int pass)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return;

	BspSurface& surface = model->Surfaces[node.Surf];

	uint32_t PolyFlags = surface.PolyFlags;

	if (surface.Material)
	{
		// To do: implement packed booleans in the VM so that this can be done as a single uint32_t
		UTexture* t = surface.Material;
		PolyFlags |= ((uint32_t)t->bInvisible()) << 0;
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
	}

	if (PolyFlags & (PF_Invisible | PF_FakeBackdrop))
		return;

	bool opaqueSurface = (PolyFlags & PF_NoOcclude) == 0;
	if ((pass == 0 && !opaqueSurface) || (pass == 1 && opaqueSurface))
		return;

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

		if (surface.Material->TextureModified)
			surface.Material->TextureModified = false;

		if (PolyFlags & PF_AutoUPan) texture.Pan.x += engine->renderer->AutoUV;
		if (PolyFlags & PF_AutoVPan) texture.Pan.y += engine->renderer->AutoUV;
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

		if (PolyFlags & PF_AutoUPan) detailtex.Pan.x += engine->renderer->AutoUV;
		if (PolyFlags & PF_AutoVPan) detailtex.Pan.y += engine->renderer->AutoUV;
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
		macrotex.Texture = tex;

		if (PolyFlags & PF_AutoUPan) macrotex.Pan.x += engine->renderer->AutoUV;
		if (PolyFlags & PF_AutoVPan) macrotex.Pan.y += engine->renderer->AutoUV;
	}

	BspVert* v = &model->Vertices[node.VertPool];

	FSurfaceFacet facet;
	facet.MapCoords.Origin = Base;
	facet.MapCoords.XAxis = UVec;
	facet.MapCoords.YAxis = VVec;

	for (int j = 0; j < node.NumVertices; j++)
	{
		facet.Points.push_back(model->Points[v[j].Vertex]);
	}

	FTextureInfo lightmap;
	FTextureInfo fogmap;
	if ((PolyFlags & PF_Unlit) == 0)
	{
		UZoneInfo* zoneActor = !model->Zones.empty() ? dynamic_cast<UZoneInfo*>(model->Zones[node.Zone0].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = engine->renderer->light.GetSurfaceLightmap(surface, facet, zoneActor, model);
		fogmap = engine->renderer->light.GetSurfaceFogmap(surface, facet, cameraZoneActor, model);
	}

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = PolyFlags;
	surfaceinfo.Texture = surface.Material ? &texture : nullptr;
	surfaceinfo.MacroTexture = surface.Material && surface.Material->MacroTexture() ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = surface.Material && surface.Material->DetailTexture() ? &detailtex : nullptr;
	surfaceinfo.LightMap = lightmap.Texture ? &lightmap : nullptr;
	surfaceinfo.FogMap = fogmap.Texture ? &fogmap : nullptr;

	engine->window->GetRenderDevice()->DrawComplexSurface(frame, surfaceinfo, facet);
}

int SceneRender::FindZoneAt(vec3 location)
{
	return FindZoneAt(vec4(location, 1.0f), &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
}

int SceneRender::FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes)
{
	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float side = dot(location, plane);

	if (node->Front >= 0 && side >= 0.0f)
		return FindZoneAt(location, nodes + node->Front, nodes);
	else if (node->Back >= 0 && side <= 0.0f)
		return FindZoneAt(location, nodes + node->Back, nodes);
	else
		return node->Zone1;
}

uint64_t SceneRender::FindRenderZoneMask(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int zone)
{
	uint64_t zonemask = (uint64_t)1 << zone;

	if ((node.ZoneMask & zonemask) == 0)
		return zonemask;

	if (node.RenderBound != -1)
	{
		const BBox& bbox = engine->Level->Model->Bounds[node.RenderBound];
		if (clip.test(bbox) == IntersectionTestResult::outside)
			return zonemask;
	}

	vec4 plane = { node.PlaneX, node.PlaneY, node.PlaneZ, -node.PlaneW };
	float cameraSide = dot(vec4(frame->ViewLocation, 1.0f), plane);

	int back = node.Back;
	int front = node.Front;

	if (cameraSide < 0.0f)
		std::swap(front, back);

	if (front >= 0)
	{
		zonemask |= FindRenderZoneMask(frame, engine->Level->Model->Nodes[front], clip, zone);
	}

	const BspNode* polynode = &node;
	while (true)
	{
		int backzone = polynode->Zone0;
		int frontzone = polynode->Zone1;
		if (cameraSide < 0.0f)
			std::swap(frontzone, backzone);

		if (frontzone == zone)
			zonemask |= FindRenderZoneMaskForPortal(frame, *polynode, clip, backzone);

		if (polynode->Plane < 0) break;
		polynode = &engine->Level->Model->Nodes[polynode->Plane];
	}

	if (back >= 0)
	{
		zonemask |= FindRenderZoneMask(frame, engine->Level->Model->Nodes[back], clip, zone);
	}

	return zonemask;
}

uint64_t SceneRender::FindRenderZoneMaskForPortal(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int portalzone)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return 0;

	auto level = engine->Level;
	BspSurface& surface = level->Model->Surfaces[node.Surf];

	if ((surface.PolyFlags & PF_Portal) == 0)
		return 0;

	// Check if the portal has already been used
	for (int index : portalsvisited)
	{
		if (index == node.Surf)
			return 0;
	}

	BspVert* v = &level->Model->Vertices[node.VertPool];

	// Check if portal is at all visible
	int vismask = 0;
	for (int j = 0; j < node.NumVertices; j++)
	{
		vec4 point = { level->Model->Points[v[j].Vertex], 1.0f };
		for (int i = 0; i < 6; i++)
		{
			if (dot(clip.planes[i], point) >= 0.0f)
				vismask |= 1 << i;
		}
	}
	if (vismask != 63)
		return 0;

	// Restrict frustum to only include the area covered by the portal:

	FrustumPlanes portalclip;

	vec3 zaxis = clip.planes[0].xyz();
	vec3 yaxis = cross(zaxis, clip.planes[2].xyz());
	vec3 xaxis = cross(zaxis, yaxis);

	// find nearest, leftmost, rightmost, topmost and bottommost points for the surface:
	int behind = 0;
	float nearestZ = 0.0f;
	float nearestdistance = std::numeric_limits<float>::max();
	vec3 points[4];
	float distances[4] = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	for (int j = 0; j < node.NumVertices; j++)
	{
		vec4 point = { level->Model->Points[v[j].Vertex], 1.0f };

		float neardist = dot(clip.planes[0], point);
		if (neardist < nearestdistance)
		{
			nearestdistance = neardist;
			nearestZ = dot(point.xyz(), zaxis);
		}

		if (neardist >= 0.0f)
		{
			for (int i = 0; i < 4; i++)
			{
				float distance = dot(clip.planes[2 + i], point);
				if (distance < distances[i])
				{
					distances[i] = distance;
					points[i] = point.xyz();
				}
			}
		}
		else
		{
			behind++;
		}
	}

	if (behind == node.NumVertices)
		return 0;

	// move the near clipping plane
	if (nearestdistance <= 0.0f)
	{
		portalclip.planes[0] = clip.planes[0];
	}
	else
	{
		portalclip.planes[0] = vec4(zaxis, -dot(zaxis, nearestZ * zaxis));
	}

	// far clipping plane never changes
	portalclip.planes[1] = clip.planes[1];

	// move clipping planes for left/right
	for (int i = 0; i < 2; i++)
	{
		if (distances[i] <= 0.0f)
		{
			portalclip.planes[i + 2] = clip.planes[i + 2];
		}
		else
		{
			float x = dot(points[i], xaxis);
			float z = dot(points[i], zaxis);
			vec3 n = normalize(cross(x * xaxis + z * zaxis, yaxis));
			if (i == 0) n = vec3(0.0f) - n;
			portalclip.planes[i + 2] = vec4(n, -dot(n, frame->ViewLocation));
		}
	}

	// move clipping planes for top/bottom
	for (int i = 2; i < 4; i++)
	{
		if (distances[i] <= 0.0f)
		{
			portalclip.planes[i + 2] = clip.planes[i + 2];
		}
		else
		{
			float y = dot(points[i], yaxis);
			float z = dot(points[i], zaxis);
			vec3 n = normalize(cross(y * yaxis + z * zaxis, xaxis));
			if (i == 2) n = vec3(0.0f) - n;
			portalclip.planes[i + 2] = vec4(n, -dot(n, frame->ViewLocation));
		}
	}

	portalsvisited.push_back(node.Surf);
	uint64_t zonemask = FindRenderZoneMask(frame, level->Model->Nodes.front(), portalclip, portalzone);
	portalsvisited.pop_back();
	return zonemask;
}

FSceneNode SceneRender::CreateSceneFrame()
{
	mat4 rotate = mat4::rotate(radians(engine->Camera.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Yaw - 90.0f), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - engine->Camera.Location);

	FSceneNode frame;
	frame.XB = 0;
	frame.YB = 0;
	frame.X = engine->window->SizeX;
	frame.Y = engine->window->SizeY;
	frame.FX = (float)engine->window->SizeX;
	frame.FY = (float)engine->window->SizeY;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.Modelview = CoordsMatrix() * rotate * translate;
	frame.ViewLocation = engine->Camera.Location;
	frame.FovAngle = 95.0f;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	return frame;
}

FSceneNode SceneRender::CreateSkyFrame(UZoneInfo* skyZone)
{
	mat4 rotate = mat4::rotate(radians(engine->Camera.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 skyrotate = mat4::rotate(radians(skyZone->Rotation().RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(skyZone->Rotation().PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(skyZone->Rotation().YawDegrees()), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - skyZone->Location());

	FSceneNode frame;
	frame.XB = 0;
	frame.YB = 0;
	frame.X = engine->window->SizeX;
	frame.Y = engine->window->SizeY;
	frame.FX = (float)engine->window->SizeX;
	frame.FY = (float)engine->window->SizeY;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.Modelview = CoordsMatrix() * rotate * skyrotate * translate;
	frame.ViewLocation = skyZone->Location();
	frame.FovAngle = 95.0f;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	return frame;
}

mat4 SceneRender::CoordsMatrix()
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
