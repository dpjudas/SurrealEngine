
#include "Precomp.h"
#include "Engine.h"
#include "File.h"
#include "Package/PackageManager.h"
#include "Package/ObjectStream.h"
#include "UObject/ULevel.h"
#include "UObject/UFont.h"
#include "UObject/UMesh.h"
#include "UObject/UActor.h"
#include "UObject/UTexture.h"
#include "UObject/UMusic.h"
#include "UObject/USound.h"
#include "UObject/UClass.h"
#include "Math/quaternion.h"
#include "Math/FrustumPlanes.h"
#include "Viewport/Viewport.h"
#include "RenderDevice/RenderDevice.h"
#include "Audio/AudioPlayer.h"
#include "Audio/AudioSource.h"
#include <chrono>
#include <set>

Engine::Engine()
{
	packages = std::make_unique<PackageManager>("C:\\Games\\UnrealTournament436");

	bigfont = UObject::Cast<UFont>(packages->GetPackage("Engine")->GetUObject("Font", "BigFont"));
	largefont = UObject::Cast<UFont>(packages->GetPackage("Engine")->GetUObject("Font", "LargeFont"));
	medfont = UObject::Cast<UFont>(packages->GetPackage("Engine")->GetUObject("Font", "MedFont"));
	smallfont = UObject::Cast<UFont>(packages->GetPackage("Engine")->GetUObject("Font", "SmallFont"));

	// auto shockrifle = packages->GetPackage("Botpack")->GetUObject("Class", "ShockRifle");
	// auto nalicow = UObject::Cast<ULodMesh>(packages->GetPackage("UnrealShare")->GetUObject("LodMesh", "NaliCow"));
}

Engine::~Engine()
{
}

void Engine::Run()
{
	viewport = Viewport::Create(this);
	//viewport->OpenWindow(1800, 950, false);
	viewport->OpenWindow(1920, 1080, true);

	LoadMap("DM-Liandri");
	//LoadMap("DM-Codex");
	//LoadMap("DM-Barricade");
	//LoadMap("DM-Deck16][");
	//LoadMap("DM-KGalleon");
	//LoadMap("DM-Turbine");
	//LoadMap("DM-Tempest");
	//LoadMap("DM-Grinder");
	//LoadMap("DM-HyperBlast");
	//LoadMap("CTF-Coret");
	//LoadMap("CTF-Dreary");
	//LoadMap("CTF-Command");
	//LoadMap("CTF-November");
	//LoadMap("CTF-Gauntlet");
	//LoadMap("CTF-EternalCave");
	//LoadMap("CTF-Niven");
	//LoadMap("CTF-Face");

	// GenerateShadowmaps();

	while (!quit)
	{
		Tick(CalcTimeElapsed());

		for (UTexture* tex : Textures)
			tex->Update();

		RenderDevice* device = viewport->GetRenderDevice();
		device->BeginFrame();
		DrawScene();
		device->EndFrame(true);
	}

	viewport->CloseWindow();
	viewport.reset();
}

void Engine::Tick(float timeElapsed)
{
	viewport->Tick();

	AutoUVTime += timeElapsed;

	quaternion viewrotation = normalize(quaternion::euler(radians(-Camera.Pitch), radians(-Camera.Roll), radians(-Camera.Yaw), EulerOrder::yxz));
	vec3 vel = { 0.0f };
	if (Buttons.StrafeLeft) vel.x = 1.0f;
	if (Buttons.StrafeRight) vel.x = -1.0f;
	if (Buttons.Forward) vel.y = 1.0f;
	if (Buttons.Backward) vel.y = -1.0f;
	if (vel != vec3(0.0f))
		vel = normalize(vel);
	vel = inverse(viewrotation) * vel;
	Camera.Location += vel * (timeElapsed * 650.0f);
}

float Engine::CalcTimeElapsed()
{
	using namespace std::chrono;

	uint64_t currentTime = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
	if (lastTime == 0)
		lastTime = currentTime;

	uint64_t deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	return clamp(deltaTime / 1'000'000.0f, 0.0f, 1.0f);
}

void Engine::GenerateShadowmaps()
{
	/*
	RenderDevice* device = viewport->GetRenderDevice();

	device->UpdateSurfaceLights(level->Model->Lights);

	std::vector<std::pair<int, Light*>> updates;
	for (auto& it : lightactors)
	{
		if (it.second)
		{
			device->BeginFrame();
			device->BeginShadowmapUpdate();
			for (int side = 0; side < 6; side++)
			{
				DrawShadowmap(it.first, *it.second, (CubeSide)side);
			}
			device->EndShadowmapUpdate();
			device->EndFrame(false);
			updates.push_back({ it.first, it.second.get() });
		}
	}
	device->UpdateLights(updates);
	*/
}

void Engine::DrawShadowmap(int index, UActor* light, CubeSide side)
{
	RenderDevice* device = viewport->GetRenderDevice();

	device->BeginShadowmapPass();

	FSceneNode frame = CreateShadowmapFrame(light, side);
	device->SetSceneNode(&frame);

	FrustumPlanes clip(frame.Projection * frame.Modelview);
	DrawNode(&frame, level->Model->Nodes[0], clip, 0xffff'ffff'ffff'ffff, 0);
	DrawNode(&frame, level->Model->Nodes[0], clip, 0xffff'ffff'ffff'ffff, 1);

	device->EndShadowmapPass(index * 6 + (int)side);
}

void Engine::DrawCoronas(FSceneNode* frame)
{
	RenderDevice* device = viewport->GetRenderDevice();

	for (UActor* light : Lights)
	{
		if (light && light->bCorona && light->Skin && !TraceAnyHit(light->Location, frame->ViewLocation))
		{
			vec4 pos = frame->Modelview * vec4(light->Location, 1.0f);
			if (pos.z >= 1.0f)
			{
				vec4 clip = frame->Projection * pos;

				float x = frame->FX2 + clip.x / clip.w * frame->FX2;
				float y = frame->FY2 + clip.y / clip.w * frame->FY2;
				float z = 2.0f;

				float width = (float)light->Skin->Mipmaps.front().Width;
				float height = (float)light->Skin->Mipmaps.front().Height;
				float scale = frame->FY / 300.0f;

				vec3 lightcolor = hsbtorgb(light->LightHue * 360.0f / 255.0f, (255 - light->LightSaturation) / 255.0f, 1.0f);

				FTextureInfo texinfo;
				texinfo.CacheID = (uint64_t)(ptrdiff_t)light->Skin;
				texinfo.Texture = light->Skin;
				device->DrawTile(frame, texinfo, x - width * scale * 0.5f, y - height * scale * 0.5f, width * scale, height * scale, 0.0f, 0.0f, width, height, z, vec4(lightcolor, 1.0f), vec4(0.0f), PF_Translucent);
			}
		}
	}
}

void Engine::DrawScene()
{
	RenderDevice* device = viewport->GetRenderDevice();

	device->BeginScenePass();

	if (HasSkyZoneInfo)
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

	FSceneNode frame = CreateSceneFrame();
	device->SetSceneNode(&frame);

	FrustumPlanes clip(frame.Projection * frame.Modelview);

	int zone = FindZoneAt(frame.ViewLocation);
	uint64_t zonemask = FindRenderZoneMask(&frame, level->Model->Nodes.front(), clip, zone);

	DrawNode(&frame, level->Model->Nodes[0], clip, zonemask, 0);

	for (UActor* actor : level->Actors)
	{
		if (actor && !actor->bHidden)
		{
			if (/*actor->DrawType == ActorDrawType::Mesh &&*/ actor->Mesh)
			{
				if (!TraceAnyHit(Camera.Location, actor->Location))
					DrawMesh(&frame, actor->Mesh, actor->Location, actor->Rotation, actor->DrawScale);
			}
			else if (/*actor->DrawType == ActorDrawType::Sprite &&*/ actor->Texture)
			{
				if (!TraceAnyHit(Camera.Location, actor->Location))
					DrawSprite(&frame, actor->Texture, actor->Location, actor->Rotation, actor->DrawScale);
			}
			else if (/*actor->DrawType == ActorDrawType::Brush &&*/ actor->Brush)
			{
				DrawBrush(&frame, actor->Brush, actor->Location, actor->Rotation, actor->DrawScale);
			}
		}
	}

	DrawNode(&frame, level->Model->Nodes[0], clip, zonemask, 1);

	DrawCoronas(&frame);

	framesDrawn++;
	if (startFPSTime == 0 || lastTime - startFPSTime >= 1'000'000)
	{
		fps = framesDrawn;
		startFPSTime = lastTime;
		framesDrawn = 0;
	}

	DrawFontTextWithShadow(&frame, medfont, vec4(1.0f), 1920 - 16, 16, std::to_string(fps) + " FPS", TextAlignment::right);

	if (LevelInfo->HasScalar("Title"))
		DrawFontTextWithShadow(&frame, largefont, vec4(1.0f), 1920 / 2, 1080 - 100, LevelInfo->GetScalar("Title").ValueString, TextAlignment::center);

	device->EndFlash(0.5f, vec4(1.0f, 0.0f, 0.0f, 1.0f));
	device->EndScenePass();
}

int Engine::FindZoneAt(vec3 location)
{
	return FindZoneAt(vec4(location, 1.0f), &level->Model->Nodes.front(), level->Model->Nodes.data());
}

int Engine::FindZoneAt(const vec4& location, BspNode* node, BspNode* nodes)
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

uint64_t Engine::FindRenderZoneMask(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int zone)
{
	uint64_t zonemask = (uint64_t)1 << zone;

	if ((node.ZoneMask & zonemask) == 0)
		return zonemask;

	if (node.RenderBound != -1)
	{
		const BBox& bbox = level->Model->Bounds[node.RenderBound];
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
		zonemask |= FindRenderZoneMask(frame, level->Model->Nodes[front], clip, zone);
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
		polynode = &level->Model->Nodes[polynode->Plane];
	}

	if (back >= 0)
	{
		zonemask |= FindRenderZoneMask(frame, level->Model->Nodes[back], clip, zone);
	}

	return zonemask;
}

uint64_t Engine::FindRenderZoneMaskForPortal(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, int portalzone)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return 0;

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

bool Engine::TraceAnyHit(vec3 from, vec3 to)
{
	return TraceAnyHit(vec4(from, 1.0f), vec4(to, 1.0f), &level->Model->Nodes.front(), level->Model->Nodes.data());
}

bool Engine::TraceAnyHit(const vec4& from, const vec4& to, BspNode* node, BspNode* nodes)
{
	BspNode* polynode = node;
	while (true)
	{
		if (HitTestNodePolygon(from, to, polynode))
			return true;

		if (polynode->Plane < 0) break;
		polynode = nodes + polynode->Plane;
	}

	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);

	if (node->Front >= 0 && (fromSide >= 0.0f || toSide >= 0.0f) && TraceAnyHit(from, to, nodes + node->Front, nodes))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0f || toSide <= 0.0f) && TraceAnyHit(from, to, nodes + node->Back, nodes))
		return true;
	else
		return false;
}

bool Engine::HitTestNodePolygon(const vec4& from, const vec4& to, BspNode* node)
{
	if (node->NumVertices == 0 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && level->Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return false;

	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };

	// Test if plane is actually crossed
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);
	if ((fromSide > 0.0f && toSide > 0.0f) || (fromSide < 0.0f && toSide < 0.0f))
		return false;

	// Find hit on plane
	vec3 rayvec = to.xyz() - from.xyz();
	float t = -(dot(from, plane) / dot(rayvec, plane.xyz()));
	vec3 planehit = from.xyz() + t * rayvec;

	// Project hit onto the plane:
	vec3 xaxis, yaxis;
	vec3 zaxis = plane.xyz();
	vec3 absN = { std::abs(plane.x), std::abs(plane.y), std::abs(plane.z) };
	if (absN.x > absN.z && absN.x > absN.y)
		xaxis = cross(zaxis, { 0.0f, 0.0f, 1.0f });
	else if (absN.y > absN.z && absN.y > absN.x)
		xaxis = cross(zaxis, { 1.0f, 0.0f, 0.0f });
	else
		xaxis = cross(zaxis, { 0.0f, 1.0f, 0.0f });
	yaxis = cross(zaxis, xaxis);

	vec2 planehit2d = { dot(xaxis, planehit), dot(yaxis, planehit) };

	// Edge test all lines in the convex polygon to determine if the point is inside
	BspVert* v = &level->Model->Vertices[node->VertPool];
	vec3* points = level->Model->Points.data();
	int count = node->NumVertices;
	int insidecount = 0;
	for (int i = 0; i < count; i++)
	{
		int j = (i + 1) % count;
		vec3 p0 = points[v[i].Vertex];
		vec3 p1 = points[v[j].Vertex];
		vec2 edgepoint0 = { dot(xaxis, p0), dot(yaxis, p0) };
		vec2 edgepoint1 = { dot(xaxis, p1), dot(yaxis, p1) };
		vec2 edgevec = edgepoint1 - edgepoint0;
		vec2 edgeN = { -edgevec.y, edgevec.x };
		vec3 edgeplane = { edgeN, -dot(edgeN, edgepoint0) };
		float side = dot(edgeplane, { planehit2d, 1.0f });
		if (side <= 0.0f)
			insidecount++;
	}

	return insidecount == count || insidecount == 0;
}

FSceneNode Engine::CreateSceneFrame()
{
	mat4 rotate = mat4::rotate(radians(Camera.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(Camera.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(Camera.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - Camera.Location);

	FSceneNode frame;
	frame.XB = 0;
	frame.YB = 0;
	frame.X = viewport->SizeX;
	frame.Y = viewport->SizeY;
	frame.FX = (float)viewport->SizeX;
	frame.FY = (float)viewport->SizeY;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.Modelview = CoordsMatrix() * rotate * translate;
	frame.ViewLocation = Camera.Location;
	frame.FovAngle = 90.0f;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	return frame;
}

FSceneNode Engine::CreateSkyFrame()
{
	mat4 rotate = mat4::rotate(radians(Camera.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(Camera.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(Camera.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - SkyLocation);

	FSceneNode frame;
	frame.XB = 0;
	frame.YB = 0;
	frame.X = viewport->SizeX;
	frame.Y = viewport->SizeY;
	frame.FX = (float)viewport->SizeX;
	frame.FY = (float)viewport->SizeY;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.Modelview = CoordsMatrix() * rotate * translate;
	frame.ViewLocation = SkyLocation;
	frame.FovAngle = 90.0f;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	return frame;
}

FSceneNode Engine::CreateShadowmapFrame(UActor* light, CubeSide side)
{
	float pitch = 0.0f;
	float yaw = 0.0f;

	switch (side)
	{
	case CubeSide::XPositive: yaw = 90.0f; break;
	case CubeSide::XNegative: yaw = -90.0f; break;
	case CubeSide::YPositive: pitch = 90.0f; break;
	case CubeSide::YNegative: pitch = -90.0f; break;
	case CubeSide::ZPositive: yaw = 0.0f; break;
	case CubeSide::ZNegative: yaw = 180.0f; break;
	}

	mat4 rotate = mat4::rotate(radians(pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(yaw), 0.0f, 0.0f, -1.0f);
	mat4 translate = mat4::translate(vec3(0.0f) - light->Location);

	FSceneNode frame;
	frame.XB = 0;
	frame.YB = 0;
	frame.X = viewport->SizeX;
	frame.Y = viewport->SizeY;
	frame.FX = (float)viewport->SizeX;
	frame.FY = (float)viewport->SizeY;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.Modelview = CoordsMatrix() * rotate * translate;
	frame.ViewLocation = light->Location;
	frame.FovAngle = 90.0f;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	return frame;
}

mat4 Engine::CoordsMatrix()
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

void Engine::DrawNode(FSceneNode* frame, const BspNode& node, const FrustumPlanes& clip, uint64_t zonemask, int pass)
{
	if ((node.ZoneMask & zonemask) == 0)
		return;

	if (node.RenderBound != -1)
	{
		const BBox& bbox = level->Model->Bounds[node.RenderBound];
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
		DrawNode(frame, level->Model->Nodes[front], clip, zonemask, pass);
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
			DrawNodeSurface(frame, level->Model, *polynode, pass);
#endif

		if (polynode->Plane < 0) break;
		polynode = &level->Model->Nodes[polynode->Plane];
	}

	if (back >= 0)
	{
		DrawNode(frame, level->Model->Nodes[back], clip, zonemask, pass);
	}
}

void Engine::DrawNodeSurface(FSceneNode* frame, UModel* model, const BspNode& node, int pass)
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
		texture.UScale = surface.Material->DrawScale;
		texture.VScale = surface.Material->DrawScale;
		texture.Pan.x = -(float)surface.PanU;
		texture.Pan.y = -(float)surface.PanV;
		texture.Texture = surface.Material;

		if (surface.Material->TextureModified)
			surface.Material->TextureModified = false;

		if (surface.PolyFlags & PF_AutoUPan) texture.Pan.x += AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) texture.Pan.y += AutoUVTime * 100.0f;
	}

	FTextureInfo detailtex;
	if (surface.Material && surface.Material->DetailTexture)
	{
		detailtex.CacheID = (uint64_t)(ptrdiff_t)surface.Material->DetailTexture;
		detailtex.bRealtimeChanged = false;
		detailtex.UScale = surface.Material->DetailTexture->DrawScale;
		detailtex.VScale = surface.Material->DetailTexture->DrawScale;
		detailtex.Pan.x = -(float)surface.PanU;
		detailtex.Pan.y = -(float)surface.PanV;
		detailtex.Texture = surface.Material->DetailTexture;

		if (surface.PolyFlags & PF_AutoUPan) detailtex.Pan.x += AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) detailtex.Pan.y += AutoUVTime * 100.0f;
	}

	FTextureInfo macrotex;
	if (surface.Material && surface.Material->MacroTexture)
	{
		macrotex.CacheID = (uint64_t)(ptrdiff_t)surface.Material->MacroTexture;
		macrotex.bRealtimeChanged = false;
		macrotex.UScale = surface.Material->MacroTexture->DrawScale;
		macrotex.VScale = surface.Material->MacroTexture->DrawScale;
		macrotex.Pan.x = -(float)surface.PanU;
		macrotex.Pan.y = -(float)surface.PanV;
		macrotex.Texture = surface.Material->MacroTexture;

		if (surface.PolyFlags & PF_AutoUPan) macrotex.Pan.x += AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) macrotex.Pan.y += AutoUVTime * 100.0f;
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
	if ((surface.PolyFlags & PF_Unlit) == 0 && model == level->Model)
		lightmap = GetSurfaceLightmap(surface, facet);

	//FTextureInfo fogmap = GetSurfaceFogmap(surface);

	FSurfaceInfo surfaceinfo;
	surfaceinfo.PolyFlags = surface.PolyFlags;
	surfaceinfo.Texture = surface.Material ? &texture : nullptr;
	surfaceinfo.MacroTexture = surface.Material && surface.Material->MacroTexture ? &macrotex : nullptr;
	surfaceinfo.DetailTexture = surface.Material && surface.Material->DetailTexture ? &detailtex : nullptr;
	surfaceinfo.LightMap = lightmap.Texture ? &lightmap : nullptr;
	surfaceinfo.FogMap = nullptr;// fogmap.Texture ? &fogmap : nullptr;

	viewport->GetRenderDevice()->DrawComplexSurface(frame, surfaceinfo, facet);
}

void Engine::DrawSprite(FSceneNode* frame, UTexture* texture, const vec3& location, const Rotator& rotation, float drawscale)
{
	auto device = viewport->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.Texture = texture;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;

	float texwidth = (float)texture->Mipmaps.front().Width;
	float texheight = (float)texture->Mipmaps.front().Height;

	drawscale *= 0.5f;

	vec3 xaxis = { texwidth * drawscale, 0.0f, 0.0f };
	vec3 yaxis = { 0.0f, 0.0f, texheight * drawscale };

	quaternion viewrotation = inverse(normalize(quaternion::euler(radians(-Camera.Pitch), radians(-Camera.Roll), radians(-Camera.Yaw), EulerOrder::yxz)));
	xaxis = viewrotation * xaxis;
	yaxis = viewrotation * yaxis;

	GouraudVertex vertices[4];
	vertices[0].Point = location - xaxis - yaxis;
	vertices[0].UV = { 0.0f, 0.0f };
	vertices[0].Light = { 1.0f };
	vertices[1].Point = location + xaxis - yaxis;
	vertices[1].UV = { texwidth, 0.0f };
	vertices[1].Light = { 1.0f };
	vertices[2].Point = location + xaxis + yaxis;
	vertices[2].UV = { texwidth, texheight };
	vertices[2].Light = { 1.0f };
	vertices[3].Point = location - xaxis + yaxis;
	vertices[3].UV = { 0.0f, texheight };
	vertices[3].Light = { 1.0f };
	device->DrawGouraudPolygon(frame, texinfo.Texture ? &texinfo : nullptr, vertices, 4, PF_Translucent); // To do: use the Style property for the polyflags
}

void Engine::DrawBrush(FSceneNode* frame, UModel* brush, const vec3& location, const Rotator& rotation, float drawscale)
{
	FSceneNode brushframe = *frame;

	mat4 rotate = mat4::rotate(radians(rotation.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(rotation.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(rotation.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 ObjectToWorld = mat4::translate(location) * rotate * mat4::scale(drawscale);
	brushframe.Modelview = brushframe.Modelview * ObjectToWorld;

	auto device = viewport->GetRenderDevice();
	device->SetSceneNode(&brushframe);

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurface(&brushframe, brush, node, 0);
	}

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurface(&brushframe, brush, node, 1);
	}

	device->SetSceneNode(frame);
}

void Engine::DrawMesh(FSceneNode* frame, UMesh* mesh, const vec3& location, const Rotator& rotation, float drawscale)
{
	vec3 color = { 3 / 255.0f };

	for (UActor* light : Lights)
	{
		if (light && !light->bCorona)
		{
			vec3 L = light->Location - location;
			float dist2 = dot(L, L);
			float lightRadius = light->LightRadius * 20.0f;
			float lightRadius2 = lightRadius * lightRadius;
			if (dist2 < lightRadius2 && !TraceAnyHit(light->Location, location))
			{
				vec3 lightcolor = hsbtorgb(light->LightHue * 360.0f / 255.0f, (255 - light->LightSaturation) / 255.0f, light->LightBrightness / 255.0f);

				float distanceAttenuation = std::max(1.0f - std::sqrt(dist2) / lightRadius, 0.0f);
				float angleAttenuation = 0.75f; // std::max(dot(normalize(L), N), 0.0f);
				float attenuation = distanceAttenuation * angleAttenuation;
				color += lightcolor * attenuation;
			}
		}
	}

	mat4 rotate = mat4::rotate(radians(rotation.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(rotation.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(rotation.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 RotOrigin = mat4::rotate(radians(mesh->RotOrigin.Roll), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(mesh->RotOrigin.Pitch), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(90.0f - mesh->RotOrigin.Yaw), 0.0f, 0.0f, -1.0f);
	mat4 ObjectToWorld = mat4::translate(location) * rotate * RotOrigin * mat4::scale(mesh->Scale * drawscale) * mat4::translate(vec3(0.0f) - mesh->Origin);

	if (dynamic_cast<USkeletalMesh*>(mesh))
		DrawSkeletalMesh(frame, static_cast<USkeletalMesh*>(mesh), ObjectToWorld, color);
	else if (dynamic_cast<ULodMesh*>(mesh))
		DrawLodMesh(frame, static_cast<ULodMesh*>(mesh), ObjectToWorld, color);
	else
		DrawMesh(frame, mesh, ObjectToWorld, color);
}

void Engine::DrawMesh(FSceneNode* frame, UMesh* mesh, const mat4& ObjectToWorld, vec3 color)
{
}

void Engine::DrawLodMesh(FSceneNode* frame, ULodMesh* mesh, const mat4& ObjectToWorld, vec3 color)
{
	int animFrame = mesh->AnimSeqs.front().StartFrame;
	DrawLodMeshFace(frame, mesh, mesh->Faces, ObjectToWorld, color, mesh->SpecialVerts + animFrame * mesh->FrameVerts);
	DrawLodMeshFace(frame, mesh, mesh->SpecialFaces, ObjectToWorld, color, animFrame * mesh->FrameVerts);
}

void Engine::DrawLodMeshFace(FSceneNode* frame, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, vec3 color, int vertexOffset)
{
	auto device = viewport->GetRenderDevice();

	GouraudVertex vertices[3];
	for (const MeshFace& face : faces)
	{
		const MeshMaterial& material = mesh->Materials[face.MaterialIndex];

		FTextureInfo texinfo;
		texinfo.Texture = mesh->Textures[material.TextureIndex];
		texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;

		float uscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Width : 256) * (1.0f / 255.0f);
		float vscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Height : 256) * (1.0f / 255.0f);

		for (int i = 0; i < 3; i++)
		{
			const MeshWedge& wedge = mesh->Wedges[face.Indices[i]];
			int vertexIndex = wedge.Vertex + vertexOffset;

			vertices[i].Point = (ObjectToWorld * vec4(mesh->Verts[vertexIndex], 1.0f)).xyz();
			vertices[i].UV = { wedge.U * uscale, wedge.V * vscale };
			vertices[i].Light = color;
		}

		device->DrawGouraudPolygon(frame, texinfo.Texture ? &texinfo : nullptr, vertices, 3, material.PolyFlags);
	}
}

void Engine::DrawSkeletalMesh(FSceneNode* frame, USkeletalMesh* mesh, const mat4& ObjectToWorld, vec3 color)
{
	DrawLodMesh(frame, mesh, ObjectToWorld, color);
}

void Engine::DrawFontTextWithShadow(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment)
{
	DrawFontText(frame, font, vec4(0.0f, 0.0f, 0.0f, color.a), x + 2, y + 2, text, alignment);
	DrawFontText(frame, font, color, x, y, text, alignment);
}

void Engine::DrawFontText(FSceneNode* frame, UFont* font, vec4 color, int x, int y, const std::string& text, TextAlignment alignment)
{
	if (alignment != TextAlignment::left)
	{
		ivec2 textsize = GetFontTextSize(font, text);
		if (alignment == TextAlignment::center)
			x -= textsize.x / 2;
		else
			x -= textsize.x;
	}

	RenderDevice* device = viewport->GetRenderDevice();

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)font->pages.front().Texture;
	texinfo.Texture = font->pages.front().Texture;

	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);

		int width = glyph.USize * 2;
		int height = glyph.VSize * 2;
		float StartU = (float)glyph.StartU;
		float StartV = (float)glyph.StartV;
		float USize = (float)glyph.USize;
		float VSize = (float)glyph.VSize;

		device->DrawTile(frame, texinfo, (float)x, (float)y, (float)width, (float)height, StartU, StartV, USize, VSize, 1.5f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

		x += width;
	}
}

ivec2 Engine::GetFontTextSize(UFont* font, const std::string& text)
{
	int x = 0;
	int y = 0;
	for (char c : text)
	{
		FontCharacter glyph = font->GetGlyph(c);

		x += glyph.USize * 2;
		y = std::max(y, glyph.VSize * 2);
	}
	return { x, y };
}

FTextureInfo Engine::GetSurfaceLightmap(BspSurface& surface, const FSurfaceFacet& facet)
{
	if (surface.LightMap < 0)
		return {};

	const LightMapIndex& lmindex = level->Model->LightMap[surface.LightMap];
	std::unique_ptr<UTexture>& lmtexture = lmtextures[surface.LightMap];
	if (!lmtexture)
	{
		lmtexture = CreateLightmapTexture(lmindex, surface);
	}

	FTextureInfo lightmap;
	lightmap.CacheID = (uint64_t)(ptrdiff_t)&surface;
	lightmap.Texture = lmtexture.get();
	lightmap.Pan = { lmindex.PanX, lmindex.PanY };
	lightmap.UScale = lmindex.UScale;
	lightmap.VScale = lmindex.VScale;
	return lightmap;
}

std::unique_ptr<UTexture> Engine::CreateLightmapTexture(const LightMapIndex& lmindex, const BspSurface& surface)
{
	int width = lmindex.UClamp;
	int height = lmindex.VClamp;

	UnrealMipmap lmmip;
	lmmip.Width = width;
	lmmip.Height = height;
	lmmip.Data.resize((size_t)lmmip.Width * lmmip.Height * 4);

	if (lmindex.LightActors >= 0)
	{
		const vec3& N = level->Model->Vectors[surface.vNormal];

		FSurfaceFacet facet;
		facet.MapCoords.Origin = level->Model->Points[surface.pBase];
		facet.MapCoords.XAxis = level->Model->Vectors[surface.vTextureU];
		facet.MapCoords.YAxis = level->Model->Vectors[surface.vTextureV];

		float UDot = dot(facet.MapCoords.XAxis, facet.MapCoords.Origin);
		float VDot = dot(facet.MapCoords.YAxis, facet.MapCoords.Origin);
		float LMUPan = UDot + lmindex.PanX - 0.5f * lmindex.UScale;
		float LMVPan = VDot + lmindex.PanY - 0.5f * lmindex.VScale;
		float LMUMult = 1.0f / lmindex.UScale;
		float LMVMult = 1.0f / lmindex.VScale;

		vec3 p[3] =
		{
			facet.MapCoords.Origin,
			facet.MapCoords.Origin + facet.MapCoords.XAxis,
			facet.MapCoords.Origin + facet.MapCoords.YAxis
		};

		vec2 uv[3];
		for (int j = 0; j < 3; j++)
		{
			uv[j] =
			{
				(dot(facet.MapCoords.XAxis, p[j]) - LMUPan) * LMUMult,
				(dot(facet.MapCoords.YAxis, p[j]) - LMVPan) * LMVMult
			};
		}

		float leftDX = uv[2].x - uv[0].x;
		float leftDY = uv[2].y - uv[0].y;
		float leftStep = leftDX / leftDY;
		float rightDX = uv[2].x - uv[1].x;
		float rightDY = uv[2].y - uv[1].y;
		float rightStep = rightDX / rightDY;

		std::vector<vec3> lightcolors;
		lightcolors.resize((size_t)width * height, vec3(3/255.0f));

		const uint8_t* bits = &level->Model->LightBits[lmindex.DataOffset];
		int bitpos = 0;

		UActor** lightpos = &level->Model->Lights[lmindex.LightActors];
		while (*lightpos)
		{
			UActor* light = *lightpos;

			for (int y = 0; y < height; y++)
			{
				float x0 = uv[0].x + leftStep * (y + 0.5f - uv[0].y) + 0.5f;
				float x1 = uv[1].x + rightStep * (y + 0.5f - uv[1].y) + 0.5f;
				float t0 = (y + 0.5f - uv[0].y) / leftDY;
				float t1 = (y + 0.5f - uv[1].y) / rightDY;
				vec3 p0 = mix(p[0], p[2], t0);
				vec3 p1 = mix(p[1], p[2], t1);
				if (x1 < x0) { std::swap(x0, x1); std::swap(p0, p1); }
				DrawLightmapSpan(&lightcolors[(size_t)y * width], 0, width, x0, x1, p0, p1, light, N, bits, bitpos);
			}

			lightpos++;
		}

		float weights[9] = { 0.125f, 0.25f, 0.125f, 0.25f, 0.50f, 0.25f, 0.125f, 0.25f, 0.125f };

		uint32_t* texels = (uint32_t*)lmmip.Data.data();
		for (int y = 0; y < height; y++)
		{
			vec3* src = &lightcolors[(size_t)y * width];
			for (int x = 0; x < width; x++)
			{
				vec3 color = { 0.0f };
				for (int yy = -1; yy <= 1; yy++)
				{
					int yyy = clamp(y + yy, 0, height - 1) - y;
					for (int xx = -1; xx <= 1; xx++)
					{
						int xxx = clamp(x + xx, 0, width - 1);
						color += src[yyy * width + xxx] * weights[4 + xx + yy * 3];
					}
				}
				color *= 0.5f;

				uint32_t red = (uint32_t)clamp(color.r * 255.0f + 0.5f, 0.0f, 255.0f);
				uint32_t green = (uint32_t)clamp(color.g * 255.0f + 0.5f, 0.0f, 255.0f);
				uint32_t blue = (uint32_t)clamp(color.b * 255.0f + 0.5f, 0.0f, 255.0f);
				uint32_t alpha = 127;
				texels[x] = (alpha << 24) | (red << 16) | (green << 8) | blue;
			}
			texels += width;
		}
	}

	auto lmtexture = std::make_unique<UTexture>();
	lmtexture->Format = TextureFormat::RGBA7;
	lmtexture->Mipmaps.push_back(std::move(lmmip));
	return lmtexture;
}

void Engine::DrawLightmapSpan(vec3* line, int start, int end, float x0, float x1, vec3 p0, vec3 p1, UActor* light, const vec3& N, const uint8_t* bits, int& bitpos)
{
	vec3 lightcolor = hsbtorgb(light->LightHue * 360.0f / 255.0f, (255 - light->LightSaturation) / 255.0f, light->LightBrightness / 255.0f);

	for (int i = start; i < end; i++)
	{
		bool shadowtest = (bits[bitpos >> 3] & (1 << (bitpos & 7))) != 0;
		if (shadowtest)
		{
			float t = (i + 0.5f - x0) / (x1 - x0);
			vec3 point = mix(p0, p1, t);

			vec3 L = light->Location - point;
			float distanceAttenuation = std::max(1.0f - length(L) / (light->LightRadius * 20), 0.0f);
			float angleAttenuation = std::max(dot(normalize(L), N), 0.0f);
			float attenuation = distanceAttenuation * angleAttenuation;
			line[i] += lightcolor * attenuation;
		}

		bitpos++;
	}

	bitpos = (bitpos + 7) / 8 * 8;
}

vec3 Engine::hsbtorgb(double hue, double saturation, double brightness)
{
	double red, green, blue;

	if (saturation == 0)
	{
		red = green = blue = brightness;
	}
	else
	{
		// the color wheel consists of 6 sectors. Figure out which sector you're in.
		double sectorPos = hue / 60.0;
		int sectorNumber = (int)(floor(sectorPos));
		// get the fractional part of the sector
		double fractionalSector = sectorPos - sectorNumber;

		// calculate values for the three axes of the color. 
		double p = brightness * (1.0 - saturation);
		double q = brightness * (1.0 - (saturation * fractionalSector));
		double t = brightness * (1.0 - (saturation * (1 - fractionalSector)));

		// assign the fractional colors to r, g, and b based on the sector the angle is in.
		switch (sectorNumber)
		{
		case 0:
			red = brightness;
			green = t;
			blue = p;
			break;
		case 1:
			red = q;
			green = brightness;
			blue = p;
			break;
		case 2:
			red = p;
			green = brightness;
			blue = t;
			break;
		case 3:
			red = p;
			green = q;
			blue = brightness;
			break;
		case 4:
			red = t;
			green = p;
			blue = brightness;
			break;
		case 5:
			red = brightness;
			green = p;
			blue = q;
			break;
		}
	}

	return vec3((float)red, (float)green, (float)blue);
}


void Engine::Key(Viewport* viewport, std::string key)
{
}

void Engine::InputEvent(Viewport* viewport, EInputKey key, EInputType type, int delta)
{
	switch (key)
	{
	case 'W':
		Buttons.Forward = (type == IST_Press);
		break;
	case 'S':
		Buttons.Backward = (type == IST_Press);
		break;
	case 'A':
		Buttons.StrafeLeft = (type == IST_Press);
		break;
	case 'D':
		Buttons.StrafeRight = (type == IST_Press);
		break;
	case VK_SPACE:
		Buttons.Jump = (type == IST_Press);
		break;
	case VK_SHIFT:
		Buttons.Crouch = (type == IST_Press);
		break;
	case IK_LeftMouse:
		break;
	case IK_MiddleMouse:
		break;
	case IK_RightMouse:
		break;
	case IK_MouseWheelDown:
		break;
	case IK_MouseWheelUp:
		break;
	case IK_MouseX:
		Camera.Yaw += delta * Mouse.SpeedX;
		while (Camera.Yaw < 0.0f) Camera.Yaw += 360.0f;
		while (Camera.Yaw >= 360.0f) Camera.Yaw -= 360.0f;
		break;
	case IK_MouseY:
		Camera.Pitch = clamp(Camera.Pitch + delta * Mouse.SpeedY, -90.0f, 90.0f);
		break;
	}
}

void Engine::SetPause(bool value)
{
}

void Engine::WindowClose(Viewport* viewport)
{
	quit = true;
}

void Engine::LoadMap(const std::string& packageName)
{
	Package* package = packages->GetPackage(packageName);

	LevelSummary = package->GetUObject("LevelSummary", "LevelSummary");
	LevelInfo = package->GetUObject("LevelInfo", "LevelInfo0");
	level = UObject::Cast<ULevel>(package->GetUObject("Level", "MyLevel"));

	std::set<UActor*> lightset;
	for (UActor* light : level->Model->Lights)
	{
		lightset.insert(light);
	}

	for (UActor* light : lightset)
		Lights.push_back(light);

	std::set<UTexture*> textureset;
	for (BspSurface& surf : level->Model->Surfaces)
	{
		if (surf.Material)
		{
			textureset.insert(surf.Material);
			if (surf.Material->DetailTexture)
				textureset.insert(surf.Material->DetailTexture);
			if (surf.Material->MacroTexture)
				textureset.insert(surf.Material->MacroTexture);
		}
	}

	for (UTexture* texture : textureset)
		Textures.push_back(texture);

	for (UObject* actor : level->Actors)
	{
		if (actor && actor->Base)
		{
			if (actor->Base->Name == "PlayerStart")
			{
				if (actor->HasScalar("Location"))
				{
					auto prop = actor->GetScalar("Location");
					Camera.Location = prop.ValueVector;
					Camera.Location.z += 70;
				}
				if (actor->HasScalar("Rotation"))
				{
					auto prop = actor->GetScalar("Rotation");
					Camera.Yaw = prop.ValueRotator.Yaw - 90.0f;
					Camera.Pitch = prop.ValueRotator.Pitch;
					Camera.Roll = prop.ValueRotator.Roll;
				}
			}
			else if (actor->Base->Name == "SkyZoneInfo")
			{
				if (actor->HasScalar("Location"))
				{
					SkyLocation = actor->GetScalar("Location").ValueVector;
				}
				HasSkyZoneInfo = true;
			}
		}
	}

	if (LevelInfo->HasScalar("Song"))
	{
		auto music = UObject::Cast<UMusic>(LevelInfo->GetUObject("Song"));
		audioplayer = AudioPlayer::Create(AudioSource::CreateMod(music->Data));
	}
}
