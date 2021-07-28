
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
	int animFrame = seq->StartFrame + (int)(actor->AnimFrame() * seq->NumFrames) % seq->NumFrames;
	DrawLodMeshFace(frame, actor, mesh, mesh->Faces, ObjectToWorld, color, mesh->SpecialVerts + animFrame * mesh->FrameVerts);
	DrawLodMeshFace(frame, actor, mesh, mesh->SpecialFaces, ObjectToWorld, color, animFrame * mesh->FrameVerts);
}

void MeshRender::DrawLodMeshFace(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& color, int vertexOffset)
{
	auto device = engine->window->GetRenderDevice();

	GouraudVertex vertices[3];
	for (const MeshFace& face : faces)
	{
		if (face.MaterialIndex >= mesh->Materials.size())
			continue;

		const MeshMaterial& material = mesh->Materials[face.MaterialIndex];

		UTexture* tex = nullptr;
		if (actor)
		{
			tex = actor->GetMultiskin(material.TextureIndex);
			if (!tex && material.TextureIndex == 0)
				tex = actor->Skin();
		}
		if (!tex && material.TextureIndex < mesh->Textures.size())
			tex = mesh->Textures[material.TextureIndex];

		if (tex)
			tex = tex->GetAnimTexture();

		FTextureInfo texinfo;
		texinfo.Texture = tex;
		texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;

		float uscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Width : 256) * (1.0f / 255.0f);
		float vscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Height : 256) * (1.0f / 255.0f);

		for (int i = 0; i < 3; i++)
		{
			const MeshWedge& wedge = mesh->Wedges[face.Indices[i]];
			int vertexIndex = wedge.Vertex + vertexOffset;

			vertices[i].Point = (ObjectToWorld * vec4(mesh->Verts[vertexIndex], 1.0f)).xyz();
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
