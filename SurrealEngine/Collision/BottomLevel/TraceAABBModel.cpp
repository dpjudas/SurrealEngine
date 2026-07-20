
#include "Precomp.h"
#include "TraceAABBModel.h"
#include "TraceRayModel.h"

// WP-3 phase 2 diagnostic. See the SemisolidNodesSeen comment in the header.
static bool DebugSemisolid()
{
	static const bool enabled = getenv("SE_DEBUG_SEMISOLID") != nullptr;
	return enabled;
}

// WP-3 phase 2 diagnostic. Walks every semisolid surface in the level once and probes it with a
// player-sized extent sweep and with a ray, from the same start to the same end. If the ray hits
// semisolid geometry that the extent sweep misses, BUG-021 is confirmed as a collision-layer gap
// rather than a map or physics problem. Temporary; goes away with the diagnostic.
static void ProbeSemisolid(UModel* model, double ex, double ez, const char* label)
{
	int probed = 0, passedThrough = 0, stoppedOnSurface = 0, stoppedEarly = 0;
	int reportedExamples = 0;

	for (BspNode& node : model->Nodes)
	{
		if (node.Surf < 0 || !(model->Surfaces[node.Surf].PolyFlags & PF_Semisolid) || node.NumVertices < 3)
			continue;

		BspVert* verts = &model->Vertices[node.VertPool];
		dvec3 centroid(0.0, 0.0, 0.0);
		for (int i = 0; i < node.NumVertices; i++)
			centroid += to_dvec3(model->Points[verts[i].Vertex]);
		centroid /= (double)node.NumVertices;

		dvec3 normal((double)node.PlaneX, (double)node.PlaneY, (double)node.PlaneZ);
		if (dot(normal, normal) < 0.5)
			continue;
		normal = normalize(normal);

		const double reach = 96.0;
		dvec3 origin = centroid + normal * reach;
		dvec3 dir = -normal;
		const double tmax = reach * 2.0;

		// The ray is the control: it walks node polygons, so it stops on this very polygon. Probes where
		// it stops on something else are skipped - the polygon is occluded and the comparison says nothing.
		TraceRayModel ray;
		CollisionHitList rayHits = ray.Trace(model, origin, 0.0, dir, tmax, false);
		if (rayHits.empty() || rayHits.front().Node != &node)
			continue;

		double rayT = rayHits.front().Fraction;
		probed++;

		// A sweep that collides with this polygon must stop within one box extent of where the ray did.
		// Stopping much earlier means it hit neighbouring geometry instead, which proves nothing about
		// the semisolid surface - so that case is counted separately rather than as a success.
		TraceAABBModel sweep;
		CollisionHitList sweepHits = sweep.Trace(model, origin, 0.0, dir, tmax, dvec3(ex, ex, ez), false);
		double window = std::max(ex, ez) + 2.0;

		if (sweepHits.empty() || sweepHits.front().Fraction > rayT + 1.0)
		{
			passedThrough++;
			if (reportedExamples < 5)
			{
				reportedExamples++;
				fprintf(stderr, "[semisolid]   %s: passed through poly at (%.0f, %.0f, %.0f) normal (%.2f, %.2f, %.2f)\n",
					label, centroid.x, centroid.y, centroid.z, normal.x, normal.y, normal.z);
			}
		}
		else if (sweepHits.front().Fraction >= rayT - window)
			stoppedOnSurface++;
		else
			stoppedEarly++;
	}

	fprintf(stderr, "[semisolid] %s (extents r=%.0f h=%.0f): %d probed - through %d, on surface %d, early(inconclusive) %d\n",
		label, ex, ez, probed, passedThrough, stoppedOnSurface, stoppedEarly);
}

static void ScanSemisolidSurfaces(UModel* model)
{
	int semisolidNodes = 0;
	for (BspNode& node : model->Nodes)
		if (node.Surf >= 0 && (model->Surfaces[node.Surf].PolyFlags & PF_Semisolid))
			semisolidNodes++;

	fprintf(stderr, "[semisolid] level scan: %d of %d node(s) are semisolid\n", semisolidNodes, (int)model->Nodes.size());
	if (semisolidNodes == 0)
		return;

	ProbeSemisolid(model, 4.0, 4.0, "small box");
	ProbeSemisolid(model, 35.0, 46.0, "player box");
}

CollisionHitList TraceAABBModel::Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly)
{
	Model = model;

	if (DebugSemisolid() && !Scanning)
	{
		static bool scanned = false;
		if (!scanned)
		{
			scanned = true;
			Scanning = true;
			ScanSemisolidSurfaces(model);
			Scanning = false;
			Model = model;
		}
	}

	CollisionHitList hits;
	SemisolidNodesSeen = 0;
	SemisolidPolysHit = 0;
	Trace(origin, tmin, dirNormalized, tmax, extents, visibilityOnly, &Model->Nodes.front(), hits);

	// If a sweep walks through semisolid nodes and comes back with nothing, the extent trace cannot see
	// semisolid geometry at all - which is the whole hypothesis behind BUG-021. Rate limited: this runs
	// for every pawn move.
	if (DebugSemisolid() && SemisolidPolysHit > 0 && hits.empty())
	{
		static int reported = 0;
		// stderr, not LogMessage: the Logger only flushes on a clean Engine::Run exit, and BUG-044
		// segfaults the shutdown, so a LogMessage line here could be lost or read back stale.
		if ((reported++ % 60) == 0)
		{
			fprintf(stderr, "[semisolid] sweep from (%.1f, %.1f, %.1f) extents (r=%.1f h=%.1f) crossed %d semisolid node(s), %d of them polygon-intersected, and hit nothing [%d total]\n",
				origin.x, origin.y, origin.z, extents.x, extents.z, SemisolidNodesSeen, SemisolidPolysHit, reported);
		}
	}

	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	return hits;
}

void TraceAABBModel::Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly, BspNode* node, CollisionHitList& hits)
{
	if (node->CollisionBound >= 0)
	{
		int32_t* hullIndexList = &Model->LeafHulls[node->CollisionBound];
		int hullPlanesCount = 0;
		while (hullIndexList[hullPlanesCount] >= 0)
			hullPlanesCount++;

		vec3* bboxStart = (vec3*)(&hullIndexList[hullPlanesCount + 1]);

		BBox bbox;
		bbox.min = bboxStart[0];
		bbox.max = bboxStart[1];

		// Shave off part of the box, or ammo pickups can fall through the floor
		float boxEpsilon = 0.1f;
		bbox.min += boxEpsilon;
		bbox.max -= boxEpsilon;

		SweepCursor cursor(origin, dirNormalized, tmax, extents);
		if (cursor.ClipBoxPlanes(bbox))
		{
			// Grab the hull planes and flip the plane direction if the plane points in the wrong direction.
			Array<dvec4> planes;
			for (int i = 0; i < hullPlanesCount; i++)
			{
				int32_t hullIndex = hullIndexList[i];
				bool hullFlip = !!(hullIndex & 0x4000'0000);
				hullIndex = hullIndex & ~0x4000'0000;
				BspNode* hullnode = &Model->Nodes[hullIndex];
				dvec4 hullplane((double)hullnode->PlaneX, (double)hullnode->PlaneY, (double)hullnode->PlaneZ, (double)hullnode->PlaneW);
				planes.push_back(hullFlip ? -hullplane : hullplane);
			}

			// AABB/hull sweep test.
			//
			// This is the same as a ray/hull sweep test, except with extended and bevel planes so that it works for AABB.
			//
			// The basic idea here is that you can find the solid line segment of a ray passing through the planes of a convex hull.
			// While we are not interested in the line segment itself, the start of the line segment will give us the the hit point.
			//
			// We can sweep with an AABB instead of a ray by moving the planes outwards by the extents of the AABB. This will produce
			// inaccuracies in the result, which we can reduce by adding bevel planes when the angle between the planes passes a threshold.

			// Check for collision for each hull plane
			for (int i = 0; i < hullPlanesCount; i++)
			{
				if (!cursor.ClipPlane(planes[i]))
				{
					break;
				}
			}

			// Check for collision for any bevel plane we need to insert at the hull edges
			for (int i = 0; i < hullPlanesCount; i++)
			{
				dvec4 plane0 = planes[i];
				for (int j = 0; j < i; j++)
				{
					dvec4 plane1 = planes[j];

					if ((plane0.x < 0.0 && plane1.x > 0.0) || (plane0.x > 0.0 && plane1.x < 0.0))
					{
						cursor.ClipBevel(plane0, plane1, dvec3(1.0, 0.0, 0.0));
					}
					if ((plane0.y < 0.0 && plane1.y > 0.0) || (plane0.y > 0.0 && plane1.y < 0.0))
					{
						cursor.ClipBevel(plane0, plane1, dvec3(0.0, 1.0, 0.0));
					}
					if ((plane0.z < 0.0 && plane1.z > 0.0) || (plane0.z > 0.0 && plane1.z < 0.0))
					{
						cursor.ClipBevel(plane0, plane1, dvec3(0.0, 0.0, 1.0));
					}
				}
			}

			// Did we hit anything?
			double t = cursor.HitFraction();
			if (t >= tmin && t < tmax)
			{
				CollisionHit hit = { (float)t, vec3(cursor.HitNormal()), nullptr, node, node };
				hits.push_back(hit);
			}
		}
	}

	dvec3 extentspadded = extents * 1.1; // For numerical stability
	int startSide = NodeAABBOverlap(origin, extentspadded, node);
	int endSide = NodeAABBOverlap(origin + dirNormalized * tmax, extentspadded, node);

	if (DebugSemisolid() && node->Surf >= 0 && (Model->Surfaces[node->Surf].PolyFlags & PF_Semisolid) && startSide == 0)
	{
		SemisolidNodesSeen++;

		// Decisive check: run the (currently unwired) per-node polygon test on this semisolid node. If it
		// reports an intersection the leaf-hull sweep did not, the geometry is there to be hit and only the
		// wiring is missing - and it also proves NodeAABBIntersect itself works before we rely on it.
		if (NodeAABBIntersect(origin, tmin, dirNormalized, tmax, extents, node) < tmax)
			SemisolidPolysHit++;
	}

	if (node->Front >= 0 && (startSide <= 0 || endSide <= 0))
	{
		Trace(origin, tmin, dirNormalized, tmax, extents, visibilityOnly, &Model->Nodes[node->Front], hits);
	}

	if (node->Back >= 0 && (startSide >= 0 || endSide >= 0))
	{
		Trace(origin, tmin, dirNormalized, tmax, extents, visibilityOnly, &Model->Nodes[node->Back], hits);
	}
}

double TraceAABBModel::TriangleAABBIntersect(const dvec3& from, const dvec3& to, const dvec3& extents, const dvec3* points)
{
	// Simplify check by re-orienting to center
	dvec3 v[3] =
	{
		points[0] - to,
		points[1] - to,
		points[2] - to
	};

	dvec3 f[3] =
	{
		v[1] - v[0],
		v[2] - v[1],
		v[0] - v[2]
	};

	// TODO: Optimize
	dvec3 a[9] =
	{
		{0, -f[0].z, f[0].y}, // a00
		{0, -f[1].z, f[1].y}, // a01
		{0, -f[2].z, f[2].y}, // a02
		{f[0].z, 0, -f[0].x}, // a10
		{f[1].z, 0, -f[1].x}, // a11
		{f[2].z, 0, -f[2].x}, // a12
		{-f[0].y, f[0].x, 0}, // a20
		{-f[1].y, f[1].x, 0}, // a21
		{-f[2].y, f[2].x, 0}  // a22
	};

	dvec3 p;
	double r;

	for (int i = 0; i < 9; i++)
	{
		p[0] = dot(v[0], a[i]);
		p[1] = dot(v[1], a[i]);
		p[2] = dot(v[2], a[i]);
		r = extents.x * std::abs(a[i].x) + extents.y * std::abs(a[i].y) + extents.z * std::abs(a[i].z);
	
		double m[2];
		m[0] = std::max(std::max(p[0], p[1]), p[2]);
		m[1] = std::min(std::min(p[0], p[1]), p[2]);
	
		if (std::max(-m[0], m[1]) > r)
			return 1.0;
	}

	// Test the three axes corresponding to the face normals of the AABB (category 1)
	if (std::max(std::max(v[0].x, v[1].x), v[2].x) < -extents[0] || std::min(std::min(v[0].x, v[1].x), v[2].x) > extents[0])
		return 1.0;

	if (std::max(std::max(v[0].y, v[1].y), v[2].y) < -extents[1] || std::min(std::min(v[0].y, v[1].y), v[2].y) > extents[1])
		return 1.0;

	if (std::max(std::max(v[0].z, v[1].z), v[2].z) < -extents[2] || std::min(std::min(v[0].z, v[1].z), v[2].z) > extents[2])
		return 1.0;

	// Test separating axis corresponding to triangle face normal (category 2)
	dvec3 pn = cross(f[0], f[1]);
	double pd = std::abs(dot(pn, v[0]));

	r = extents.x * std::abs(pn.x) + extents.y * std::abs(pn.y) + extents.z * std::abs(pn.z);
	if (pd > r)
		return 1.0;

	// Get intersection time
	double t = (pd / r);
	return t;
}

double TraceAABBModel::NodeAABBIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, BspNode* node)
{
	if (node->NumVertices < 3 || (node->Surf >= 0 && Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return tmax;

	dvec3 target = origin + dirNormalized * tmax;

	// Test if plane is actually crossed
	dvec3 extentspadded = extents * 1.1; // For numerical stability
	int startSide = NodeAABBOverlap(origin, extentspadded, node);
	int endSide = NodeAABBOverlap(origin + dirNormalized * tmax, extentspadded, node);
	if (startSide == 1 && endSide == 1)
		return tmax;

	BspVert* v = &Model->Vertices[node->VertPool];
	vec3* points = Model->Points.data();

	dvec3 p[3];
	p[0] = to_dvec3(points[v[0].Vertex]);
	p[1] = to_dvec3(points[v[1].Vertex]);

	double t = tmax;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = to_dvec3(points[v[i].Vertex]);
		double tval = TriangleAABBIntersect(origin, target, extents, p) * tmax;
		if (tval >= tmin)
			t = std::min(tval, t);
		p[1] = p[2];
	}
	return t;
}

// -1 = inside, 0 = intersects, 1 = outside
int TraceAABBModel::NodeAABBOverlap(const dvec3& center, const dvec3& extents, BspNode* node)
{
	double e = extents.x * std::abs(node->PlaneX) + extents.y * std::abs(node->PlaneY) + extents.z * std::abs(node->PlaneZ);
	double s = center.x * node->PlaneX + center.y * node->PlaneY + center.z * node->PlaneZ - node->PlaneW;
	if (s - e > 0)
		return -1;
	else if (s + e < 0)
		return 1;
	else
		return 0;
}
