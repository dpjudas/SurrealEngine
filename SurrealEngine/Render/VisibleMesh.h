#pragma once

#include "Math/mat.h"

class UActor;
class UMesh;
class USkeletalMesh;
struct MeshFace;
class ULodMesh;
class VisibleFrame;

class VisibleMesh
{
public:
	bool DrawMesh(VisibleFrame* frame, UActor* actor, bool wireframe, bool translucentPass);

private:
	bool DrawMesh(VisibleFrame* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	bool DrawLodMesh(VisibleFrame* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	bool DrawLodMeshFace(VisibleFrame* frame, UActor* actor, ULodMesh* mesh, const Array<MeshFace>& faces, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, int baseVertexOffset, const int* vertexOffsets, float t0, float t1, bool translucentPass);
	bool DrawSkeletalMesh(VisibleFrame* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	void SetupMeshTextures(UActor* actor, UMesh* mesh);
	void SetupLodMeshTextures(UActor* actor, ULodMesh* mesh);
};
