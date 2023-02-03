#pragma once

#include "UObject/ULevel.h"

class TraceAABBModel
{
public:
	std::vector<SweepHit> Trace(UModel* model, const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly);

private:
	void Trace(const dvec3& origin, double tmin, const dvec3& dirNormalized, double tmax, const dvec3& extents, bool visibilityOnly, BspNode* node, std::vector<SweepHit>& hits);

	static int NodeAABBOverlap(const dvec3& center, const dvec3& extents, BspNode* node);

	struct SweepCursor
	{
		SweepCursor(const dvec3& origin, const dvec3& dir, double tmax, const dvec3& extents) : start(origin), end(origin + dir * tmax), tmax(tmax), extents(extents)
		{
		}

		bool ClipPlane(const dvec4& plane)
		{
			// Push out plane by the extents of the AABB
			double aabbpushout = std::abs(plane.x * extents.x) + std::abs(plane.y * extents.y) + std::abs(plane.z * extents.z);

			double dist0 = plane.x * start.x + plane.y * start.y + plane.z * start.z - plane.w;
			double dist1 = plane.x * end.x + plane.y * end.y + plane.z * end.z - plane.w;
			double planeFrontFace = dist0 - dist1;

			// Is the AABB intersecting with the plane?
			double aabbdist = dist0 - aabbpushout;
			if (dist0 > dist1 && aabbdist >= -aabbpushout && aabbdist < 0.0)
				aabbdist = 0.0;

			// Where did we hit the plane?
			double t = aabbdist / (dist0 - dist1);

			if (planeFrontFace > 0.00001) // Plane normal is pointing towards our movement direction. Update the start hit location for the line segment
			{
				if (t > tstart)
				{
					tstart = t;
					hitnormal = plane.xyz();
				}
			}
			else if (planeFrontFace < -0.00001) // Plane normal is pointing away from our movement direction. Update the end hit location for the line segment
			{
				tend = std::min(t, tend);
			}
			else if (dist0 > aabbpushout && dist1 > aabbpushout) // Line segment never hits our plane
			{
				nohit = true;
				return false;
			}

			if (tstart < tend)
			{
				return true;
			}
			else
			{
				nohit = true;
				return false;
			}
		}

		bool ClipBevel(const dvec4& plane0, const dvec4& plane1, const dvec3& bevelDirection)
		{
			dvec3 cross1 = cross(bevelDirection, plane0.xyz());
			dvec3 cross2 = cross(bevelDirection, plane1.xyz());
			if (dot(cross1, cross2) > 0.00001)
			{
				dvec3 linedir = cross(plane0.xyz(), plane1.xyz());
				double length2 = dot(linedir, linedir);
				if (length2 >= 0.000001)
				{
					dvec3 point = (plane0.w * cross(plane1.xyz(), linedir) + plane1.w * cross(linedir, plane0.xyz())) / length2;
					linedir = normalize(linedir);

					dvec3 normal = normalize(cross(bevelDirection, linedir));
					if (dot(plane0.xyz(), normal) < 0.0)
					{
						normal = -normal;
					}

					dvec4 bevelplane(normal, dot(point, normal));
					if (!ClipPlane(bevelplane))
					{
						return false;
					}
				}
			}
			return true;
		}

		double HitFraction()
		{
			if (!nohit && tstart > -1.0 && tstart < tend && tend > 0.0)
			{
				return std::max(tstart * tmax - 0.1, 0.0);
			}
			else
			{
				return tmax;
			}
		}

		dvec3 HitNormal()
		{
			return hitnormal;
		}

	private:
		dvec3 start;
		dvec3 end;
		double tstart = -1.0;
		double tend = 2.0;
		double tmax;
		dvec3 extents;
		dvec3 hitnormal = { 0.0, 0.0, 0.0 };
		bool nohit = false;
	};

	UModel* Model = nullptr;
};
