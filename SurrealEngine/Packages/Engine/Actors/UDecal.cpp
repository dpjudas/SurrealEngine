
#include "Precomp.h"
#include "UDecal.h"
#include "Collision/TopLevel/CollisionSystem.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

UObject* UDecal::AttachDecal(float traceDistance, vec3 decalDir)
{
	if (!Texture())
		return nullptr;

	vec3 traceDirection = -Coords::Rotation(Rotation()).XAxis;

	CollisionHitList hits = XLevel()->Collision.TraceDecal(to_dvec3(Location()), 0.0f, to_dvec3(traceDirection), traceDistance, false);
	if (hits.empty()) return nullptr;

	UModel* model = XLevel()->Model;

	// Do not attempt to create a decal if we hit a surface that's invisible or a fake backdrop
	auto& hit = hits.front();
	if (!hit.Node || (model->Surfaces[hit.Node->Surf].PolyFlags & (PF_FakeBackdrop | PF_Invisible)) != 0)
		return nullptr;

	vec3 N = hit.Normal;
	vec3 pos = Location() + traceDirection * hit.Fraction;

	if (dot(decalDir, decalDir) < 0.01f) // decalDir specifies which direction the decal texture faces. If its zero use a random direction
	{
		vec3 randomDir;
		while (true)
		{
			randomDir = vec3((float)(std::rand() / (double)RAND_MAX), (float)(std::rand() / (double)RAND_MAX), (float)(std::rand() / (double)RAND_MAX)) * 2.0f - 1.0f;
			if (dot(randomDir, randomDir) >= 1.0f)
				break;
		}
		decalDir = randomDir;
	}

	vec3 ydir = -(decalDir - dot(decalDir, N) * N);
	if (dot(ydir, ydir) < 0.01f)
		ydir = normalize(cross(N, std::abs(N.x) > std::abs(N.y) ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f)));
	else
		ydir = normalize(ydir);
	vec3 xdir = cross(N, ydir);

	float usize = (float)Texture()->USize();
	float vsize = (float)Texture()->VSize();
	xdir *= DrawScale() * usize * 0.5f;
	ydir *= DrawScale() * vsize * 0.5f;

	static Array<vec3> positions;
	static Array<vec2> uvs;
	static Array<float> edgeDistances;

	// Walk all nodes in the same plane
	BspNode* polynode = hit.NodeHead;
	while (true)
	{
		positions.clear();
		uvs.clear();

		// Place decal on the surface plane
		positions.push_back(pos - xdir - ydir);
		positions.push_back(pos + xdir - ydir);
		positions.push_back(pos + xdir + ydir);
		positions.push_back(pos - xdir + ydir);
		uvs.push_back({ 0.0f, 0.0f });
		uvs.push_back({ usize, 0.0f });
		uvs.push_back({ usize, vsize });
		uvs.push_back({ 0.0f, vsize });

		// Clip to BSP node shape
		int vertCount = (int)positions.size();
		BspVert* v = &model->Vertices[polynode->VertPool];
		for (int j = 0; j < polynode->NumVertices; j++)
		{
			const vec3& edgeStart = model->Points[v[j > 0 ? j - 1 : polynode->NumVertices - 1].Vertex];
			const vec3& edgeEnd = model->Points[v[j].Vertex];
			vec3 planeN = cross(N, edgeEnd - edgeStart); // Note: not normalized as we don't need it
			vec4 plane(planeN, -dot(edgeEnd, planeN));

			// Find vertex distances to edge plane
			edgeDistances.clear();
			for (int i = 0; i < vertCount; i++)
				edgeDistances.push_back(dot(plane, vec4(positions[i], 1.0f)));

			// Insert points at the edge for any line crossing the plane
			for (int i = 0; i < vertCount; i++)
			{
				float dist = edgeDistances[i];
				float distNext = edgeDistances[(i + 1) % vertCount];
				if ((dist > 0.0f && distNext < 0.0f) || (distNext > 0.0f && dist < 0.0f))
				{
					vec3 p = positions[i];
					vec3 pNext = positions[(i + 1) % vertCount];
					vec2 uv = uvs[i];
					vec2 uvNext = uvs[(i + 1) % vertCount];

					// Ray/plane intersection
					float t = -dot(vec4(p, 1.0f), plane) / dot(plane.xyz(), pNext - p);
					vec3 pInsert = mix(p, pNext, t);
					vec2 uvInsert = mix(uv, uvNext, t);

					int insertAt = i + 1;
					positions.insert(positions.begin() + insertAt, pInsert);
					uvs.insert(uvs.begin() + insertAt, uvInsert);
					edgeDistances.insert(edgeDistances.begin() + insertAt, 0.0f);
					vertCount++;
				}
			}

			// Remove points outside
			int i = 0;
			while (i < vertCount)
			{
				if (edgeDistances[i] < 0.0f)
				{
					positions.erase(positions.begin() + i);
					uvs.erase(uvs.begin() + i);
					edgeDistances.erase(edgeDistances.begin() + i);
					vertCount--;
				}
				else
				{
					i++;
				}
			}
		}

		// Add to decals list if we still got anything left to render
		if (!positions.empty())
		{
			LevelDecal leveldecal;
			leveldecal.Decal = this;
			leveldecal.Positions = positions;
			leveldecal.UVs = uvs;
			polynode->Decals.push_back(leveldecal);
			Nodes.push_back(polynode);
		}

		if (polynode->Plane < 0) break;
		polynode = &model->Nodes[polynode->Plane];
	}

	return Level();
}

void UDecal::DetachDecal()
{
	for (BspNode* node : Nodes)
	{
		auto& decals = node->Decals;
		auto it = decals.begin();
		while (it != decals.end())
		{
			auto& leveldecal = *it;
			if (leveldecal.Decal == this)
				it = decals.erase(it);
			else
				++it;
		}
	}
	Nodes.clear();
}
