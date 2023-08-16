
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "Window/Window.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

void RenderSubsystem::DrawBrush(FSceneNode* frame, UActor* actor)
{
	UModel* brush = actor->Brush();
	const vec3& location = actor->Location();
	float drawscale = actor->DrawScale();
	FSceneNode brushframe = *frame;

	UpdateActorLightList(actor);

	brushframe.ObjectToWorld = mat4::translate(location) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::scale(drawscale);

	Device->SetSceneNode(&brushframe);

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(frame, brush, node, 0, actor);
	}

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(frame, brush, node, 1, actor);
	}

	Device->SetSceneNode(frame);
}

void RenderSubsystem::DrawNodeSurfaceGouraud(FSceneNode* frame, UModel* model, const BspNode& node, int pass, UActor* actor)
{
	if (node.NumVertices <= 0 || node.Surf < 0)
		return;

	BspSurface& surface = model->Surfaces[node.Surf];

	if (surface.PolyFlags & (PF_Invisible | PF_FakeBackdrop))
		return;

	bool opaqueSurface = (surface.PolyFlags & PF_NoOcclude) == 0;
	if ((pass == 0 && !opaqueSurface) || (pass == 1 && opaqueSurface))
		return;

	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	if (!surface.Material)
		return;

	FTextureInfo texture;
	if (surface.Material)
	{
		UTexture* tex = surface.Material->GetAnimTexture();

		texture.CacheID = (uint64_t)(ptrdiff_t)tex;
		texture.bRealtimeChanged = tex->TextureModified;
		texture.UScale = tex->DrawScale();
		texture.VScale = tex->DrawScale();
		texture.Pan.x = -(float)surface.PanU;
		texture.Pan.y = -(float)surface.PanV;
		texture.Texture = tex;
		texture.Format = texture.Texture->ActualFormat;
		texture.Mips = tex->Mipmaps.data();
		texture.NumMips = (int)tex->Mipmaps.size();
		texture.USize = tex->USize();
		texture.VSize = tex->VSize();
		if (tex->Palette())
			texture.Palette = (FColor*)tex->Palette()->Colors.data();

		if (tex->TextureModified)
			tex->TextureModified = false;

		if (surface.PolyFlags & PF_AutoUPan) texture.Pan.x -= AutoUV;
		if (surface.PolyFlags & PF_AutoVPan) texture.Pan.y -= AutoUV;
	}

	BspVert* v = &model->Vertices[node.VertPool];

	float UDot = dot(UVec, Base);
	float VDot = dot(VVec, Base);
	float UPan = UDot + texture.Pan.x;
	float VPan = VDot + texture.Pan.y;

	std::vector<GouraudVertex> vertices;
	for (int j = 0; j < node.NumVertices; j++)
	{
		GouraudVertex gv;
		gv.Point = model->Points[v[j].Vertex];
		gv.Light = GetVertexLight(actor, gv.Point, model->Vectors[surface.vNormal], surface.PolyFlags & PF_Unlit);
		gv.UV = { dot(UVec, gv.Point) - UPan, dot(VVec, gv.Point) - VPan };
		vertices.push_back(gv);
	}

	Device->DrawGouraudPolygon(frame, texture, vertices.data(), (int)vertices.size(), surface.PolyFlags);
}
