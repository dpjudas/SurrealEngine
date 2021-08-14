
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
		CollisionActors.resize(Model->Nodes.size());
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
	ticked = !ticked;
}

void ULevel::AddToCollision(UActor* actor)
{
	if (actor->bBlockActors() || actor->bBlockPlayers())
	{
		vec3 location = actor->Location();
		float height = actor->CollisionHeight();
		float radius = actor->CollisionRadius();
		AddToCollision(actor, location, vec3(radius, radius, height), &Model->Nodes.front());
	}
}

void ULevel::RemoveFromCollision(UActor* actor)
{
	if (actor->bBlockActors() || actor->bBlockPlayers())
	{
		vec3 location = actor->Location();
		float height = actor->CollisionHeight();
		float radius = actor->CollisionRadius();
		RemoveFromCollision(actor, location, vec3(radius, radius, height), &Model->Nodes.front());
	}
}

void ULevel::AddToCollision(UActor* actor, const vec3& location, const vec3& extents, BspNode* node)
{
	ptrdiff_t index = (ptrdiff_t)(node - Model->Nodes.data());
	int side = NodeAABBOverlap(location, extents, node);
	if ((side <= 0 && node->Front < 0) || (side >= 0 && node->Back < 0))
	{
		CollisionActors[index].push_back(actor);
	}
	else
	{
		if (side <= 0)
		{
			AddToCollision(actor, location, extents, &Model->Nodes[node->Front]);
		}
		if (side >= 0)
		{
			AddToCollision(actor, location, extents, &Model->Nodes[node->Back]);
		}
	}
}

void ULevel::RemoveFromCollision(UActor* actor, const vec3& location, const vec3& extents, BspNode* node)
{
	ptrdiff_t index = (ptrdiff_t)(node - Model->Nodes.data());
	int side = NodeAABBOverlap(location, extents, node);
	if ((side <= 0 && node->Front < 0) || (side >= 0 && node->Back < 0))
	{
		CollisionActors[index].remove(actor);
	}
	else
	{
		if (side <= 0)
		{
			AddToCollision(actor, location, extents, &Model->Nodes[node->Front]);
		}
		if (side >= 0)
		{
			AddToCollision(actor, location, extents, &Model->Nodes[node->Back]);
		}
	}
}

// -1 = inside, 0 = intersects, 1 = outside
int ULevel::NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node)
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

double ULevel::RaySphereIntersect(const dvec3& rayOrigin, const dvec3& rayDir, const dvec3& sphereCenter, double sphereRadius)
{
	dvec3 l = sphereCenter - rayOrigin;
	double s = dot(l, rayDir);
	double l2 = dot(l, l);
	double r2 = sphereRadius * sphereRadius;
	if (s < 0 && l2 > r2)
		return 1.0;
	double s2 = s * s;
	double m2 = l2 - s2;
	if (m2 > r2)
		return 1.0;
	double q = std::sqrt(r2 - m2);
	double t = (l2 > r2) ? s - q : s + q;
	return t;
}

double ULevel::ActorRayIntersect(const dvec3& from, const dvec3& to, UActor* actor)
{
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius();
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	dvec3 origin = from;
	dvec3 dir = to - origin;
	return std::min(RaySphereIntersect(origin, dir, sphere0, radius), RaySphereIntersect(origin, dir, sphere1, radius));
}

double ULevel::ActorSphereIntersect(const dvec3& from, const dvec3& to, double sphereRadius, UActor* actor)
{
	float height = actor->CollisionHeight();
	float radius = actor->CollisionRadius() + (float)sphereRadius;
	vec3 offset = vec3(0.0, 0.0, height - radius);
	dvec3 sphere0 = to_dvec3(actor->Location() - offset);
	dvec3 sphere1 = to_dvec3(actor->Location() + offset);
	dvec3 origin = from;
	dvec3 dir = to - origin;
	return std::min(RaySphereIntersect(origin, dir, sphere0, radius), RaySphereIntersect(origin, dir, sphere1, radius));
}

bool ULevel::TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld)
{
	if (from == to)
		return false;

	return TraceAnyHit(dvec4(from.x, from.y, from.z, 1.0), dvec4(to.x, to.y, to.z, 1.0), tracingActor, traceActors, traceWorld, &Model->Nodes.front());
}

bool ULevel::TraceAnyHit(const dvec4& from, const dvec4& to, UActor* tracingActor, bool traceActors, bool traceWorld, BspNode* node)
{
	if (traceActors)
	{
		ptrdiff_t index = (ptrdiff_t)(node - Model->Nodes.data());
		for (UActor* actor : CollisionActors[index])
		{
			if (actor != tracingActor && ActorRayIntersect(from.xyz(), to.xyz(), actor) < 1.0)
				return true;
		}
	}

	if (traceWorld)
	{
		BspNode* polynode = node;
		while (true)
		{
			if (NodeRayIntersect(from, to, polynode) < 1.0)
				return true;

			if (polynode->Plane < 0) break;
			polynode = &Model->Nodes[polynode->Plane];
		}
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);

	if (node->Front >= 0 && (fromSide >= 0.0 || toSide >= 0.0) && TraceAnyHit(from, to, tracingActor, traceActors, traceWorld, &Model->Nodes[node->Front]))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0 || toSide <= 0.0) && TraceAnyHit(from, to, tracingActor, traceActors, traceWorld, &Model->Nodes[node->Back]))
		return true;
	else
		return false;
}

SweepHit ULevel::Sweep(const vec3& from, const vec3& to, float height, float radius, UActor* tracingActor, bool traceActors, bool traceWorld)
{
	if (from == to || (!traceActors && !traceWorld))
		return {};

	CylinderSweep cylinder(from, to, height, radius, tracingActor, traceActors, traceWorld);

	// Add margin:
	double margin = 3.0;
	dvec3 direction = cylinder.To - cylinder.From;
	double moveDistance = length(direction);
	direction /= moveDistance;
	dvec3 finalTo = cylinder.To + direction * margin;

	// Do the collision sweep:
	dvec3 offset = dvec3(0.0, 0.0, cylinder.Height - cylinder.Radius);
	SphereSweep top(cylinder.From + offset, finalTo + offset, cylinder.Radius, cylinder.Filter);
	SphereSweep bottom(cylinder.From - offset, finalTo - offset, cylinder.Radius, cylinder.Filter);
	Sweep(&top, &Model->Nodes.front());
	Sweep(&bottom, &Model->Nodes.front());
	cylinder.Hit = top.Hit.Fraction < bottom.Hit.Fraction ? top.Hit : bottom.Hit;

	// Apply margin to result:
	if (cylinder.Hit.Fraction < 1.0)
	{
		cylinder.Hit.Fraction = (float)clamp(((moveDistance + margin) * cylinder.Hit.Fraction - margin) / moveDistance, 0.0, 1.0);
		if (dot(to_dvec3(cylinder.Hit.Normal), direction) < 0.0)
			cylinder.Hit.Normal = -cylinder.Hit.Normal;
	}
	return cylinder.Hit;
}

void ULevel::Sweep(SphereSweep* sphere, BspNode* node)
{
	if (sphere->Filter.TraceActors)
	{
		ptrdiff_t index = (ptrdiff_t)(node - Model->Nodes.data());
		for (UActor* actor : CollisionActors[index])
		{
			if (actor != sphere->Filter.TracingActor)
			{
				double t = ActorSphereIntersect(sphere->From, sphere->To, sphere->Radius, actor);
				if (t < sphere->Hit.Fraction)
				{
					dvec3 hitpos = mix(sphere->From, sphere->To, t);
					sphere->Hit.Fraction = (float)t;
					sphere->Hit.Normal = normalize(to_vec3(hitpos) - actor->Location());
					sphere->Hit.Actor = actor;
				}
			}
		}
	}

	if (sphere->Filter.TraceWorld)
	{
		BspNode* polynode = node;
		while (true)
		{
			double t = NodeSphereIntersect(sphere->From4, sphere->To4, sphere->Radius, polynode);
			if (t < sphere->Hit.Fraction)
			{
				sphere->Hit.Fraction = (float)t;
				sphere->Hit.Normal = { node->PlaneX, node->PlaneY, node->PlaneZ };
				sphere->Hit.Actor = nullptr;
			}

			if (polynode->Plane < 0) break;
			polynode = &Model->Nodes[polynode->Plane];
		}
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(sphere->From4, plane);
	double toSide = dot(sphere->To4, plane);

	if (node->Front >= 0 && (fromSide >= -sphere->Radius || toSide >= -sphere->Radius))
	{
		Sweep(sphere, &Model->Nodes[node->Front]);
	}

	if (node->Back >= 0 && (fromSide <= sphere->Radius || toSide <= sphere->Radius))
	{
		Sweep(sphere, &Model->Nodes[node->Back]);
	}
}

double ULevel::NodeRayIntersect(const dvec4& from, const dvec4& to, BspNode* node)
{
	if (node->NumVertices < 3 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return 1.0;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);
	if ((fromSide > 0.0 && toSide > 0.0) || (fromSide < 0.0 && toSide < 0.0))
		return 1.0;

	BspVert* v = &Model->Vertices[node->VertPool];
	vec3* points = Model->Points.data();

	dvec3 p[3];
	p[0] = to_dvec3(points[v[0].Vertex]);
	p[1] = to_dvec3(points[v[1].Vertex]);

	double t = 1.0;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = to_dvec3(points[v[i].Vertex]);
		t = std::min(TriangleRayIntersect(from, to, p), t);
		p[1] = p[2];
	}
	return t;
}

double ULevel::NodeSphereIntersect(const dvec4& from, const dvec4& to, double radius, BspNode* node)
{
	if (node->NumVertices < 3 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return 1.0;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);
	if ((fromSide > radius && toSide > radius) || (fromSide < -radius && toSide < -radius))
		return 1.0;

	BspVert* v = &Model->Vertices[node->VertPool];
	vec3* points = Model->Points.data();

	dvec3 p[3];
	p[0] = to_dvec3(points[v[0].Vertex]);
	p[1] = to_dvec3(points[v[1].Vertex]);

	double t = 1.0;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = to_dvec3(points[v[i].Vertex]);
		t = std::min(TriangleSphereIntersect(from, to, radius, p), t);
		p[1] = p[2];
	}
	return t;
}

double ULevel::TriangleRayIntersect(const dvec4& from, const dvec4& to, const dvec3* p)
{
	// Moeller–Trumbore ray-triangle intersection algorithm:

	dvec3 D = to.xyz() - from.xyz();

	// Find vectors for two edges sharing p[0]
	dvec3 e1 = p[1] - p[0];
	dvec3 e2 = p[2] - p[0];

	// Begin calculating determinant - also used to calculate u parameter
	dvec3 P = cross(D, e2);
	double det = dot(e1, P);

	// Backface check
	//if (det < 0.0)
	//	return 1.0;

	// If determinant is near zero, ray lies in plane of triangle
	if (det > -FLT_EPSILON && det < FLT_EPSILON)
		return 1.0;

	double inv_det = 1.0 / det;

	// Calculate distance from p[0] to ray origin
	dvec3 T = from.xyz() - p[0];

	// Calculate u parameter and test bound
	double u = dot(T, P) * inv_det;

	// Check if the intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
		return 1.0;

	// Prepare to test v parameter
	dvec3 Q = cross(T, e1);

	// Calculate V parameter and test bound
	double v = dot(D, Q) * inv_det;

	// The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f)
		return 1.0;

	double t = dot(e2, Q) * inv_det;
	if (t <= FLT_EPSILON)
		return 1.0;

	// Return hit location on triangle in barycentric coordinates
	// barycentricB = u;
	// barycentricC = v;

	return t;
}

double ULevel::TriangleSphereIntersect(const dvec4& from, const dvec4& to, double radius, const dvec3* p)
{
	dvec3 c = from.xyz();
	dvec3 e = to.xyz();
	double r = radius;

	// Dynamic intersection test between a ray and the minkowski sum of the sphere and polygon:

	dvec3 n = normalize(cross(p[1] - p[0], p[2] - p[0]));
	dvec4 plane(n, -dot(n, p[0]));

	// Step 1: Plane intersect test

	double sc = dot(plane, from);
	double se = dot(plane, to);
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

/////////////////////////////////////////////////////////////////////////////

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
