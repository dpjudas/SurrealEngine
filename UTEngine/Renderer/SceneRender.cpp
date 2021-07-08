
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
	auto level = engine->level;

	if (engine->SkyZoneInfo)
	{
		FSceneNode frame = CreateSkyFrame();
		device->SetSceneNode(&frame);

		FrustumPlanes clip(frame.Projection * frame.Modelview);

		int zone = FindZoneAt(frame.ViewLocation);
		uint64_t zonemask = FindRenderZoneMask(&frame, level->Model->Nodes.front(), clip, zone);

		DrawNode(&frame, level->Model->Nodes[0], clip, zonemask, 0);
		DrawNode(&frame, level->Model->Nodes[0], clip, zonemask, 1);

		device->ClearZ(&frame);
	}

	engine->renderer->canvas.SceneFrame = CreateSceneFrame();
	device->SetSceneNode(&engine->renderer->canvas.SceneFrame);
	auto& SceneFrame = engine->renderer->canvas.SceneFrame;

	FrustumPlanes clip(SceneFrame.Projection * SceneFrame.Modelview);

	int zone = FindZoneAt(SceneFrame.ViewLocation);
	uint64_t zonemask = FindRenderZoneMask(&SceneFrame, level->Model->Nodes.front(), clip, zone);

	DrawNode(&SceneFrame, level->Model->Nodes[0], clip, zonemask, 0);

	for (UActor* actor : level->Actors)
	{
		if (actor && !actor->bHidden())
		{
			ActorDrawType dt = (ActorDrawType)actor->DrawType();
			if (dt == ActorDrawType::Mesh && actor->Mesh())
			{
				BBox bbox = actor->Mesh()->BoundingBox;
				bbox.min += actor->Location();
				bbox.max += actor->Location();
				if (clip.test(bbox) != IntersectionTestResult::outside)
				{
					int actorZone = FindZoneAt(actor->Location());
					if (((uint64_t)1 << actorZone) & zonemask)
						engine->renderer->mesh.DrawMesh(&SceneFrame, actor->Mesh(), actor->Location(), actor->Rotation(), actor->DrawScale(), actorZone);
				}
			}
			else if ((dt == ActorDrawType::Sprite || dt == ActorDrawType::SpriteAnimOnce) && actor->Texture())
			{
				int actorZone = FindZoneAt(actor->Location());
				if (((uint64_t)1 << actorZone) & zonemask)
					engine->renderer->sprite.DrawSprite(&SceneFrame, actor->Texture(), actor->Location(), actor->Rotation(), actor->DrawScale());
			}
			else if (dt == ActorDrawType::Brush && actor->Brush())
			{
				BBox bbox = actor->Brush()->BoundingBox;
				bbox.min += actor->Location();
				bbox.max += actor->Location();
				if (clip.test(bbox) != IntersectionTestResult::outside)
				{
					int actorZone = FindZoneAt(actor->Location());
					if (((uint64_t)1 << actorZone) & zonemask)
						engine->renderer->brush.DrawBrush(&SceneFrame, actor->Brush(), actor->Location(), actor->Rotation(), actor->DrawScale(), actorZone);
				}
			}
		}
	}

	DrawNode(&SceneFrame, level->Model->Nodes[0], clip, zonemask, 1);

	engine->renderer->corona.DrawCoronas(&SceneFrame);

	engine->renderer->framesDrawn++;
	if (engine->renderer->startFPSTime == 0 || engine->lastTime - engine->renderer->startFPSTime >= 1'000'000)
	{
		engine->renderer->fps = engine->renderer->framesDrawn;
		engine->renderer->startFPSTime = engine->lastTime;
		engine->renderer->framesDrawn = 0;
	}

	engine->renderer->canvas.DrawFontTextWithShadow(engine->renderer->canvas.medfont, vec4(1.0f), engine->window->SizeX / engine->renderer->uiscale - 16, 64, std::to_string(engine->renderer->fps) + " FPS", TextAlignment::right);
	engine->renderer->canvas.DrawFontTextWithShadow(engine->renderer->canvas.largefont, vec4(1.0f), engine->window->SizeX / engine->renderer->uiscale / 2, engine->window->SizeY / engine->renderer->uiscale - 100, engine->LevelInfo->GetString("Title"), TextAlignment::center);
}

void SceneRender::DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass)
{
	if ((node.ZoneMask & zonemask) == 0)
		return;

	if (node.RenderBound != -1)
	{
		const BBox& bbox = engine->level->Model->Bounds[node.RenderBound];
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
		DrawNode(frame, engine->level->Model->Nodes[front], clip, zonemask, pass);
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
			DrawNodeSurface(frame, engine->level->Model, *polynode, pass);
#endif

		if (polynode->Plane < 0) break;
		polynode = &engine->level->Model->Nodes[polynode->Plane];
	}

	if (back >= 0)
	{
		DrawNode(frame, engine->level->Model->Nodes[back], clip, zonemask, pass);
	}
}

void SceneRender::DrawNodeSurface(FSceneNode* frame, UModel* model, const BspNode& node, int pass)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return;

	BspSurface& surface = model->Surfaces[node.Surf];

	if (surface.PolyFlags & (PF_Invisible | PF_FakeBackdrop))
		return;

	bool opaqueSurface = (surface.PolyFlags & PF_NoOcclude) == 0;
	if ((pass == 0 && !opaqueSurface) || (pass == 1 && opaqueSurface))
		return;

	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	FTextureInfo texture;
	if (surface.Material)
	{
		texture.CacheID = (uint64_t)(ptrdiff_t)surface.Material;
		texture.bRealtimeChanged = surface.Material->TextureModified;
		texture.UScale = surface.Material->DrawScale();
		texture.VScale = surface.Material->DrawScale();
		texture.Pan.x = -(float)surface.PanU;
		texture.Pan.y = -(float)surface.PanV;
		texture.Texture = surface.Material;

		if (surface.Material->TextureModified)
			surface.Material->TextureModified = false;

		if (surface.PolyFlags & PF_AutoUPan) texture.Pan.x += engine->renderer->AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) texture.Pan.y += engine->renderer->AutoUVTime * 100.0f;
	}

	FTextureInfo detailtex;
	if (surface.Material && surface.Material->DetailTexture())
	{
		detailtex.CacheID = (uint64_t)(ptrdiff_t)surface.Material->DetailTexture();
		detailtex.bRealtimeChanged = false;
		detailtex.UScale = surface.Material->DetailTexture()->DrawScale();
		detailtex.VScale = surface.Material->DetailTexture()->DrawScale();
		detailtex.Pan.x = -(float)surface.PanU;
		detailtex.Pan.y = -(float)surface.PanV;
		detailtex.Texture = surface.Material->DetailTexture();

		if (surface.PolyFlags & PF_AutoUPan) detailtex.Pan.x += engine->renderer->AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) detailtex.Pan.y += engine->renderer->AutoUVTime * 100.0f;
	}

	FTextureInfo macrotex;
	if (surface.Material && surface.Material->MacroTexture())
	{
		macrotex.CacheID = (uint64_t)(ptrdiff_t)surface.Material->MacroTexture();
		macrotex.bRealtimeChanged = false;
		macrotex.UScale = surface.Material->MacroTexture()->DrawScale();
		macrotex.VScale = surface.Material->MacroTexture()->DrawScale();
		macrotex.Pan.x = -(float)surface.PanU;
		macrotex.Pan.y = -(float)surface.PanV;
		macrotex.Texture = surface.Material->MacroTexture();

		if (surface.PolyFlags & PF_AutoUPan) macrotex.Pan.x += engine->renderer->AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) macrotex.Pan.y += engine->renderer->AutoUVTime * 100.0f;
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
	if ((surface.PolyFlags & PF_Unlit) == 0)
	{
		UZoneInfo* zoneActor = !model->Zones.empty() ? dynamic_cast<UZoneInfo*>(model->Zones[node.Zone0].ZoneActor) : nullptr;
		if (!zoneActor)
			zoneActor = engine->LevelInfo;
		lightmap = engine->renderer->light.GetSurfaceLightmap(surface, facet, zoneActor, model);
	}

	//FTextureInfo fogmap = GetSurfaceFogmap(surface);

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = surface.PolyFlags;
	surfaceinfo.Texture = surface.Material ? &texture : nullptr;
	surfaceinfo.MacroTexture = surface.Material && surface.Material->MacroTexture() ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = surface.Material && surface.Material->DetailTexture() ? &detailtex : nullptr;
	surfaceinfo.LightMap = lightmap.Texture ? &lightmap : nullptr;
	surfaceinfo.FogMap = nullptr;// fogmap.Texture ? &fogmap : nullptr;

	engine->window->GetRenderDevice()->DrawComplexSurface(frame, surfaceinfo, facet);
}

int SceneRender::FindZoneAt(vec3 location)
{
	return FindZoneAt(vec4(location, 1.0f), &engine->level->Model->Nodes.front(), engine->level->Model->Nodes.data());
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
		const BBox& bbox = engine->level->Model->Bounds[node.RenderBound];
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
		zonemask |= FindRenderZoneMask(frame, engine->level->Model->Nodes[front], clip, zone);
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
		polynode = &engine->level->Model->Nodes[polynode->Plane];
	}

	if (back >= 0)
	{
		zonemask |= FindRenderZoneMask(frame, engine->level->Model->Nodes[back], clip, zone);
	}

	return zonemask;
}

uint64_t SceneRender::FindRenderZoneMaskForPortal(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int portalzone)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return 0;

	auto level = engine->level;
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
	mat4 rotate = mat4::rotate(radians(engine->Camera.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Yaw), 0.0f, 0.0f, -1.0f);
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

FSceneNode SceneRender::CreateSkyFrame()
{
	mat4 rotate = mat4::rotate(radians(engine->Camera.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->Camera.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 skyrotate = mat4::rotate(radians(engine->SkyZoneInfo->Rotation().RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(engine->SkyZoneInfo->Rotation().PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(engine->SkyZoneInfo->Rotation().YawDegrees()), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - engine->SkyZoneInfo->Location());

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
	frame.ViewLocation = engine->SkyZoneInfo->Location();
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
