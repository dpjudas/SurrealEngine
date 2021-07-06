#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

class Rotator;
struct MeshFace;
class UMesh;
class ULodMesh;
class USkeletalMesh;
struct FSceneNode;

class MeshRender
{
public:
	void DrawMesh(FSceneNode* frame, UMesh* mesh, const vec3& location, const Rotator& rotation, float drawscale, int zoneIndex);
	void DrawMesh(FSceneNode* frame, UMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
	void DrawLodMesh(FSceneNode* frame, ULodMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
	void DrawLodMeshFace(FSceneNode* frame, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& color, int vertexOffset);
	void DrawSkeletalMesh(FSceneNode* frame, USkeletalMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
};
