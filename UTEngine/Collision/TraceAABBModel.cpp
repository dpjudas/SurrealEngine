
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
			hullIndexList[hullPlanesCount++];
		vec3* bboxStart = (vec3*)(&hullIndexList[hullPlanesCount + 1]);

		BBox bbox;
		bbox.min = bboxStart[0];
		bbox.max = bboxStart[1];

		double t = tmax;
		for (int i = 0; i < hullPlanesCount; i++)
		{
			int32_t hullIndex = hullIndexList[i];
			bool hullFlip = !!(hullIndex & 0x4000'0000);
			hullIndex = hullIndex & ~0x4000'0000;
			BspNode* hullnode = &Model->Nodes[hullIndex];

			// To do: perform correct hull planes intersection test
			t = HullNodeAABBIntersect(origin, tmin, dirNormalized, tmax, extents, hullnode, hullFlip);
		}

		if (t < tmax && t >= tmin)
		{
			SweepHit hit = { (float)t, vec3(node->PlaneX, node->PlaneY, node->PlaneZ), nullptr };
			if (dot(to_dvec3(hit.Normal), dirNormalized) > 0.0)
				hit.Normal = -hit.Normal;
			hits.push_back(hit);
		}
	}

	int startSide = NodeAABBOverlap(origin, extents, node);
	int endSide = NodeAABBOverlap(origin + dirNormalized * tmax, extents, node);

	if (node->Front >= 0 && (startSide <= 0 || endSide <= 0))
	{
		Trace(origin, tmin, dirNormalized, tmax, extents, visibilityOnly, &Model->Nodes[node->Front], hits);
	}

	if (node->Back >= 0 && (startSide >= 0 || endSide >= 0))
	{
		Trace(origin, tmin, dirNormalized, tmax, extents, visibilityOnly, &Model->Nodes[node->Back], hits);
	}
}

double TraceAABBModel::HullNodeAABBIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, BspNode* node, bool flipPlane)
{
	// Find plane intersection points
	double e = extents.x * std::abs(node->PlaneX) + extents.y * std::abs(node->PlaneY) + extents.z * std::abs(node->PlaneZ);
	double s;
	if (!flipPlane)
		s = origin.x * node->PlaneX + origin.y * node->PlaneY + origin.z * node->PlaneZ - node->PlaneW;
	else
		s = node->PlaneW - origin.x * node->PlaneX - origin.y * node->PlaneY - origin.z * node->PlaneZ;
	double d = 1.0 / (dirNormalized.x + dirNormalized.y + dirNormalized.z);
	double t0 = (e - s) * d;
	double t1 = (e + s) * d;
	double t = tmax;
	if (t0 >= tmin)
		t = std::min(t, t0);
	if (t1 >= tmin)
		t = std::min(t, t1);
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
