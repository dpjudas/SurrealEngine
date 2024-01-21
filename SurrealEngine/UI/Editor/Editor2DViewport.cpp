
#include "Precomp.h"
#include "Editor2DViewport.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include <zwidget/core/colorf.h>

Editor2DViewport::Editor2DViewport(const Coords& coords, Widget* parent) : EditorViewport(parent), ViewCoords(coords)
{
}

Editor2DViewport::~Editor2DViewport()
{
}

void Editor2DViewport::OnPaint(Canvas* canvas)
{
	Colorf background(160 / 255.0f, 160 / 255.0f, 160 / 255.0f);
	canvas->fillRect(Rect::xywh(0.0, 0.0, GetWidth(), GetHeight()), background);
	DrawGrid(canvas);
	if (engine && engine->Level)
	{
		//DrawLevel(canvas);
	}
}

void Editor2DViewport::DrawLevel(Canvas* canvas)
{
	DrawNode(canvas, &engine->Level->Model->Nodes[0]);
}

void Editor2DViewport::DrawNode(Canvas* canvas, BspNode* node)
{
	// Draw surfaces on this plane
	BspNode* polynode = node;
	while (true)
	{
		DrawNodeSurface(canvas, polynode);

		if (polynode->Plane < 0) break;
		polynode = &engine->Level->Model->Nodes[polynode->Plane];
	}

	if (node->Front >= 0)
	{
		DrawNode(canvas, &engine->Level->Model->Nodes[node->Front]);
	}
	if (node->Back >= 0)
	{
		DrawNode(canvas, &engine->Level->Model->Nodes[node->Back]);
	}
}

void Editor2DViewport::DrawNodeSurface(Canvas* canvas, BspNode* node)
{
	UModel* model = engine->Level->Model;
	const BspSurface& surface = model->Surfaces[node->Surf];

	uint32_t PolyFlags = surface.PolyFlags;
	const vec3& UVec = model->Vectors[surface.vTextureU];
	const vec3& VVec = model->Vectors[surface.vTextureV];
	const vec3& Base = model->Points[surface.pBase];

	BspVert* v = &model->Vertices[node->VertPool];

	Colorf linecolor(80 / 255.0f, 80 / 255.0f, 220 / 255.0f);
	Point center(GetWidth() * 0.5, GetHeight() * 0.5);

	int numverts = node->NumVertices;
	for (int j = 0; j < numverts; j++)
	{
		int k = j + 1;
		if (k == numverts)
			k = 0;

		const vec3 p0 = ViewCoords * model->Points[v[j].Vertex];
		const vec3 p1 = ViewCoords * model->Points[v[k].Vertex];

		canvas->line(Point(p0.x - Location.x, p0.y - Location.y) * Zoom + center, Point(p1.x - Location.x, p1.y - Location.y) * Zoom + center, linecolor);
	}
}

void Editor2DViewport::DrawGrid(Canvas* canvas)
{
	Colorf linecolor(128 / 255.0f, 128 / 255.0f, 128 / 255.0f);
	Point center(GetWidth() * 0.5, GetHeight() * 0.5);

	for (double y = -32767.0; y < 32767.0; y += 1000.0)
	{
		canvas->line(Point(-32767.0 - Location.x, y - Location.y) * Zoom + center, Point(32767.0 - Location.x, y - Location.y) * Zoom + center, linecolor);
	}
	for (double x = -32767.0; x < 32767.0; x += 1000.0)
	{
		canvas->line(Point(x - Location.x, -32767.0 - Location.y) * Zoom + center, Point(x - Location.x, 32767.0 - Location.y) * Zoom + center, linecolor);
	}
}

void Editor2DViewport::OnMouseMove(const Point& pos)
{
}

bool Editor2DViewport::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();
	if (key == InputKey::RightMouse)
	{
		LockCursor();
		MouseIsMoving = true;
	}
	return true;
}

bool Editor2DViewport::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	return true;
}

bool Editor2DViewport::OnMouseUp(const Point& pos, InputKey key)
{
	if (key == InputKey::RightMouse && MouseIsMoving)
	{
		UnlockCursor();
		MouseIsMoving = false;
	}
	return true;
}

void Editor2DViewport::OnKeyDown(InputKey key)
{
	if (key == InputKey::A)
	{
		MoveCamera(-100.0f, 0.0f);
	}
	else if (key == InputKey::D)
	{
		MoveCamera(100.0f, 0.0f);
	}
	else if (key == InputKey::W)
	{
		MoveCamera(0.0f, -100.0f);
	}
	else if (key == InputKey::S)
	{
		MoveCamera(0.0f, 100.0f);
	}
}

void Editor2DViewport::OnKeyUp(InputKey key)
{
}

void Editor2DViewport::OnRawMouseMove(int dx, int dy)
{
	if (MouseIsMoving)
	{
		MoveCamera(-dx * 20.0f, -dy * 20.0f);
	}
}

void Editor2DViewport::MoveCamera(float x, float y)
{
	Location.x += x;
	Location.y += y;
	Update();
}
