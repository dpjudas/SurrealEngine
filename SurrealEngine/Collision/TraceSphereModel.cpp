
#include "Precomp.h"
#include "TraceSphereModel.h"

std::vector<SweepHit> TraceSphereModel::Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, bool visibilityOnly)
{
	Model = model;
	std::vector<SweepHit> hits;
	Trace(origin, tmin, dirNormalized, tmax, radius, visibilityOnly, &Model->Nodes.front(), hits);
	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	return hits;
}

void TraceSphereModel::Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, bool visibilityOnly, BspNode* node, std::vector<SweepHit>& hits)
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
		polynode = &Model->Nodes[polynode->Plane];
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);

	if (node->Front >= 0 && (fromSide >= -radius || toSide >= -radius))
	{
		Trace(origin, tmin, dirNormalized, tmax, radius, visibilityOnly, &Model->Nodes[node->Front], hits);
	}

	if (node->Back >= 0 && (fromSide <= radius || toSide <= radius))
	{
		Trace(origin, tmin, dirNormalized, tmax, radius, visibilityOnly, &Model->Nodes[node->Back], hits);
	}
}

double TraceSphereModel::NodeSphereIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, double radius, BspNode* node)
{
	if (node->NumVertices < 3 || (node->Surf >= 0 && Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return tmax;

	dvec3 target = origin + dirNormalized * tmax;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(target, 1.0), plane);
	if ((fromSide > radius && toSide > radius) || (fromSide < -radius && toSide < -radius))
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
		double tval = TriangleSphereIntersect(origin, target, radius, p) * tmax;
		if (tval >= tmin)
			t = std::min(tval, t);
		p[1] = p[2];
	}
	return t;
}

double TraceSphereModel::TriangleSphereIntersect(const dvec3& from, const dvec3& to, double radius, const dvec3* p)
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
