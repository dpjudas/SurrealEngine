
#include "Precomp.h"
#include "TraceAABBModel.h"

std::vector<SweepHit> TraceAABBModel::Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly)
{
	Model = model;
	std::vector<SweepHit> hits;
	Trace(origin, tmin, dirNormalized, tmax, extents, visibilityOnly, &Model->Nodes.front(), hits);
	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	return hits;
}

void TraceAABBModel::Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly, BspNode* node, std::vector<SweepHit>& hits)
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
		double boxEpsilon = 0.1;
		bbox.min += boxEpsilon;
		bbox.max -= boxEpsilon;

		SweepCursor cursor(origin, dirNormalized, tmax, extents);
		if (cursor.ClipBoxPlanes(bbox))
		{
			// Grab the hull planes and flip the plane direction if the plane points in the wrong direction.
			std::vector<dvec4> planes;
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
				SweepHit hit = { (float)t, vec3(cursor.HitNormal()), nullptr };
				hits.push_back(hit);
			}
		}
	}

	dvec3 extentspadded = extents * 1.1; // For numerical stability
	int startSide = NodeAABBOverlap(origin, extentspadded, node);
	int endSide = NodeAABBOverlap(origin + dirNormalized * tmax, extentspadded, node);

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
		r = extents.x * abs(a[i].x) + extents.y * abs(a[i].y) + extents.z * abs(a[i].z);
	
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
	double pd = abs(dot(pn, v[0]));

	r = extents.x * abs(pn.x) + extents.y * abs(pn.y) + extents.z * abs(pn.z);
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

/*bool TraceAABBModel::IntersectMovingAABBAABB(const BBox& a, const BBox& b, const dvec3& start, const dvec3& end)
{
	if (!a.intersects(b))
		return false;

	dvec3 v = end - start;
	double t0 = 0.0;
	double t1 = 1.0;

	for (int i = 0; i < 3; i++)
	{
		if (v[i] < 0.0)
		{
			if (b.max[i] < a.min[i])
				return false;

			if (a.max[i] < b.min[i])
				t0 = std::max((a.max[i] - b.min[i]) / v[i], t0);

			if (b.max[i] > a.min[i])
				t1 = std::min((a.min[i] - b.max[i]) / v[i], t1);
		}
		if (v[i] > 0.0)
		{
			if (b.min[i] > a.max[i])
				return false;

			if (b.max[i] < a.min[i])
				t0 = std::max((a.min[i] - b.max[i]) / v[i], t0);

			if (a.max[i] > b.min[i]) 
				t1 = std::min((a.max[i] - b.min[i]) / v[i], t1);
		}

		if (t0 > t1)
			return false;
	}

	return true;
}*/
