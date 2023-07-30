
#include "Precomp.h"
#include "TraceRayModel.h"

TraceHitList TraceRayModel::Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly)
{
	Model = model;
	TraceHitList hits;
	Trace(origin, tmin, dirNormalized, tmax, visibilityOnly, &Model->Nodes.front(), hits);
	std::stable_sort(hits.begin(), hits.end(), [](const auto& a, const auto& b) { return a.Fraction < b.Fraction; });
	return hits;
}

bool TraceRayModel::TraceAnyHit(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly)
{
	Model = model;
	return TraceAnyHit(origin, tmin, dirNormalized, tmax, visibilityOnly, &Model->Nodes.front());
}

void TraceRayModel::Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node, TraceHitList& hits)
{
	BspNode* polynode = node;
	while (true)
	{
		if (!visibilityOnly || (polynode->NodeFlags & NF_NotVisBlocking) == 0)
		{
			double t = NodeRayIntersect(origin, tmin, dirNormalized, tmax, polynode);
			if (t >= tmin && t < tmax)
			{
				TraceHit hit = { (float)t, vec3(node->PlaneX, node->PlaneY, node->PlaneZ) };
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

	if (node->Front >= 0 && (fromSide >= 0.0 || toSide >= 0.0))
		Trace(origin, tmin, dirNormalized, tmax, visibilityOnly, &Model->Nodes[node->Front], hits);
	if (node->Back >= 0 && (fromSide <= 0.0 || toSide <= 0.0))
		Trace(origin, tmin, dirNormalized, tmax, visibilityOnly, &Model->Nodes[node->Back], hits);
}

bool TraceRayModel::TraceAnyHit(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, bool visibilityOnly, BspNode* node)
{
	BspNode* polynode = node;
	while (true)
	{
		if (!visibilityOnly || (polynode->NodeFlags & NF_NotVisBlocking) == 0)
		{
			double t = NodeRayIntersect(origin, tmin, dirNormalized, tmax, polynode);
			if (t >= tmin && t < tmax)
				return true;
		}

		if (polynode->Plane < 0) break;
		polynode = &Model->Nodes[polynode->Plane];
	}

	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);

	if (node->Front >= 0 && (fromSide >= 0.0 || toSide >= 0.0) && TraceAnyHit(origin, tmin, dirNormalized, tmax, visibilityOnly, &Model->Nodes[node->Front]))
		return true;
	else if (node->Back >= 0 && (fromSide <= 0.0 || toSide <= 0.0) && TraceAnyHit(origin, tmin, dirNormalized, tmax, visibilityOnly, &Model->Nodes[node->Back]))
		return true;
	else
		return false;
}

double TraceRayModel::NodeRayIntersect(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, BspNode* node)
{
	if (node->NumVertices < 3 || (node->Surf >= 0 && Model->Surfaces[node->Surf].PolyFlags & PF_NotSolid))
		return tmax;

	// Test if plane is actually crossed.
	dvec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
	double fromSide = dot(dvec4(origin, 1.0), plane);
	double toSide = dot(dvec4(origin + dirNormalized * tmax, 1.0), plane);
	if ((fromSide > 0.0 && toSide > 0.0) || (fromSide < 0.0 && toSide < 0.0))
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
		double tval = TriangleRayIntersect(origin, dirNormalized, tmax, p);
		if (tval >= tmin)
			t = std::min(tval, t);
		p[1] = p[2];
	}
	return t;
}

double TraceRayModel::TriangleRayIntersect(const dvec3& origin, const dvec3& dirNormalized, double tmax, const dvec3* p)
{
	// Moeller-Trumbore ray-triangle intersection algorithm:

	// Find vectors for two edges sharing p[0]
	dvec3 e1 = p[1] - p[0];
	dvec3 e2 = p[2] - p[0];

	// Begin calculating determinant - also used to calculate u parameter
	dvec3 P = cross(dirNormalized, e2);
	double det = dot(e1, P);

	// Backface check
	//if (det < 0.0)
	//	return tmax;

	// If determinant is near zero, ray lies in plane of triangle
	if (det > -FLT_EPSILON && det < FLT_EPSILON)
		return tmax;

	double inv_det = 1.0 / det;

	// Calculate distance from p[0] to ray origin
	dvec3 T = origin - p[0];

	// Calculate u parameter and test bound
	double u = dot(T, P) * inv_det;

	// Check if the intersection lies outside of the triangle
	if (u < 0.f || u > 1.f)
		return tmax;

	// Prepare to test v parameter
	dvec3 Q = cross(T, e1);

	// Calculate V parameter and test bound
	double v = dot(dirNormalized, Q) * inv_det;

	// The intersection lies outside of the triangle
	if (v < 0.f || u + v  > 1.f)
		return tmax;

	double t = dot(e2, Q) * inv_det;
	if (t <= FLT_EPSILON)
		return tmax;

	// Return hit location on triangle in barycentric coordinates
	// barycentricB = u;
	// barycentricC = v;

	return std::min(t, tmax);
}
