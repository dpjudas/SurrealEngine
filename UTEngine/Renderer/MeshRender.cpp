
#include "Precomp.h"
#include "MeshRender.h"
#include "UObject/UMesh.h"
#include "UObject/UTexture.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"
#include "UTRenderer.h"
#include "Window/Window.h"

void MeshRender::DrawMesh(FSceneNode* frame, UMesh* mesh, const vec3& location, const Rotator& rotation, float drawscale, int zoneIndex)
{
	vec3 color = engine->renderer->light.FindLightAt(location, zoneIndex);

	mat4 rotate = mat4::rotate(radians(180.0f - rotation.RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(180.0f - rotation.PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(rotation.YawDegrees() - 90.0f), 0.0f, 0.0f, -1.0f);
	mat4 RotOrigin = mat4::rotate(radians(mesh->RotOrigin.RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(mesh->RotOrigin.PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(90.0f - mesh->RotOrigin.YawDegrees()), 0.0f, 0.0f, -1.0f);
	mat4 ObjectToWorld = mat4::translate(location) * rotate * RotOrigin * mat4::scale(mesh->Scale * drawscale) * mat4::translate(vec3(0.0f) - mesh->Origin);

	if (dynamic_cast<USkeletalMesh*>(mesh))
		DrawSkeletalMesh(frame, static_cast<USkeletalMesh*>(mesh), ObjectToWorld, color);
	else if (dynamic_cast<ULodMesh*>(mesh))
		DrawLodMesh(frame, static_cast<ULodMesh*>(mesh), ObjectToWorld, color);
	else
		DrawMesh(frame, mesh, ObjectToWorld, color);
}

void MeshRender::DrawMesh(FSceneNode* frame, UMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
}

void MeshRender::DrawLodMesh(FSceneNode* frame, ULodMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
	int animFrame = mesh->AnimSeqs.front().StartFrame;
	DrawLodMeshFace(frame, mesh, mesh->Faces, ObjectToWorld, color, mesh->SpecialVerts + animFrame * mesh->FrameVerts);
	DrawLodMeshFace(frame, mesh, mesh->SpecialFaces, ObjectToWorld, color, animFrame * mesh->FrameVerts);
}

void MeshRender::DrawLodMeshFace(FSceneNode* frame, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& color, int vertexOffset)
{
	auto device = engine->window->GetRenderDevice();

	GouraudVertex vertices[3];
	for (const MeshFace& face : faces)
	{
		const MeshMaterial& material = mesh->Materials[face.MaterialIndex];

		FTextureInfo texinfo;
		texinfo.Texture = mesh->Textures[material.TextureIndex];
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

void MeshRender::DrawSkeletalMesh(FSceneNode* frame, USkeletalMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
	DrawLodMesh(frame, mesh, ObjectToWorld, color);
}
