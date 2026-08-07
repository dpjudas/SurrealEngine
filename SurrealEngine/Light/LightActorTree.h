#pragma once

#include "Math/vec.h"

class UActor;

class LightActorTree
{
public:
	std::vector<UActor*> Lights;
	Array<UActor*> CollectedLights;

	void CreateTLAS();
	void CollectLights(const vec3& center, float radius);

private:
	void CollectLights(const vec3& center, float radius, int nodeIndex);

	enum class OverlapResult
	{
		disjoint,
		overlap
	};
	static OverlapResult TestSphereAABB(const vec3& center, float radius, const vec3& aabb_min, const vec3& aabb_max);

	int Subdivide(int* instances, int numInstances, const vec4* centroids, int* workBuffer);

	class NodeBBox
	{
	public:
		NodeBBox() = default;

		NodeBBox(const vec3& aabb_min, const vec3& aabb_max)
		{
			min = aabb_min;
			max = aabb_max;
			auto halfmin = aabb_min * 0.5f;
			auto halfmax = aabb_max * 0.5f;
			Center = halfmax + halfmin;
			Extents = halfmax - halfmin;
		}

		vec3 min;
		vec3 max;
		vec3 Center;
		vec3 Extents;
	};

	struct Node
	{
		Node() = default;
		Node(const vec3& aabb_min, const vec3& aabb_max, int blas_index) : aabb(aabb_min, aabb_max), blas_index(blas_index) {}
		Node(const vec3& aabb_min, const vec3& aabb_max, int left, int right) : aabb(aabb_min, aabb_max), left(left), right(right) {}

		bool IsLeaf() const { return blas_index != -1; }

		NodeBBox aabb;
		int left = -1;
		int right = -1;
		int blas_index = -1;
	};

	struct
	{
		std::vector<Node> Nodes;
		int Root = -1;
	} TLAS;

	struct
	{
		std::vector<int> leafs;
		std::vector<vec4> centroids;
		std::vector<int> workbuffer;
	} Scratch;
};
