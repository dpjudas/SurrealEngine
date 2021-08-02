
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
	CylinderShape shape(from, 1.0f, 1.0f);
	return Sweep(&shape, to).Fraction != 1.0f;
#else
	return TraceAnyHit(vec4(from, 1.0f), vec4(to, 1.0f), &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
#endif
}

bool Collision::TraceAnyHit(const vec4& from, const vec4& to, BspNode* node, BspNode* nodes)
{
	BspNode* polynode = node;
	while (true)
	{
		if (NodeRayIntersect(from, to, polynode) < 1.0f)
			return true;

		if (polynode->Plane < 0) break;
		polynode = nodes + polynode->Plane;
	}

	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);

	if (node->Front >= 0 && (fromSide >= 0.0f || toSide >= 0.0f) && TraceAnyHit(from, to, nodes + node->Front, nodes))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0f || toSide <= 0.0f) && TraceAnyHit(from, to, nodes + node->Back, nodes))
		return true;
	else
		return false;
}

SweepHit Collision::Sweep(CylinderShape* shape, const vec3& to)
{
	if (shape->Center == to)
		return {};

	vec3 offset = vec3(0.0f, 0.0f, shape->Height - shape->Radius);
	SweepHit top = Sweep(vec4(shape->Center + offset, 1.0f), vec4(to + offset, 1.0f), shape->Radius, &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
	SweepHit bottom = Sweep(vec4(shape->Center - offset, 1.0f), vec4(to - offset, 1.0f), shape->Radius, &engine->Level->Model->Nodes.front(), engine->Level->Model->Nodes.data());
	return top.Fraction < bottom.Fraction ? top : bottom;
}

SweepHit Collision::Sweep(const vec4& from, const vec4& to, float radius, BspNode* node, BspNode* nodes)
{
	SweepHit hit;

	BspNode* polynode = node;
	while (true)
	{
		float t = NodeSphereIntersect(from, to, radius, polynode);
		if (t < hit.Fraction)
		{
			hit.Fraction = t;
			hit.Normal = { node->PlaneX, node->PlaneY, node->PlaneZ };
		}

		if (polynode->Plane < 0) break;
		polynode = nodes + polynode->Plane;
	}

	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);

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

float Collision::NodeRayIntersect(const vec4& from, const vec4& to, BspNode* node)
{
	if (node->NumVertices < 3 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && engine->Level->Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return 1.0f;

	// Test if plane is actually crossed.
	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);
	if ((fromSide > 0.0f && toSide > 0.0f) || (fromSide < 0.0f && toSide < 0.0f))
		return 1.0f;

	BspVert* v = &engine->Level->Model->Vertices[node->VertPool];
	vec3* points = engine->Level->Model->Points.data();

	vec3 p[3];
	p[0] = points[v[0].Vertex];
	p[1] = points[v[1].Vertex];

	float t = 1.0f;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = points[v[i].Vertex];
		t = std::min(TriangleRayIntersect(from, to, p), t);
		p[1] = p[2];
	}
	return t;
}

float Collision::NodeSphereIntersect(const vec4& from, const vec4& to, float radius, BspNode* node)
{
	if (node->NumVertices < 3 || (node->NodeFlags & NF_NotVisBlocking) || (node->Surf >= 0 && engine->Level->Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return 1.0f;

	// Test if plane is actually crossed.
	vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	float fromSide = dot(from, plane);
	float toSide = dot(to, plane);
	if ((fromSide > radius && toSide > radius) || (fromSide < -radius && toSide < -radius))
		return 1.0f;

	BspVert* v = &engine->Level->Model->Vertices[node->VertPool];
	vec3* points = engine->Level->Model->Points.data();

	vec3 p[3];
	p[0] = points[v[0].Vertex];
	p[1] = points[v[1].Vertex];

	float t = 1.0f;
	int count = node->NumVertices;
	for (int i = 2; i < count; i++)
	{
		p[2] = points[v[i].Vertex];
#ifdef TEST_SWEEP_WITH_RAY
		t = std::min(TriangleRayIntersect(from, to, p), t);
#else
		t = std::min(TriangleSphereIntersect(from, to, radius, p), t);
#endif
		p[1] = p[2];
	}
	return t;
}

float Collision::TriangleRayIntersect(const vec4& from, const vec4& to, const vec3* p)
{
	// Moeller–Trumbore ray-triangle intersection algorithm:

	vec3 D = to.xyz() - from.xyz();

	// Find vectors for two edges sharing p[0]
	vec3 e1 = p[1] - p[0];
	vec3 e2 = p[2] - p[0];

	// Begin calculating determinant - also used to calculate u parameter
	vec3 P = cross(D, e2);
	float det = dot(e1, P);

	// Backface check
	//if (det < 0.0f)
	//	return 1.0f;

	// If determinant is near zero, ray lies in plane of triangle
	if (det > -FLT_EPSILON && det < FLT_EPSILON)
		return 1.0f;

	float inv_det = 1.0f / det;

	// Calculate distance from p[0] to ray origin
	vec3 T = from.xyz() - p[0];

	// Calculate u parameter and test bound
	float u = dot(T, P) * inv_det;

	// Check if the intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
		return 1.0f;

	// Prepare to test v parameter
	vec3 Q = cross(T, e1);

	// Calculate V parameter and test bound
	float v = dot(D, Q) * inv_det;

	// The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f)
		return 1.0f;

	float t = dot(e2, Q) * inv_det;
	if (t <= FLT_EPSILON)
		return 1.0f;

	// Return hit location on triangle in barycentric coordinates
	// barycentricB = u;
	// barycentricC = v;

	return t;
}

float Collision::TriangleSphereIntersect(const vec4& from, const vec4& to, float radius, const vec3* p)
{
	vec3 c = from.xyz();
	vec3 e = to.xyz();
	float r = radius;

	// Dynamic intersection test between a ray and the minkowski sum of the sphere and polygon:

	vec3 n = normalize(cross(p[1] - p[0], p[2] - p[0]));
	vec4 plane(n, -dot(n, p[0]));

	// Step 1: Plane intersect test

	float sc = dot(plane, from);
	float se = dot(plane, to);
	bool same_side = sc * se > 0.0f;

	if (same_side && std::abs(sc) > r && std::abs(se) > r)
		return 1.0f;

	// Step 1a: Check if point is in polygon (using crossing ray test in 2d)
	float t = (sc - r) / (sc - se);
	if (t >= 0.0f && t <= 1.0f)
	{
		// To do: this can be precalculated for the mesh
		vec3 u0, u1;
		if (std::abs(n.x) > std::abs(n.y))
		{
			u0 = { 0.0f, 1.0f, 0.0f };
			if (std::abs(n.z) > std::abs(n.x))
				u1 = { 1.0f, 0.0f, 0.0f };
			else
				u1 = { 0.0f, 0.0f, 1.0f };
		}
		else
		{
			u0 = { 1.0f, 0.0f, 0.0f };
			if (std::abs(n.z) > std::abs(n.y))
				u1 = { 0.0f, 1.0f, 0.0f };
			else
				u1 = { 0.0f, 0.0f, 1.0f };
		}
		vec2 v_2d[3] =
		{
			vec2(dot(u0, p[0]), dot(u1, p[0])),
			vec2(dot(u0, p[1]), dot(u1, p[1])),
			vec2(dot(u0, p[2]), dot(u1, p[2])),
		};

		vec3 vt = c + (e - c) * t;
		vec2 point(dot(u0, vt), dot(u1, vt));

		bool inside = false;
		vec2 e0 = v_2d[2];
		bool y0 = e0.y >= point.y;
		for (int i = 0; i < 3; i++)
		{
			vec2 e1 = v_2d[i];
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

	vec3 ke[3] =
	{
		p[1] - p[0],
		p[2] - p[1],
		p[0] - p[2],
	};

	vec3 kg[3] =
	{
		p[0] - c,
		p[1] - c,
		p[2] - c,
	};

	vec3 ks = e - c;

	float kgg[3];
	float kgs[3];

	float kss = dot(ks, ks);
	float rr = r * r;

	float hitFraction = 1.0f;

	for (int i = 0; i < 3; i++)
	{
		float kee = dot(ke[i], ke[i]);
		float keg = dot(ke[i], kg[i]);
		float kes = dot(ke[i], ks);
		kgg[i] = dot(kg[i], kg[i]);
		kgs[i] = dot(kg[i], ks);

		float aa = kee * kss - kes * kes;
		float bb = 2 * (keg * kes - kee * kgs[i]);
		float cc = kee * (kgg[i] - rr) - keg * keg;

		float dsqr = bb * bb - 4 * aa * cc;
		if (dsqr >= 0.0f)
		{
			float sign = (bb >= 0.0f) ? 1.0f : -1.0f;
			float q = -0.5f * (bb + sign * std::sqrt(dsqr));
			float t0 = q / aa;
			float t1 = cc / q;

			float t;
			if (t0 < 0.0f || t0 > 1.0f)
				t = t1;
			else if (t1 < 0.0f || t1 > 1.0f)
				t = t0;
			else
				t = std::min(t0, t1);

			if (t >= 0.0f && t <= 1.0f)
			{
				vec3 ct = c + ks * t;
				float d = dot(ct - p[i], ke[i]);
				if (d >= 0.0f && d <= kee)
					hitFraction = std::min(t, hitFraction);
			}
		}
	}

	if (hitFraction < 1.0f)
		return hitFraction;

	// Step 3: Point intersect test

	for (int i = 0; i < 3; i++)
	{
		float aa = kss;
		float bb = -2.0f * kgs[i];
		float cc = kgg[i] - rr;

		float dsqr = bb * bb - 4 * aa * cc;
		if (dsqr >= 0.0f)
		{
			float sign = (bb >= 0.0f) ? 1.0f : -1.0f;
			float q = -0.5f * (bb + sign * std::sqrt(dsqr));
			float t0 = q / aa;
			float t1 = cc / q;

			float t;
			if (t0 < 0.0f || t0 > 1.0f)
				t = t1;
			else if (t1 < 0.0f || t1 > 1.0f)
				t = t0;
			else
				t = std::min(t0, t1);

			if (t >= 0.0f && t <= 1.0f)
				hitFraction = std::min(t, hitFraction);
		}
	}

	return hitFraction;
}
