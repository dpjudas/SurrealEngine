
#include "Precomp.h"
#include "MeshRender.h"
#include "UObject/UMesh.h"
#include "UObject/UTexture.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "UTRenderer.h"
#include "Window/Window.h"

void MeshRender::DrawMesh(FSceneNode* frame, UActor* actor)
{
	UMesh* mesh = actor->Mesh();
	const vec3& location = actor->Location();
	const Rotator& rotation = actor->Rotation();
	int zoneIndex = actor->actorZone;
	const vec3& color = actor->light;

	if (!mesh)
		return;

	mat4 objectToWorld = mat4::translate(actor->Location() + actor->PrePivot()) * actor->Rotation().ToMatrix() * mat4::scale(actor->DrawScale());

	mat4 meshToObject = mesh->RotOrigin.ToMatrix() * mat4::scale(mesh->Scale) * mat4::translate(-mesh->Origin);
	mat4 meshToWorld = objectToWorld * meshToObject;

	if (dynamic_cast<USkeletalMesh*>(mesh))
		DrawSkeletalMesh(frame, actor, static_cast<USkeletalMesh*>(mesh), meshToWorld, color);
	else if (dynamic_cast<ULodMesh*>(mesh))
		DrawLodMesh(frame, actor, static_cast<ULodMesh*>(mesh), meshToWorld, color);
	else
		DrawMesh(frame, actor, mesh, meshToWorld, color);
}

void MeshRender::DrawMesh(FSceneNode* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
}

void MeshRender::DrawLodMesh(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
	MeshAnimSeq* seq = mesh->GetSequence(actor->AnimSequence());

	float animFrame = std::max(actor->AnimFrame(), 0.0f) * seq->NumFrames;

	int frame0 = (int)animFrame;
	int frame1 = frame0 + 1;
	float t = animFrame - (float)frame0;

	frame0 = frame0 % seq->NumFrames;
	frame1 = frame1 % seq->NumFrames;

	int vertexOffset0 = (seq->StartFrame + frame0) * mesh->FrameVerts;
	int vertexOffset1 = (seq->StartFrame + frame1) * mesh->FrameVerts;

	DrawLodMeshFace(frame, actor, mesh, mesh->Faces, ObjectToWorld, color, mesh->SpecialVerts + vertexOffset0, mesh->SpecialVerts + vertexOffset1, t);
	DrawLodMeshFace(frame, actor, mesh, mesh->SpecialFaces, ObjectToWorld, color, vertexOffset0, vertexOffset1, t);
}

void MeshRender::DrawLodMeshFace(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& color, int vertexOffset0, int vertexOffset1, float t)
{
	auto device = engine->window->GetRenderDevice();

	if (textures.size() < mesh->Textures.size())
		textures.resize(mesh->Textures.size());

	for (int i = 0; i < (int)mesh->Textures.size(); i++)
	{
		UTexture* tex = actor->GetMultiskin(i);

		if (i == 0)
		{
			if (!tex) tex = actor->Skin();
			if (!tex) tex = mesh->Textures[i];
		}
		else
		{
			if (!tex) tex = mesh->Textures[i];
			if (!tex) tex = actor->Skin();
		}

		if (tex)
			tex = tex->GetAnimTexture();

		textures[i] = tex;
	}

	GouraudVertex vertices[3];
	for (const MeshFace& face : faces)
	{
		if (face.MaterialIndex >= mesh->Materials.size())
			continue;

		const MeshMaterial& material = mesh->Materials[face.MaterialIndex];

		UTexture* tex = textures[material.TextureIndex];

		FTextureInfo texinfo;
		texinfo.Texture = tex;
		texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;

		float uscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Width : 256) * (1.0f / 255.0f);
		float vscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Height : 256) * (1.0f / 255.0f);

		for (int i = 0; i < 3; i++)
		{
			const MeshWedge& wedge = mesh->Wedges[face.Indices[i]];
			const vec3& v0 = mesh->Verts[wedge.Vertex + vertexOffset0];
			const vec3& v1 = mesh->Verts[wedge.Vertex + vertexOffset1];
			vec3 vertex = mix(v0, v1, t);

			vertices[i].Point = (ObjectToWorld * vec4(vertex, 1.0f)).xyz();
			vertices[i].UV = { wedge.U * uscale, wedge.V * vscale };
			vertices[i].Light = color;
		}

		device->DrawGouraudPolygon(frame, texinfo.Texture ? &texinfo : nullptr, vertices, 3, material.PolyFlags);
	}
}

void MeshRender::DrawSkeletalMesh(FSceneNode* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
	DrawLodMesh(frame, actor, mesh, ObjectToWorld, color);
}
