
#include "Precomp.h"
#include "VisiblePortal.h"
#include "VisibleFrame.h"
#include "RenderSubsystem.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Engine.h"

void VisiblePortal::DrawStencil(VisibleFrame* frame, StencilPolygonType type)
{
	UModel* model = engine->Level->Model;
	for (const VisibleNode& info : Nodes)
	{
		int numverts = info.Node->NumVertices;
		vec3* points = engine->render->GetTempVertexBuffer(numverts);

		BspVert* v = &model->Vertices[info.Node->VertPool];
		if (frame->MirrorFlag)
		{
			for (int j = 0; j < numverts; j++)
			{
				points[numverts - 1 - j] = model->Points[v[j].Vertex];
			}
		}
		else
		{
			for (int j = 0; j < numverts; j++)
			{
				points[j] = model->Points[v[j].Vertex];
			}
		}

		frame->Device->DrawStencilPolygon(type, points, numverts);
	}
}
