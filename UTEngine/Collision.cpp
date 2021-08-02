
#include "Precomp.h"
#include "Collision.h"
#include "UObject/ULevel.h"
#include "Engine.h"

// #define TEST_SWEEP
// #define TEST_SWEEP_WITH_RAY

bool Collision::TraceAnyHit(vec3 from, vec3 to)
{
	if (from == to)
		return false;

#ifdef TEST_SWEEP
	CylinderShape shape(from, 1.0, 1.0);
	return Sweep(&shape, to).Fraction != 1.0;
#else
	return TraceAnyHit(dvec4(from.x, from.y, from.z, 1.0), dvec4(to.x, to.y, to.z, 1.0), &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
#endif
}

bool Collision::TraceAnyHit(const dvec4& from, const dvec4& to, BspNode* node, BspNode* nodes)
{
	BspNode* polynode = node;
	while (true)
	{
		if (NodeRayIntersect(from, to, polynode) < 1.0)
			return true;

		if (polynode->Plane < 0) break;
		polynode = nodes + polynode->Plane;
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);

	if (node->Front >= 0 && (fromSide >= 0.0 || toSide >= 0.0) && TraceAnyHit(from, to, nodes + node->Front, nodes))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0 || toSide <= 0.0) && TraceAnyHit(from, to, nodes + node->Back, nodes))
		return true;
	else
		return false;
}

SweepHit Collision::Sweep(CylinderShape* shape, const vec3& toF)
{
	dvec3 to(toF.x, toF.y, toF.z);

	if (shape->Center == to)
		return {};

	// Add margin:
	double margin = 3.0;
	dvec3 direction = to - shape->Center;
	double moveDistance = length(direction);
	direction /= moveDistance;
	dvec3 finalTo = to + direction * margin;

	// Do the collision sweep:
	dvec3 offset = dvec3(0.0, 0.0, shape->Height - shape->Radius);
	SweepHit top = Sweep(dvec4(shape->Center + offset, 1.0), dvec4(finalTo + offset, 1.0), shape->Radius, &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
	SweepHit bottom = Sweep(dvec4(shape->Center - offset, 1.0), dvec4(finalTo - offset, 1.0), shape->Radius, &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
	SweepHit hit = top.Fraction < bottom.Fraction ? top : bottom;

	// Apply margin to result:
	if (hit.Fraction < 1.0)
	{
		hit.Fraction = (float)clamp(((moveDistance + margin) * hit.Fraction - margin) / moveDistance, 0.0, 1.0);
		if (dot(dvec3(hit.Normal.x, hit.Normal.y, hit.Normal.z), direction) < 0.0)
			hit.Normal = -hit.Normal;
	}
	return hit;
}

SweepHit Collision::Sweep(const dvec4& from, const dvec4& to, double radius, BspNode* node, BspNode* nodes)
{
	SweepHit hit;

	BspNode* polynode = node;
	while (true)
	{
		double t = NodeSphereIntersect(from, to, radius, polynode);
		if (t < hit.Fraction)
		{
			hit.Fraction = (float)t;
			hit.Normal = { node->PlaneX, node->PlaneY, node->PlaneZ };
		}

		if (polynode->Plane < 0) break;
		polynode = nodes + polynode->Plane;
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);

	if (node->Front >= 0 && (fromSide >= -radius || toSide >= -radius))
	{
		SweepHit childhit = Sweep(from, to, radius, nodes + node->Front, nodes);
		if (childhit.Fraction < hit.Fraction)
			hit = childhit;
	}

	if (node->Back >= 0 && (fromSide <= radius || toSide <= radius))
	{
		SweepHit childhit = Sweep(from, to, radius, nodes + node->Back, nodes);
		if (childhit.Fraction < hit.Fraction)
			hit = childhit;
	}

	return hit;
}

static dvec3 to_dvec3(vec3 v) { return dvec3(v.x, v.y, v.z); }

double Collision::NodeRayIntersect(const dvec4& from, const dvec4& to, BspNode* node)
{
	if (node->NumVertices < 3 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && engine->Level->Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return 1.0;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);
	if ((fromSide > 0.0 && toSide > 0.0) || (fromSide < 0.0 && toSide < 0.0))
		return 1.0;

	BspVert* v = &engine->Level->Model->Vertices[node->VertPool];
	vec3* points = engine->Level->Model->Points.data();

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

double Collision::NodeSphereIntersect(const dvec4& from, const dvec4& to, double radius, BspNode* node)
{
	if (node->NumVertices < 3 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && engine->Level->Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return 1.0;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(from, plane);
	double toSide = dot(to, plane);
	if ((fromSide > radius && toSide > radius) || (fromSide < -radius && toSide < -radius))
		return 1.0;

	BspVert* v = &engine->Level->Model->Vertices[node->VertPool];
	vec3* points = engine->Level->Model->Points.data();

	dvec3 p[3];
	p[0] = to_dvec3(points[v[0].Vertex]);
	p[1] = to_dvec3(points[v[1].Vertex]);

	double t = 1.0;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = to_dvec3(points[v[i].Vertex]);
#ifdef TEST_SWEEP_WITH_RAY
		t = std::min(TriangleRayIntersect(from, to, p), t);
#else
		t = std::min(TriangleSphereIntersect(from, to, radius, p), t);
#endif
		p[1] = p[2];
	}
	return t;
}

double Collision::TriangleRayIntersect(const dvec4& from, const dvec4& to, const dvec3* p)
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

double Collision::TriangleSphereIntersect(const dvec4& from, const dvec4& to, double radius, const dvec3* p)
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
