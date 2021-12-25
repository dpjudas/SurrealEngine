
#include "Precomp.h"
#include "ULevel.h"
#include "UActor.h"
#include "UTexture.h"
#include "UClass.h"
#include "VM/ScriptCall.h"

void ULevelBase::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	int32_t dbnum = stream->ReadInt32();
	int32_t dbmax = stream->ReadInt32();
	for (int32_t i = 0; i < dbnum; i++)
	{
		Actors.push_back(stream->ReadObject<UActor>());
	}

	Protocol = stream->ReadString();
	Host = stream->ReadString();
	if (!Host.empty())
		Port = stream->ReadInt32();
	Map = stream->ReadString();

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		Options.push_back(stream->ReadString());
	}

	Portal = stream->ReadString();

	stream->Skip(7);
}

/////////////////////////////////////////////////////////////////////////////

void ULevel::Load(ObjectStream* stream)
{
	ULevelBase::Load(stream);

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LevelReachSpec spec;
		spec.distance = stream->ReadInt32();
		spec.startActor = stream->ReadIndex();
		spec.endActor = stream->ReadIndex();
		spec.collisionRadius = stream->ReadInt32();
		spec.collisionHeight = stream->ReadInt32();
		spec.reachFlags = stream->ReadInt32();
		spec.bPruned = stream->ReadInt8();
		ReachSpecs.push_back(spec);
	}

	Model = stream->ReadObject<UModel>();

	if (Model)
	{
		Model->LoadNow();
	}
}

void ULevel::Tick(float elapsed)
{
	// To do: owned actors must tick before their children:
	for (size_t i = 0; i < Actors.size(); i++)
	{
		UActor* actor = Actors[i];
		if (actor)
		{
			actor->Tick(elapsed, ticked);

			if (actor->Role() >= ROLE_SimulatedProxy && actor->LifeSpan() != 0.0f)
			{
				actor->LifeSpan() = std::max(actor->LifeSpan() - elapsed, 0.0f);
				if (actor->LifeSpan() == 0.0f)
				{
					CallEvent(actor, "Expired");
					actor->Destroy();
				}
			}
		}
	}

	std::vector<UActor*> newActorList;
	newActorList.reserve(Actors.size());
	for (UActor* actor : Actors)
	{
		if (actor)
			newActorList.push_back(actor);
	}
	Actors.swap(newActorList);

	ticked = !ticked;
}

void ULevel::AddToCollision(UActor* actor)
{
	if (actor->bCollideActors())
	{
		vec3 location = actor->Location();
		float height = actor->CollisionHeight();
		float radius = actor->CollisionRadius();
		vec3 extents = { radius, radius, height };

		actor->CollisionHashInfo.Inserted = true;
		actor->CollisionHashInfo.Location = location;
		actor->CollisionHashInfo.Height = height;
		actor->CollisionHashInfo.Radius = radius;

		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					CollisionActors[GetBucketId(x, y, z)].push_back(actor);
				}
			}
		}
	}
}

void ULevel::RemoveFromCollision(UActor* actor)
{
	if (actor->CollisionHashInfo.Inserted)
	{
		vec3 location = actor->CollisionHashInfo.Location;
		float height = actor->CollisionHashInfo.Height;
		float radius = actor->CollisionHashInfo.Radius;
		vec3 extents = { radius, radius, height };

		ivec3 start = GetStartExtents(location, extents);
		ivec3 end = GetEndExtents(location, extents);
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					auto it = CollisionActors.find(GetBucketId(x, y, z));
					if (it != CollisionActors.end())
					{
						it->second.remove(actor);
						if (it->second.empty())
							CollisionActors.erase(it);
					}
				}
			}
		}

		actor->CollisionHashInfo.Inserted = false;
	}
}

double ULevel::RaySphereIntersect(const dvec3& rayOrigin, double tmin, const dvec3& rayDirNormalized, double tmax, const dvec3& sphereCenter, double sphereRadius)
{
	dvec3 l = sphereCenter - rayOrigin;
	double s = dot(l, rayDirNormalized);
	double l2 = dot(l, l);
	double r2 = sphereRadius * sphereRadius;
	if (s < 0 && l2 > r2)
		return tmax;
	double s2 = s * s;
	double m2 = l2 - s2;
	if (m2 > r2)
		return tmax;
	double q = std::sqrt(r2 - m2);
	double t = (l2 > r2) ? s - q : s + q;
	return (t >= tmin) ? t : tmax;
}

bool ULevel::ActorSphereCollision(const dvec3& origin, double sphereRadius, UActor* actor)
{
	double sphereRadius2 = sphereRadius * sphereRadius;

	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);

	dvec3 delta0 = sphere0 - origin;
	if (dot(delta0, delta0) < sphereRadius2)
		return true;
	dvec3 delta1 = sphere1 - origin;
	return dot(delta1, delta1) < sphereRadius2;
}

double ULevel::ActorRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, UActor* actor)
{
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	double t0 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere0, radius);
	double t1 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere1, radius);
	return std::min(t0, t1);
}

double ULevel::ActorSphereIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double sphereRadius, UActor* actor)
{
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	double t0 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere0, radius + sphereRadius);
	double t1 = RaySphereIntersect(origin, tmin, dirNormalized, tmax, sphere1, radius + sphereRadius);
	return std::min(t0, t1);
}

bool ULevel::TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	if (from == to || (!traceActors && !traceWorld))
		return false;

	dvec3 origin = to_dvec3(from);
	dvec3 direction = to_dvec3(to) - origin;
	double tmin = 0.01f;
	double tmax = length(direction);
	if (tmax < tmin)
		return false;
	direction *= 1.0f / tmax;

	float margin = 1.0f;
	tmax += margin;

	if (traceActors)
	{
		ivec3 start = GetRayStartExtents(from, to);
		ivec3 end = GetRayEndExtents(from, to);
		if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
		{
			for (int z = start.z; z < end.z; z++)
			{
				for (int y = start.y; y < end.y; y++)
				{
					for (int x = start.x; x < end.x; x++)
					{
						auto it = CollisionActors.find(GetBucketId(x, y, z));
						if (it != CollisionActors.end())
						{
							for (UActor* actor : it->second)
							{
								if (actor != tracingActor && actor->bBlockActors() && ActorRayIntersect(origin, tmin, direction, tmax, actor) < tmax)
									return true;
							}
						}
					}
				}
			}
		}
	}

	if (traceWorld)
	{
		return Model->TraceAnyHit(origin, tmin, direction, tmax, visibilityOnly);
	}

	return false;
}

SweepHit ULevel::TraceFirstHit(const vec3& from, const vec3& to, UActor* tracingActor, const vec3& extents, const TraceFlags& flags)
{
	for (const SweepHit& hit : Sweep(from, to, extents.z, extents.x, flags.traceActors(), flags.traceWorld(), false))
	{
		if (hit.Actor && (!tracingActor || !tracingActor->IsOwnedBy(hit.Actor)))
		{
			if (hit.Actor->IsA("Pawn"))
			{
				if (flags.pawns)
					return hit;
			}
			else if (hit.Actor->IsA("Mover"))
			{
				if (flags.movers)
					return hit;
			}
			else if (hit.Actor->IsA("ZoneInfo"))
			{
				if (flags.zoneChanges)
					return hit;
			}
			else if (flags.others)
			{
				if (!flags.onlyProjectiles || hit.Actor->bProjTarget() || (hit.Actor->bBlockActors() && hit.Actor->bBlockPlayers()))
					return hit;
			}
		}
		else if (flags.world && !hit.Actor)
		{
			SweepHit worldHit = hit;
			if (tracingActor)
				worldHit.Actor = tracingActor->Level();
			return worldHit;
		}
	}
	return {};
}

std::vector<SweepHit> ULevel::Sweep(const vec3& from, const vec3& to, float height, float radius, bool traceActors, bool traceWorld, bool visibilityOnly)
{
	if (from == to || (!traceActors && !traceWorld))
		return {};

	dvec3 origin = to_dvec3(from);
	dvec3 direction = to_dvec3(to) - origin;
	double tmin = 0.01f;
	double tmax = length(direction);
	if (tmax < tmin)
		return {};
	direction *= 1.0f / tmax;

	float margin = 1.0f;
	tmax += margin;

	std::vector<SweepHit> hits;

	if (traceActors)
	{
		double dradius = radius;
		vec3 extents = { radius, radius, height };

		ivec3 start = GetSweepStartExtents(from, to, extents);
		ivec3 end = GetSweepEndExtents(from, to, extents);
		if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
		{
			for (int z = start.z; z < end.z; z++)
			{
				for (int y = start.y; y < end.y; y++)
				{
					for (int x = start.x; x < end.x; x++)
					{
						auto it = CollisionActors.find(GetBucketId(x, y, z));
						if (it != CollisionActors.end())
						{
							for (UActor* actor : it->second)
							{
								double t = ActorSphereIntersect(origin, tmin, direction, tmax, dradius, actor);
								if (t < tmax)
								{
									dvec3 hitpos = origin + direction * t;
									hits.push_back({ (float)t, normalize(to_vec3(hitpos) - actor->Location()), actor });
								}
							}
						}
					}
				}
			}
		}
	}

	if (traceWorld)
	{
		vec3 offset = vec3(0.0, 0.0, height - radius);
		dvec3 origin0 = to_dvec3(from - offset);
		dvec3 origin1 = to_dvec3(from + offset);
		std::vector<SweepHit> worldHits0 = Model->Sweep(origin0, tmin, direction, tmax, radius, visibilityOnly);
		std::vector<SweepHit> worldHits1 = Model->Sweep(origin1, tmin, direction, tmax, radius, visibilityOnly);
		hits.insert(hits.end(), worldHits0.begin(), worldHits0.end());
		hits.insert(hits.end(), worldHits1.begin(), worldHits1.end());
	}

	// Sort by closest hit and only include the first hit for each actor

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	std::set<UActor*> seenActors;
	std::vector<SweepHit> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit.Actor)
		{
			if (seenActors.find(hit.Actor) == seenActors.end())
			{
				seenActors.insert(hit.Actor);
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	tmax -= margin;
	for (auto& hit : uniqueHits)
	{
		hit.Fraction = (float)(std::max(hit.Fraction - margin, 0.0f) / tmax);
	}

	return uniqueHits;
}

std::vector<UActor*> ULevel::CollidingActors(const vec3& origin, float radius)
{
	dvec3 dorigin = to_dvec3(origin);
	double dradius = radius;
	vec3 extents = { radius, radius, radius };

	std::vector<UActor*> hits;

	ivec3 start = GetStartExtents(origin, extents);
	ivec3 end = GetEndExtents(origin, extents);
	if (end.x - start.x < 100 && end.y - start.y < 100 && end.z - start.z < 100)
	{
		for (int z = start.z; z < end.z; z++)
		{
			for (int y = start.y; y < end.y; y++)
			{
				for (int x = start.x; x < end.x; x++)
				{
					auto it = CollisionActors.find(GetBucketId(x, y, z));
					if (it != CollisionActors.end())
					{
						for (UActor* actor : it->second)
						{
							if (ActorSphereCollision(dorigin, dradius, actor))
								hits.push_back(actor);
						}
					}
				}
			}
		}
	}

	std::set<UActor*> seenActors;
	std::vector<UActor*> uniqueHits;
	uniqueHits.reserve(hits.size());
	for (auto& hit : hits)
	{
		if (hit)
		{
			if (seenActors.find(hit) == seenActors.end())
			{
				seenActors.insert(hit);
				uniqueHits.push_back(hit);
			}
		}
		else
		{
			uniqueHits.push_back(hit);
		}
	}

	return uniqueHits;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<TraceHit> UModel::Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly)
{
	std::vector<TraceHit> hits;
	Trace(origin, tmin, dirNormalized, tmax, visibilityOnly, &Nodes.front(), hits);
	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	return hits;
}

void UModel::Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node, std::vector<TraceHit>& hits)
{
	BspNode* polynode = node;
	while (true)
	{
		if (!visibilityOnly || (polynode->NodeFlags & NF_NotVisBlocking) == 0)
		{
			double t = NodeRayIntersect(origin, tmin, dirNormalized, tmax, polynode);
			if (t >= tmin && t < tmax)
			{
				TraceHit hit = { (float)t, vec3(node->PlaneX, node->PlaneY, node->PlaneZ) };
				if (dot(to_dvec3(hit.Normal), dirNormalized) > 0.0)
					hit.Normal = -hit.Normal;
				hits.push_back(hit);
			}
		}

		if (polynode->Plane < 0) break;
		polynode = &Nodes[polynode->Plane];
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);

	if (node->Front >= 0 && (fromSide >= 0.0 || toSide >= 0.0))
		Trace(origin, tmin, dirNormalized, tmax, visibilityOnly, &Nodes[node->Front], hits);
	if (node->Back >= 0 && (fromSide <= 0.0 || toSide <= 0.0))
		Trace(origin, tmin, dirNormalized, tmax, visibilityOnly, &Nodes[node->Back], hits);
}

bool UModel::TraceAnyHit(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly)
{
	return TraceAnyHit(origin, tmin, dirNormalized, tmax, visibilityOnly, &Nodes.front());
}

bool UModel::TraceAnyHit(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node)
{
	BspNode* polynode = node;
	while (true)
	{
		if (!visibilityOnly || (polynode->NodeFlags & NF_NotVisBlocking) == 0)
		{
			double t = NodeRayIntersect(origin, tmin, dirNormalized, tmax, polynode);
			if (t >= tmin && t < tmax)
				return true;
		}

		if (polynode->Plane < 0) break;
		polynode = &Nodes[polynode->Plane];
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);

	if (node->Front >= 0 && (fromSide >= 0.0 || toSide >= 0.0) && TraceAnyHit(origin, tmin, dirNormalized, tmax, visibilityOnly, &Nodes[node->Front]))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0 || toSide <= 0.0) && TraceAnyHit(origin, tmin, dirNormalized, tmax, visibilityOnly, &Nodes[node->Back]))
		return true;
	else
		return false;
}

std::vector<SweepHit> UModel::Sweep(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, bool visibilityOnly)
{
	// Sort by closest hit
	std::vector<SweepHit> hits;
	Sweep(origin, tmin, dirNormalized, tmax, radius, visibilityOnly, &Nodes.front(), hits);
	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	return hits;
}

void UModel::Sweep(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, bool visibilityOnly, BspNode* node, std::vector<SweepHit>& hits)
{
	BspNode* polynode = node;
	while (true)
	{
		bool blocking = !visibilityOnly || (polynode->NodeFlags & NF_NotVisBlocking) == 0;
		if (blocking)
		{
			double t = NodeSphereIntersect(origin, tmin, dirNormalized, tmax, radius, polynode);
			if (t < tmax && t >= tmin)
			{
				SweepHit hit = { (float)t, vec3(node->PlaneX, node->PlaneY, node->PlaneZ), nullptr };
				if (dot(to_dvec3(hit.Normal), dirNormalized) > 0.0)
					hit.Normal = -hit.Normal;
				hits.push_back(hit);
			}
		}

		if (polynode->Plane < 0) break;
		polynode = &Nodes[polynode->Plane];
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);

	if (node->Front >= 0 && (fromSide >= -radius || toSide >= -radius))
	{
		Sweep(origin, tmin, dirNormalized, tmax, radius, visibilityOnly, &Nodes[node->Front], hits);
	}

	if (node->Back >= 0 && (fromSide <= radius || toSide <= radius))
	{
		Sweep(origin, tmin, dirNormalized, tmax, radius, visibilityOnly, &Nodes[node->Back], hits);
	}
}

double UModel::NodeRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, BspNode* node)
{
	if (node->NumVertices < 3 || (node->Surf >= 0 && Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return tmax;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);
	if ((fromSide > 0.0 && toSide > 0.0) || (fromSide < 0.0 && toSide < 0.0))
		return tmax;

	BspVert* v = &Vertices[node->VertPool];
	vec3* points = Points.data();

	dvec3 p[3];
	p[0] = to_dvec3(points[v[0].Vertex]);
	p[1] = to_dvec3(points[v[1].Vertex]);

	double t = tmax;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = to_dvec3(points[v[i].Vertex]);
		double tval = TriangleRayIntersect(origin, dirNormalized, tmax, p);
		if (tval >= tmin)
			t = std::min(tval, t);
		p[1] = p[2];
	}
	return t;
}

double UModel::NodeSphereIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, BspNode* node)
{
	if (node->NumVertices < 3 || (node->Surf >= 0 && Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return tmax;

	dvec3 target = origin + dirNormalized * tmax;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(target, 1.0), plane);
	if ((fromSide > radius && toSide > radius) || (fromSide < -radius && toSide < -radius))
		return tmax;

	BspVert* v = &Vertices[node->VertPool];
	vec3* points = Points.data();

	dvec3 p[3];
	p[0] = to_dvec3(points[v[0].Vertex]);
	p[1] = to_dvec3(points[v[1].Vertex]);

	double t = tmax;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = to_dvec3(points[v[i].Vertex]);
		double tval = TriangleSphereIntersect(origin, target, radius, p) * tmax;
		if (tval >= tmin)
			t = std::min(tval, t);
		p[1] = p[2];
	}
	return t;
}

double UModel::TriangleRayIntersect(const dvec3& origin, const dvec3& dirNormalized, double tmax, const dvec3* p)
{
	// Moeller-Trumbore ray-triangle intersection algorithm:

	// Find vectors for two edges sharing p[0]
	dvec3 e1 = p[1] - p[0];
	dvec3 e2 = p[2] - p[0];

	// Begin calculating determinant - also used to calculate u parameter
	dvec3 P = cross(dirNormalized, e2);
	double det = dot(e1, P);

	// Backface check
	//if (det < 0.0)
	//	return tmax;

	// If determinant is near zero, ray lies in plane of triangle
	if (det > -FLT_EPSILON && det < FLT_EPSILON)
		return tmax;

	double inv_det = 1.0 / det;

	// Calculate distance from p[0] to ray origin
	dvec3 T = origin - p[0];

	// Calculate u parameter and test bound
	double u = dot(T, P) * inv_det;

	// Check if the intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
		return tmax;

	// Prepare to test v parameter
	dvec3 Q = cross(T, e1);

	// Calculate V parameter and test bound
	double v = dot(dirNormalized, Q) * inv_det;

	// The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f)
		return tmax;

	double t = dot(e2, Q) * inv_det;
	if (t <= FLT_EPSILON)
		return tmax;

	// Return hit location on triangle in barycentric coordinates
	// barycentricB = u;
	// barycentricC = v;

	return std::min(t, tmax);
}

double UModel::TriangleSphereIntersect(const dvec3& from, const dvec3& to, double radius, const dvec3* p)
{
	dvec3 c = from;
	dvec3 e = to;
	double r = radius;

	// Dynamic intersection test between a ray and the minkowski sum of the sphere and polygon:

	dvec3 n = normalize(cross(p[1] - p[0], p[2] - p[0]));
	dvec4 plane(n, -dot(n, p[0]));

	// Step 1: Plane intersect test

	double sc = dot(plane, dvec4(from, 1.0));
	double se = dot(plane, dvec4(to, 1.0));
	bool same_side = sc * se > 0.0;

	if (same_side && std::abs(sc) > r && std::abs(se) > r)
		return 1.0;

	// Step 1a: Check if point is in polygon (using crossing ray test in 2d)
	double t = (sc - r) / (sc - se);
	if (t >= 0.0 && t <= 1.0)
	{
		// To do: this can be precalculated for the mesh
		dvec3 u0, u1;
		if (std::abs(n.x) > std::abs(n.y))
		{
			u0 = { 0.0, 1.0, 0.0 };
			if (std::abs(n.z) > std::abs(n.x))
				u1 = { 1.0, 0.0, 0.0 };
			else
				u1 = { 0.0, 0.0, 1.0 };
		}
		else
		{
			u0 = { 1.0, 0.0, 0.0 };
			if (std::abs(n.z) > std::abs(n.y))
				u1 = { 0.0, 1.0, 0.0 };
			else
				u1 = { 0.0, 0.0, 1.0 };
		}
		dvec2 v_2d[3] =
		{
			dvec2(dot(u0, p[0]), dot(u1, p[0])),
			dvec2(dot(u0, p[1]), dot(u1, p[1])),
			dvec2(dot(u0, p[2]), dot(u1, p[2])),
		};

		dvec3 vt = c + (e - c) * t;
		dvec2 point(dot(u0, vt), dot(u1, vt));

		bool inside = false;
		dvec2 e0 = v_2d[2];
		bool y0 = e0.y >= point.y;
		for (int i = 0; i < 3; i++)
		{
			dvec2 e1 = v_2d[i];
			bool y1 = e1.y >= point.y;

			if (y0 != y1 && ((e1.y - point.y) * (e0.x - e1.x) >= (e1.x - point.x) * (e0.y - e1.y)) == y1)
				inside = !inside;

			y0 = y1;
			e0 = e1;
		}

		if (inside)
			return t;
	}

	// Step 2: Edge intersect test

	dvec3 ke[3] =
	{
		p[1] - p[0],
		p[2] - p[1],
		p[0] - p[2],
	};

	dvec3 kg[3] =
	{
		p[0] - c,
		p[1] - c,
		p[2] - c,
	};

	dvec3 ks = e - c;

	double kgg[3];
	double kgs[3];

	double kss = dot(ks, ks);
	double rr = r * r;

	double hitFraction = 1.0;

	for (int i = 0; i < 3; i++)
	{
		double kee = dot(ke[i], ke[i]);
		double keg = dot(ke[i], kg[i]);
		double kes = dot(ke[i], ks);
		kgg[i] = dot(kg[i], kg[i]);
		kgs[i] = dot(kg[i], ks);

		double aa = kee * kss - kes * kes;
		double bb = 2 * (keg * kes - kee * kgs[i]);
		double cc = kee * (kgg[i] - rr) - keg * keg;

		double dsqr = bb * bb - 4 * aa * cc;
		if (dsqr >= 0.0)
		{
			double sign = (bb >= 0.0) ? 1.0 : -1.0;
			double q = -0.5f * (bb + sign * std::sqrt(dsqr));
			double t0 = q / aa;
			double t1 = cc / q;

			double t;
			if (t0 < 0.0 || t0 > 1.0)
				t = t1;
			else if (t1 < 0.0 || t1 > 1.0)
				t = t0;
			else
				t = std::min(t0, t1);

			if (t >= 0.0 && t <= 1.0)
			{
				dvec3 ct = c + ks * t;
				double d = dot(ct - p[i], ke[i]);
				if (d >= 0.0 && d <= kee)
					hitFraction = std::min(t, hitFraction);
			}
		}
	}

	if (hitFraction < 1.0)
		return hitFraction;

	// Step 3: Point intersect test

	for (int i = 0; i < 3; i++)
	{
		double aa = kss;
		double bb = -2.0 * kgs[i];
		double cc = kgg[i] - rr;

		double dsqr = bb * bb - 4 * aa * cc;
		if (dsqr >= 0.0)
		{
			double sign = (bb >= 0.0) ? 1.0 : -1.0;
			double q = -0.5f * (bb + sign * std::sqrt(dsqr));
			double t0 = q / aa;
			double t1 = cc / q;

			double t;
			if (t0 < 0.0 || t0 > 1.0)
				t = t1;
			else if (t1 < 0.0 || t1 > 1.0)
				t = t0;
			else
				t = std::min(t0, t1);

			if (t >= 0.0 && t <= 1.0)
				hitFraction = std::min(t, hitFraction);
		}
	}

	return hitFraction;
}

// -1 = inside, 0 = intersects, 1 = outside
int UModel::NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node)
{
	float e = extents.x * std::abs(node->PlaneX) + extents.y * std::abs(node->PlaneY) + extents.z * std::abs(node->PlaneZ);
	float s = center.x * node->PlaneX + center.y * node->PlaneY + center.z * node->PlaneZ - node->PlaneW;
	if (s - e > 0)
		return -1;
	else if (s + e < 0)
		return 1;
	else
		return 0;
}

void UModel::Load(ObjectStream* stream)
{
	UPrimitive::Load(stream);

	if (stream->GetVersion() <= 61)
	{
		UVectors* vectors = stream->ReadObject<UVectors>();
		UVectors* points = stream->ReadObject<UVectors>();
		UBspNodes* nodes = stream->ReadObject<UBspNodes>();
		UBspSurfs* surfaces = stream->ReadObject<UBspSurfs>();
		UVerts* verts = stream->ReadObject<UVerts>();

		vectors->LoadNow();
		points->LoadNow();
		nodes->LoadNow();
		surfaces->LoadNow();
		verts->LoadNow();

		Vectors = vectors->Vectors;
		Points = points->Vectors;
		Nodes = nodes->Nodes;
		Zones = nodes->Zones;
		Surfaces = surfaces->Surfaces;
		Vertices = verts->Vertices;
		NumSharedSides = verts->NumSharedSides;
	}
	else
	{
		int count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			vec3 v;
			v.x = stream->ReadFloat();
			v.y = stream->ReadFloat();
			v.z = stream->ReadFloat();
			Vectors.push_back(v);
		}

		count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			vec3 v;
			v.x = stream->ReadFloat();
			v.y = stream->ReadFloat();
			v.z = stream->ReadFloat();
			Points.push_back(v);
		}

		count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			BspNode node;
			node.PlaneX = stream->ReadFloat();
			node.PlaneY = stream->ReadFloat();
			node.PlaneZ = stream->ReadFloat();
			node.PlaneW = stream->ReadFloat();
			node.ZoneMask = stream->ReadUInt64();
			node.NodeFlags = stream->ReadUInt8();
			node.VertPool = stream->ReadIndex();
			node.Surf = stream->ReadIndex();
			node.Back = stream->ReadIndex();
			node.Front = stream->ReadIndex();
			node.Plane = stream->ReadIndex();
			node.CollisionBound = stream->ReadIndex();
			node.RenderBound = stream->ReadIndex();
			node.Zone0 = stream->ReadIndex();
			node.Zone1 = stream->ReadIndex();
			node.NumVertices = stream->ReadUInt8();
			node.Leaf0 = stream->ReadInt32();
			node.Leaf1 = stream->ReadInt32();
			Nodes.push_back(node);
		}

		count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			BspSurface surface;
			surface.Material = stream->ReadObject<UTexture>();
			surface.PolyFlags = stream->ReadUInt32();
			surface.pBase = stream->ReadIndex();
			surface.vNormal = stream->ReadIndex();
			surface.vTextureU = stream->ReadIndex();
			surface.vTextureV = stream->ReadIndex();
			surface.LightMap = stream->ReadIndex();
			surface.BrushPoly = stream->ReadIndex();
			surface.PanU = stream->ReadInt16();
			surface.PanV = stream->ReadInt16();
			surface.BrushActor = stream->ReadIndex();
			Surfaces.push_back(surface);
		}

		count = stream->ReadIndex();
		for (int i = 0; i < count; i++)
		{
			BspVert vert;
			vert.Vertex = stream->ReadIndex();
			vert.Side = stream->ReadIndex();
			Vertices.push_back(vert);
		}

		NumSharedSides = stream->ReadInt32();

		int32_t NumZones = stream->ReadInt32();
		for (int i = 0; i < NumZones; i++)
		{
			ZoneProperties zone;
			zone.ZoneActor = stream->ReadObject<UActor>();
			zone.Connectivity = stream->ReadUInt64();
			zone.Visibility = stream->ReadUInt64();
			Zones.push_back(zone);
		}
	}

	Polys = stream->ReadIndex();

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LightMapIndex entry;
		entry.DataOffset = stream->ReadInt32();
		entry.PanX = stream->ReadFloat();
		entry.PanY = stream->ReadFloat();
		entry.PanZ = stream->ReadFloat();
		entry.UClamp = stream->ReadIndex();
		entry.VClamp = stream->ReadIndex();
		entry.UScale = stream->ReadFloat();
		entry.VScale = stream->ReadFloat();
		entry.LightActors = stream->ReadInt32();
		LightMap.push_back(entry);
	}

	LightBits.resize(stream->ReadIndex());
	stream->ReadBytes(LightBits.data(), (uint32_t)LightBits.size());

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		BBox boundingBox;
		boundingBox.min.x = stream->ReadFloat();
		boundingBox.min.y = stream->ReadFloat();
		boundingBox.min.z = stream->ReadFloat();
		boundingBox.max.x = stream->ReadFloat();
		boundingBox.max.y = stream->ReadFloat();
		boundingBox.max.z = stream->ReadFloat();
		boundingBox.IsValid = stream->ReadInt8() != 0;
		Bounds.push_back(boundingBox);
	}

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		LeafHulls.push_back(stream->ReadInt32());
	}

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		ConvexVolumeLeaf leaf;
		leaf.Zone = stream->ReadIndex();
		leaf.Permeating = stream->ReadIndex();
		leaf.Volumetric = stream->ReadIndex();
		leaf.VisibleZones = stream->ReadUInt64();
		Leaves.push_back(leaf);
	}

	count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		Lights.push_back(stream->ReadObject<UActor>());
	}

	if (stream->GetVersion() <= 61)
	{
		UObject* unknown1 = stream->ReadObject<UObject>();
		UObject* unknown2 = stream->ReadObject<UObject>();
	}

	RootOutside = stream->ReadInt32();
	Linked = stream->ReadInt32();
}

/////////////////////////////////////////////////////////////////////////////

void UPolys::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		int numVertices = stream->ReadIndex();
		Poly poly;
		poly.Base.x = stream->ReadFloat();
		poly.Base.y = stream->ReadFloat();
		poly.Base.z = stream->ReadFloat();
		poly.Normal.x = stream->ReadFloat();
		poly.Normal.y = stream->ReadFloat();
		poly.Normal.z = stream->ReadFloat();
		poly.TextureU.x = stream->ReadFloat();
		poly.TextureU.y = stream->ReadFloat();
		poly.TextureU.z = stream->ReadFloat();
		poly.TextureV.x = stream->ReadFloat();
		poly.TextureV.y = stream->ReadFloat();
		poly.TextureV.z = stream->ReadFloat();
		for (int i = 0; i < numVertices; i++)
		{
			vec3 v;
			v.x = stream->ReadFloat();
			v.y = stream->ReadFloat();
			v.z = stream->ReadFloat();
			poly.Vertices.push_back(v);
		}
		poly.PolyFlags = stream->ReadUInt32();
		poly.Actor = stream->ReadObject<UBrush>();
		poly.Texture = stream->ReadObject<UTexture>();
		poly.ItemName = stream->ReadName();
		poly.LinkIndex = stream->ReadIndex();
		poly.BrushPolyIndex = stream->ReadIndex();
		poly.PanU = stream->ReadInt16();
		poly.PanV = stream->ReadInt16();
		Polys.push_back(poly);
	}
}

/////////////////////////////////////////////////////////////////////////////

void UBspNodes::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		BspNode node;
		node.PlaneX = stream->ReadFloat();
		node.PlaneY = stream->ReadFloat();
		node.PlaneZ = stream->ReadFloat();
		node.PlaneW = stream->ReadFloat();
		node.ZoneMask = stream->ReadUInt64();
		node.NodeFlags = stream->ReadUInt8();
		node.VertPool = stream->ReadIndex();
		node.Surf = stream->ReadIndex();
		node.Back = stream->ReadIndex();
		node.Front = stream->ReadIndex();
		node.Plane = stream->ReadIndex();
		node.CollisionBound = stream->ReadIndex();
		node.RenderBound = stream->ReadIndex();
		node.Zone0 = stream->ReadIndex();
		node.Zone1 = stream->ReadIndex();
		node.NumVertices = stream->ReadUInt8();
		node.Leaf0 = stream->ReadInt32();
		node.Leaf1 = stream->ReadInt32();
		Nodes.push_back(node);
	}

	int32_t NumZones = stream->ReadIndex();
	for (int i = 0; i < NumZones; i++)
	{
		ZoneProperties zone;
		zone.ZoneActor = stream->ReadObject<UActor>();
		zone.Connectivity = stream->ReadUInt64();
		zone.Visibility = stream->ReadUInt64();
		Zones.push_back(zone);
	}
}

/////////////////////////////////////////////////////////////////////////////

void UBspSurfs::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		BspSurface surface;
		surface.Material = stream->ReadObject<UTexture>();
		surface.PolyFlags = stream->ReadUInt32();
		surface.pBase = stream->ReadIndex();
		surface.vNormal = stream->ReadIndex();
		surface.vTextureU = stream->ReadIndex();
		surface.vTextureV = stream->ReadIndex();
		surface.LightMap = stream->ReadIndex();
		surface.BrushPoly = stream->ReadIndex();
		surface.PanU = stream->ReadInt16();
		surface.PanV = stream->ReadInt16();
		surface.BrushActor = stream->ReadIndex();
		Surfaces.push_back(surface);
	}
}

/////////////////////////////////////////////////////////////////////////////

void UVectors::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		vec3 v;
		v.x = stream->ReadFloat();
		v.y = stream->ReadFloat();
		v.z = stream->ReadFloat();
		Vectors.push_back(v);
	}
}

/////////////////////////////////////////////////////////////////////////////

void UVerts::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		BspVert vert;
		vert.Vertex = stream->ReadIndex();
		vert.Side = stream->ReadIndex();
		Vertices.push_back(vert);
	}

	NumSharedSides = stream->ReadIndex();
}
