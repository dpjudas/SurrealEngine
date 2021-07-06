
#include "Precomp.h"
#include "BrushRender.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/ULevel.h"
#include "UObject/UTexture.h"
#include "Engine.h"
#include "Viewport/Viewport.h"
#include "UTRenderer.h"

void BrushRender::DrawBrush(FSceneNode* frame, UModel* brush, const vec3& location, const Rotator& rotation, float drawscale, int zoneIndex)
{
	FSceneNode brushframe = *frame;

	mat4 rotate = mat4::rotate(radians(rotation.RollDegrees()), 0.0f, 1.0f, 0.0f) * mat4::rotate(radians(rotation.PitchDegrees()), -1.0f, 0.0f, 0.0f) * mat4::rotate(radians(rotation.YawDegrees()), 0.0f, 0.0f, -1.0f);
	mat4 ObjectToWorld = mat4::translate(location) * rotate * mat4::scale(drawscale);
	brushframe.Modelview = brushframe.Modelview * ObjectToWorld;

	auto device = engine->viewport->GetRenderDevice();
	device->SetSceneNode(&brushframe);

	vec3 color = engine->renderer->light.FindLightAt(location, zoneIndex);

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(&brushframe, brush, node, 0, color);
	}

	for (const BspNode& node : brush->Nodes)
	{
		DrawNodeSurfaceGouraud(&brushframe, brush, node, 1, color);
	}

	device->SetSceneNode(frame);
}

void BrushRender::DrawNodeSurfaceGouraud(FSceneNode* frame, UModel* model, const BspNode& node, int pass, const vec3& color)
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
		texture.CacheID = (uint64_t)(ptrdiff_t)surface.Material;
		texture.bRealtimeChanged = surface.Material->TextureModified;
		texture.UScale = surface.Material->DrawScale();
		texture.VScale = surface.Material->DrawScale();
		texture.Pan.x = -(float)surface.PanU;
		texture.Pan.y = -(float)surface.PanV;
		texture.Texture = surface.Material;

		if (surface.Material->TextureModified)
			surface.Material->TextureModified = false;

		if (surface.PolyFlags & PF_AutoUPan) texture.Pan.x += engine->renderer->AutoUVTime * 100.0f;
		if (surface.PolyFlags & PF_AutoVPan) texture.Pan.y += engine->renderer->AutoUVTime * 100.0f;
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

	engine->viewport->GetRenderDevice()->DrawGouraudPolygon(frame, &texture, vertices.data(), (int)vertices.size(), surface.PolyFlags);
}
