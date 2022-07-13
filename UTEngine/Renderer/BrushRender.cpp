
#include "Precomp.h"
#include "BrushRender.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"
#include "UObject/UActor.h"
#include "Engine.h"
#include "Window/Window.h"
#include "UTRenderer.h"

void BrushRender::DrawBrush(FSceneNode* frame, UActor* actor)
{
	UModel* brush = actor->Brush();
	const vec3& location = actor->Location();
	const Rotator& rotation = actor->Rotation();
	float drawscale = actor->DrawScale();
	const vec3& color = actor->light;
	FSceneNode brushframe = *frame;

	mat4 rotate = mat4::rotate(radians(rotation.RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(rotation.PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(rotation.YawDegrees()), 0.0f, 0.0f, -1.0f);
	brushframe.ObjectToWorld = mat4::translate(location) * rotate * mat4::scale(drawscale);

	auto device = engine->window->GetRenderDevice();
	device->SetSceneNode(&brushframe);

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(brush, node, 0, color);
	}

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(brush, node, 1, color);
	}

	device->SetSceneNode(frame);
}

void BrushRender::DrawNodeSurfaceGouraud(UModel* model, const BspNode& node, int pass, const vec3& color)
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

		if (tex->TextureModified)
			tex->TextureModified = false;

		if (surface.PolyFlags & PF_AutoUPan) texture.Pan.x += engine->renderer->AutoUV;
		if (surface.PolyFlags & PF_AutoVPan) texture.Pan.y += engine->renderer->AutoUV;
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
		gv.Light = color;
		gv.UV = { dot(UVec, gv.Point) - UPan, dot(VVec, gv.Point) - VPan };
		vertices.push_back(gv);
	}

	engine->window->GetRenderDevice()->DrawGouraudPolygon(texture.Texture ? &texture : nullptr, vertices.data(), (int)vertices.size(), surface.PolyFlags);
}
