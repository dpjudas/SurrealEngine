#pragma once

#include "Math/vec.h"
#include "Math/mat.h"

class UActor;
class Rotator;
struct MeshFace;
class UMesh;
class ULodMesh;
class USkeletalMesh;
class UTexture;
struct FSceneNode;

class MeshRender
{
public:
	void DrawMesh(FSceneNode* frame, UActor* actor);
	void DrawMesh(FSceneNode* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
	void DrawLodMesh(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld, const vec3& color);
	void DrawLodMeshFace(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& color, int vertexOffset);
	void DrawSkeletalMesh(FSceneNode* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld, const vec3& color);

private:
	std::vector<UTexture*> textures;
};
