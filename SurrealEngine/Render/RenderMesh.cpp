
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Engine.h"

void RenderSubsystem::DrawMesh(FSceneNode* frame, UActor* actor, bool wireframe)
{
	UMesh* mesh = actor->Mesh();
	if (!mesh)
		return;

	UpdateActorLightList(actor);

	mat4 objectToWorld = mat4::translate(actor->Location() + actor->PrePivot()) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::scale(actor->DrawScale());
	mat4 meshToObject = Coords::Rotation(mesh->RotOrigin).ToMatrix() * mat4::scale(mesh->Scale) * mat4::translate(-mesh->Origin);
	mat4 meshToWorld = objectToWorld * meshToObject;

	if (dynamic_cast<USkeletalMesh*>(mesh))
		DrawSkeletalMesh(frame, actor, static_cast<USkeletalMesh*>(mesh), meshToWorld);
	else if (dynamic_cast<ULodMesh*>(mesh))
		DrawLodMesh(frame, actor, static_cast<ULodMesh*>(mesh), meshToWorld);
	else
		DrawMesh(frame, actor, mesh, meshToWorld);
}

void RenderSubsystem::DrawMesh(FSceneNode* frame, UActor* actor, UMesh* mesh, const mat4& ObjectToWorld)
{
}

void RenderSubsystem::DrawLodMesh(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const mat4& ObjectToWorld)
{
	MeshAnimSeq* seq = mesh->GetSequence(actor->AnimSequence());
	float animFrame = actor->AnimFrame() * seq->NumFrames;

	int vertexOffsets[3];
	float t0, t1;

	if (animFrame >= 0.0f)
	{
		int frame0 = (int)animFrame;
		int frame1 = frame0 + 1;
		frame0 = frame0 % seq->NumFrames;
		frame1 = frame1 % seq->NumFrames;
		t0 = animFrame - (float)frame0;
		t1 = 0.0f;
		vertexOffsets[0] = (seq->StartFrame + frame0) * mesh->FrameVerts;
		vertexOffsets[1] = (seq->StartFrame + frame1) * mesh->FrameVerts;
		vertexOffsets[2] = 0;
	}
	else // Tween from old animation
	{
		t0 = actor->TweenFromAnimFrame.T;
		t1 = clamp(animFrame + 1.0f, 0.0f, 1.0f);
		vertexOffsets[0] = actor->TweenFromAnimFrame.V0;
		vertexOffsets[1] = actor->TweenFromAnimFrame.V1;
		vertexOffsets[2] = seq->StartFrame * mesh->FrameVerts;
	}

	SetupMeshTextures(actor, mesh);
	DrawLodMeshFace(frame, actor, mesh, mesh->Faces, ObjectToWorld, mesh->SpecialVerts, vertexOffsets, t0, t1);
	DrawLodMeshFace(frame, actor, mesh, mesh->SpecialFaces, ObjectToWorld, 0, vertexOffsets, t0, t1);
}

void RenderSubsystem::SetupMeshTextures(UActor* actor, ULodMesh* mesh)
{
	if (Mesh.textures.size() < mesh->Textures.size())
		Mesh.textures.resize(mesh->Textures.size());

	Mesh.envmap = nullptr;

	for (int i = 0; i < (int)mesh->Textures.size(); i++)
	{
		// Multiskins always take precedent
		UTexture* tex = actor->GetMultiskin(i);
		if (!tex)
		{
			// Skin acts as MultiSkin[0], unless the mesh group has no texture
			if (!mesh->Textures[i] || i == 0)
				tex = actor->Skin();

			// Check mesh skin next
			if (!tex)
				tex = mesh->Textures[i];

			// Check texture
			if (!tex)
				tex = actor->Texture();

			// Get the last multiskin
			if (!tex)
			{
				for (int j = 0; j < mesh->Materials.size(); j++)
				{
					UTexture* multiskin = actor->GetMultiskin(j);
					if (multiskin)
						tex = multiskin;
				}
			}
		}

		//if (tex)
		//{
		//	tex = tex->GetAnimTexture();
		//	Mesh.envmap = tex;
		//}

		Mesh.textures[i] = tex;
	}

	if (actor->Texture())
	{
		Mesh.envmap = actor->Texture();
	}
	else if (actor->Region().Zone && actor->Region().Zone->EnvironmentMap())
	{
		Mesh.envmap = actor->Region().Zone->EnvironmentMap();
	}
	else if (actor->Level()->EnvironmentMap())
	{
		Mesh.envmap = actor->Level()->EnvironmentMap();
	}
}

void RenderSubsystem::DrawLodMeshFace(FSceneNode* frame, UActor* actor, ULodMesh* mesh, const std::vector<MeshFace>& faces, const mat4& ObjectToWorld, int baseVertexOffset, const int* vertexOffsets, float t0, float t1)
{
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

	GouraudVertex vertices[3];
	for (const MeshFace& face : faces)
	{
		if (face.MaterialIndex >= mesh->Materials.size())
			continue;

		const MeshMaterial& material = mesh->Materials[face.MaterialIndex];

		uint32_t renderflags = material.PolyFlags | polyFlags;
		UTexture* tex = (renderflags & PF_Environment) ? Mesh.envmap : Mesh.textures[material.TextureIndex];

		// skip if no texture
		if ( !tex )
			continue;

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
		}

		// To do: this needs to be the smoothed normal
		vec3 n = normalize(cross(vertices[1].Point - vertices[0].Point, vertices[2].Point - vertices[0].Point));

		if (renderflags & PF_Environment)
		{
			mat3 rotmat = mat3(frame->WorldToView * frame->ObjectToWorld);
			for (int i = 0; i < 3; i++)
			{
				vec3 v = normalize(vertices[i].Point);
				vec3 p = rotmat * reflect(v, n);
				vertices[i].UV = { (p.x + 1.0f) * 128.0f * uscale, (p.y + 1.0f) * 128.0f * vscale };
			}
		}

		for (int i = 0; i < 3; i++)
		{
			vertices[i].Light = GetVertexLight(actor, vertices[i].Point, n, !!(polyFlags & PF_Unlit));
		}

		Device->DrawGouraudPolygon(frame, texinfo, vertices, 3, renderflags);
	}
}

void RenderSubsystem::DrawSkeletalMesh(FSceneNode* frame, UActor* actor, USkeletalMesh* mesh, const mat4& ObjectToWorld)
{
	DrawLodMesh(frame, actor, mesh, ObjectToWorld);
}
