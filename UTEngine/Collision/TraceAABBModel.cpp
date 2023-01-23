
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
	// Simplify trace by re-orienting to the origin
	BBox aabb(to_vec3(-extents), to_vec3(extents));
	BspNode* polynode = node;
	while (true)
	{
		bool blocking = !visibilityOnly || (polynode->NodeFlags & NF_NotVisBlocking) == 0;
		if (blocking)
		{
			double t = NodeAABBIntersect(origin, tmin, dirNormalized, tmax, aabb, extents, polynode);
			if (t < tmax && t >= tmin)
			{
				SweepHit hit = {(float)t, vec3(node->PlaneX, node->PlaneY, node->PlaneZ), nullptr};
				if (dot(to_dvec3(hit.Normal), dirNormalized) > 0.0)
					hit.Normal = -hit.Normal;
				hits.push_back(hit);
			}
		}

		if (polynode->Plane < 0) break;
		polynode = &Model->Nodes[polynode->Plane];
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

double TraceAABBModel::TriangleAABBIntersect(const dvec3& origin, double tmax, const BBox& aabb, const dvec3& extents, const dvec3* points)
{
	// Simplify check by re-orienting to center
	dvec3 v[3] =
	{
		points[0] - origin,
		points[1] - origin,
		points[2] - origin
	};

	dvec3 f[3] =
	{
		v[1] - v[0],
		v[2] - v[1],
		v[0] - v[2]
	};

	// TODO: Optimize
	//dvec3 a[9] =
	//{
	//	{0, -f[0].z, f[0].y}, // a00
	//	{0, -f[1].z, f[1].y}, // a01
	//	{0, -f[2].z, f[2].y}, // a02
	//	{f[0].z, 0, -f[0].x}, // a10
	//	{f[1].z, 0, -f[1].x}, // a11
	//	{f[2].z, 0, -f[2].x}, // a12
	//	{-f[0].y, f[0].x, 0}, // a20
	//	{-f[1].y, f[1].x, 0}, // a21
	//	{-f[2].y, f[2].x, 0}  // a22
	//};

	dvec3 p;
	double r;
	double m[2];


	//for (int i = 0; i < 9; i++)
	//{
	//	p[0] = dot(v[0], a[i]);
	//	p[1] = dot(v[1], a[i]);
	//	p[2] = dot(v[2], a[i]);
	//	r = extents.x * abs(a[i].x) + extents.y * abs(a[i].y) + extents.z * abs(a[i].z);
	//
	//	double m[2];
	//	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	//	m[1] = std::min(std::min(p[0], p[1]), p[2]);
	//
	//	if (std::max(-m[0], m[1]) > r)
	//		return tmax;
	//}

	// Test axes a00 - a22 (category 3)
	// a00
	dvec3 a00(0, -f[0].z, f[0].y);
	p[0] = dot(v[0], a00);
	p[1] = dot(v[1], a00);
	p[2] = dot(v[2], a00);
	r = extents.y * abs(f[0].z) + extents.z * abs(f[0].y);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a01
	dvec3 a01(0, -f[1].z, f[1].y);
	p[0] = dot(v[0], a01);
	p[1] = dot(v[1], a01);
	p[2] = dot(v[2], a01);
	r = extents.y * abs(f[1].z) + extents.z * abs(f[1].y);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a02
	dvec3 a02(0, -f[2].z, f[2].y);
	p[0] = dot(v[0], a02);
	p[1] = dot(v[1], a02);
	p[2] = dot(v[2], a02);
	r = extents.y * abs(f[2].z) + extents.z * abs(f[2].y);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a10
	dvec3 a10(f[0].z, 0, -f[0].x);
	p[0] = dot(v[0], a10);
	p[1] = dot(v[1], a10);
	p[2] = dot(v[2], a10);
	r = extents.x * abs(f[0].z) + extents.z * abs(f[0].x);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a11
	dvec3 a11(f[1].z, 0, -f[1].x);
	p[0] = dot(v[0], a11);
	p[1] = dot(v[1], a11);
	p[2] = dot(v[2], a11);
	r = extents.x * abs(f[1].z) + extents.z * abs(f[1].x);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a12
	dvec3 a12(f[2].z, 0, -f[2].x);
	p[0] = dot(v[0], a12);
	p[1] = dot(v[1], a12);
	p[2] = dot(v[2], a12);
	r = extents.x * abs(f[2].z) + extents.z * abs(f[2].x);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a20
	dvec3 a20(-f[0].y, f[0].x, 0);
	p[0] = dot(v[0], a20);
	p[1] = dot(v[1], a20);
	p[2] = dot(v[2], a20);
	r = extents.x * abs(f[0].y) + extents.y * abs(f[0].x);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a21
	dvec3 a21(-f[1].y, f[1].x, 0);
	p[0] = dot(v[0], a21);
	p[1] = dot(v[1], a21);
	p[2] = dot(v[2], a21);
	r = extents.x * abs(f[1].y) + extents.y * abs(f[1].x);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// a22
	dvec3 a22(-f[2].y, f[2].x, 0);
	p[0] = dot(v[0], a22);
	p[1] = dot(v[1], a22);
	p[2] = dot(v[2], a22);
	r = extents.x * abs(f[2].y) + extents.y * abs(f[2].x);

	m[0] = std::max(std::max(p[0], p[1]), p[2]);
	m[1] = std::min(std::min(p[0], p[1]), p[2]);

	if (std::max(-m[0], m[1]) > r)
		return tmax;

	// Test the three axes corresponding to the face normals of the AABB (category 1)
	if (std::max(std::max(v[0].x, v[1].x), v[2].x) < -extents[0] || std::min(std::min(v[0].x, v[1].x), v[2].x) > extents[0])
		return tmax;

	if (std::max(std::max(v[0].y, v[1].y), v[2].y) < -extents[1] || std::min(std::min(v[0].y, v[1].y), v[2].y) > extents[1])
		return tmax;

	if (std::max(std::max(v[0].z, v[1].z), v[2].z) < -extents[2] || std::min(std::min(v[0].z, v[1].z), v[2].z) > extents[2])
		return tmax;

	// Test separating axis corresponding to triangle face normal (category 2)
	dvec3 pn = cross(f[0], f[1]);
	double pd = abs(dot(pn, v[0]));

	r = extents.x * abs(pn.x) + extents.y * abs(pn.y) + extents.z * abs(pn.z);
	if (pd > r)
		return tmax;

	double t = (pd / r) * tmax;
	return t;
}

double TraceAABBModel::NodeAABBIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const BBox& aabb, const dvec3& extents, BspNode* node)
{
	if (node->NumVertices < 3 || (node->Surf >= 0 && Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
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
		double tval = TriangleAABBIntersect(origin, tmax, aabb, extents, p) * tmax;
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
