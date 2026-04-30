#pragma once

#include "Math/mat.h"

class UActor;
class UMesh;
class USkeletalMesh;
struct MeshFace;
class ULodMesh;
class VisibleFrame;

struct BlendInfo
{
	int offsets[3];
	float t0, t1;
	float weight;
};

class VisibleMesh
{
public:
	bool DrawMesh(VisibleFrame* frame, UActor* actor, bool wireframe, bool translucentPass);

private:
	bool DrawMesh(VisibleFrame* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	bool DrawMeshDX(VisibleFrame* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	bool DrawLodMesh(VisibleFrame* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	bool DrawLodMeshDX(VisibleFrame* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	bool DrawLodMeshFace(VisibleFrame* frame, UActor* actor, ULodMesh* mesh, const Array<MeshFace>& faces, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, int baseVertexOffset, const int* vertexOffsets, float t0, float t1, bool translucentPass);
	bool DrawLodMeshFaceDX(VisibleFrame* frame, UActor* actor, ULodMesh* mesh, const Array<MeshFace>& faces, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, int baseVertexOffset, const int* vertexOffsets, float t0, float t1, bool translucentPass, BlendInfo* blends, int blendCount);
	bool DrawSkeletalMesh(VisibleFrame* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld, const mat3& ObjectNormalToWorld, bool translucentPass);
	void SetupMeshTextures(UActor* actor, UMesh* mesh);
	void SetupLodMeshTextures(UActor* actor, ULodMesh* mesh);
};
