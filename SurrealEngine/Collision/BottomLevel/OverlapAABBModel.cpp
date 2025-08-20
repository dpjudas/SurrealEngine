
#include "Precomp.h"
#include "OverlapAABBModel.h"

CollisionHitList OverlapAABBModel::TestOverlap(UModel* model, const vec3& center, const vec3& extents, bool visibilityOnly)
{
	Model = model;
	CollisionHitList hits;
	TestOverlap(to_dvec3(center), to_dvec3(extents), visibilityOnly, &Model->Nodes.front(), hits);
	return hits;
}

void OverlapAABBModel::TestOverlap(const dvec3& center, const dvec3& extents, bool visibilityOnly, BspNode* node, CollisionHitList& hits)
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

		BBox bbox2;
		bbox2.min = to_vec3(center - extents);
		bbox2.max = to_vec3(center + extents);

		if (AABBOverlap(bbox, bbox2))
		{
			bool foundOutside = false;
			for (int i = 0; i < hullPlanesCount; i++)
			{
				int32_t hullIndex = hullIndexList[i];
				bool hullFlip = !!(hullIndex & 0x4000'0000);
				hullIndex = hullIndex & ~0x4000'0000;
				BspNode* hullnode = &Model->Nodes[hullIndex];
				dvec4 hullplane((double)hullnode->PlaneX, (double)hullnode->PlaneY, (double)hullnode->PlaneZ, (double)hullnode->PlaneW);

				int outsideside = hullFlip ? 1 : -1;
				if (PlaneAABBOverlap(center, extents, hullplane) == outsideside)
				{
					foundOutside = true;
					break;
				}
			}
			if (!foundOutside)
			{
				vec3 normal(0.0f);
				hits.push_back({ 0.0f, normal, nullptr, node, node });
			}
		}
	}

	dvec3 extentspadded = extents * 1.1; // For numerical stability
	int side = NodeAABBOverlap(center, extentspadded, node);

	if (node->Front >= 0 && side <= 0)
	{
		TestOverlap(center, extents, visibilityOnly, &Model->Nodes[node->Front], hits);
	}

	if (node->Back >= 0 && side >= 0)
	{
		TestOverlap(center, extents, visibilityOnly, &Model->Nodes[node->Back], hits);
	}
}

// -1 = inside, 0 = intersects, 1 = outside
int OverlapAABBModel::NodeAABBOverlap(const dvec3& center, const dvec3& extents, BspNode* node)
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

// -1 = inside, 0 = intersects, 1 = outside
int OverlapAABBModel::PlaneAABBOverlap(const dvec3& center, const dvec3& extents, const dvec4& plane)
{
	double e = extents.x * std::abs(plane.x) + extents.y * std::abs(plane.y) + extents.z * std::abs(plane.z);
	double s = center.x * plane.x + center.y * plane.y + center.z * plane.z - plane.w;
	if (s - e > 0)
		return -1;
	else if (s + e < 0)
		return 1;
	else
		return 0;
}

bool OverlapAABBModel::AABBOverlap(const BBox& a, const BBox& b)
{
	if (a.min.x > b.max.x || b.min.x > a.max.x ||
		a.min.y > b.max.y || b.min.y > a.max.y ||
		a.min.z > b.max.z || b.min.z > a.max.z)
	{
		return false;
	}
	else
	{
		return true;
	}
}
