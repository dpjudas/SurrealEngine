
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
	float animFrame = actor->AnimFrame() * seq->NumFrames;

	int vertexOffsets[3];
	float t0, t1;

	if (animFrame >= 0.0f)
	{
		int frame0 = (int)animFrame;
		int frame1 = frame0 + 1;
		t0 = animFrame - (float)frame0;
		t1 = 0.0f;
		frame0 = frame0 % seq->NumFrames;
		frame1 = frame1 % seq->NumFrames;
		vertexOffsets[0] = (seq->StartFrame + frame0) * mesh->FrameVerts;
		vertexOffsets[1] = (seq->StartFrame + frame1) * mesh->FrameVerts;
		vertexOffsets[2] = 0;

		// Save old animation location to be able to tween from it:
		actor->LastAnimFrame.V0 = vertexOffsets[0];
		actor->LastAnimFrame.V1 = vertexOffsets[1];
		actor->LastAnimFrame.T = t0;
	}
	else // Tween from old animation
	{
		vertexOffsets[2] = seq->StartFrame * mesh->FrameVerts;

		if (actor->LastAnimFrame.T < 0.0f)
		{
			actor->LastAnimFrame.V0 = vertexOffsets[2];
			actor->LastAnimFrame.V1 = vertexOffsets[2];
			actor->LastAnimFrame.T = 0.0f;
		}

		vertexOffsets[0] = actor->LastAnimFrame.V0;
		vertexOffsets[1] = actor->LastAnimFrame.V1;

		t0 = actor->LastAnimFrame.T;
		t1 = clamp(animFrame + 1.0f, 0.0f, 1.0f);
	}

	SetupTextures(actor, mesh);
	DrawLodMeshFace(frame, actor, mesh, mesh->Faces, ObjectToWorld, color, mesh->SpecialVerts, vertexOffsets, t0, t1);
	DrawLodMeshFace(frame, actor, mesh, mesh->SpecialFaces, ObjectToWorld, color, 0, vertexOffsets, t0, t1);
}

void MeshRender::SetupTextures(UActor* actor, ULodMesh* mesh)
{
	if (textures.size() < mesh->Textures.size())
		textures.resize(mesh->Textures.size());

	envmap = nullptr;

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

		if (tex)
			envmap = tex;

		textures[i] = tex;
	}

	if (actor->Texture())
	{
		envmap = actor->Texture();
	}
	else if (actor->Region().Zone && actor->Region().Zone->EnvironmentMap())
	{
		envmap = actor->Region().Zone->EnvironmentMap();
	}
	else if (actor->Level()->EnvironmentMap())
	{
		envmap = actor->Level()->EnvironmentMap();
	}
}

void MeshRender::DrawLodMeshFace(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, const vec3& lightcolor, int baseVertexOffset, const int* vertexOffsets, float t0, float t1)
{
	auto device = engine->window->GetRenderDevice();

	uint32_t polyFlags = 0;
	switch (actor->Style())
	{
	case 2: polyFlags |= PF_Masked; break; // STY_Masked
	case 3: polyFlags |= PF_Translucent; break; // STY_Translucent
	case 4: polyFlags |= PF_Modulated; break; // STY_Modulated
	}
	if (actor->bNoSmooth()) polyFlags |= PF_NoSmooth;
	if (actor->bSelected()) polyFlags |= PF_Selected;
	if (actor->bMeshEnviroMap()) polyFlags |= PF_Environment;
	if (actor->bMeshCurvy()) polyFlags |= PF_Flat;
	if (actor->bNoSmooth()) polyFlags |= PF_NoSmooth;
	if (actor->bUnlit() || actor->Region().ZoneNumber == 0) polyFlags |= PF_Unlit;
	
	vec3 color;
	if (polyFlags & PF_Unlit)
	{
		color = vec3(clamp(actor->ScaleGlow() * 0.5f + actor->AmbientGlow() * (1.0f / 256.0f), 0.0f, 1.0f));
	}
	else
	{
		color = lightcolor;
	}

	GouraudVertex vertices[3];
	for (const MeshFace& face : faces)
	{
		if (face.MaterialIndex >= mesh->Materials.size())
			continue;

		const MeshMaterial& material = mesh->Materials[face.MaterialIndex];

		uint32_t renderflags = material.PolyFlags | polyFlags;
		UTexture* tex = (renderflags & PF_Environment) ? envmap : textures[material.TextureIndex];

		if (!tex)
			continue;

		if (tex && tex->bMasked())
			renderflags |= PF_Masked;

		FTextureInfo texinfo;
		texinfo.Texture = tex;
		texinfo.CacheID = (uint64_t)(ptrdiff_t)texinfo.Texture;
		texinfo.Format = texinfo.Texture->ActualFormat;
		texinfo.Mips = texinfo.Texture->Mipmaps.data();
		texinfo.NumMips = (int)texinfo.Texture->Mipmaps.size();
		texinfo.USize = texinfo.Texture->USize();
		texinfo.VSize = texinfo.Texture->VSize();
		if (texinfo.Texture->Palette())
			texinfo.Palette = (FColor*)texinfo.Texture->Palette()->Colors.data();

		float uscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Width : 256) * (1.0f / 255.0f);
		float vscale = (texinfo.Texture ? texinfo.Texture->Mipmaps.front().Height : 256) * (1.0f / 255.0f);

		for (int i = 0; i < 3; i++)
		{
			const MeshWedge& wedge = mesh->Wedges[face.Indices[i]];
			const vec3& v0 = mesh->Verts[(size_t)wedge.Vertex + baseVertexOffset + vertexOffsets[0]];
			const vec3& v1 = mesh->Verts[(size_t)wedge.Vertex + baseVertexOffset + vertexOffsets[1]];
			vec3 vertex = mix(v0, v1, t0);
			if (t1 != 0.0f)
			{
				const vec3& v2 = mesh->Verts[(size_t)wedge.Vertex + baseVertexOffset + vertexOffsets[2]];
				vertex = mix(vertex, v2, t1);
			}

			vertices[i].Point = (ObjectToWorld * vec4(vertex, 1.0f)).xyz();
			vertices[i].UV = { wedge.U * uscale, wedge.V * vscale };
			vertices[i].Light = color;
		}

		if (renderflags & PF_Environment)
		{
			// To do: this needs to be the smoothed normal
			vec3 n = normalize(cross(vertices[1].Point - vertices[0].Point, vertices[2].Point - vertices[0].Point));
			mat3 rotmat = mat3(frame->WorldToView * frame->ObjectToWorld);
			for (int i = 0; i < 3; i++)
			{
				vec3 v = normalize(vertices[i].Point);
				vec3 p = rotmat * reflect(v, n);
				vertices[i].UV = { (p.x + 1.0f) * 128.0f * uscale, (p.y + 1.0f) * 128.0f * vscale };
			}
		}

		device->DrawGouraudPolygon(frame, texinfo, vertices, 3, renderflags);
	}
}

void MeshRender::DrawSkeletalMesh(FSceneNode* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld, const vec3& color)
{
	DrawLodMesh(frame, actor, mesh, ObjectToWorld, color);
}
