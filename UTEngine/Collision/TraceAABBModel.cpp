
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
		// To do: first do an AABB test for the bounds of the hull and early out if we don't hit it

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

		SweepCursor cursor(origin, dirNormalized, tmax, extents);

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

bool TraceAABBModel::IntersectMovingAABBAABB(const BBox& a, const BBox& b, const dvec3& start, const dvec3& end)
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
}
