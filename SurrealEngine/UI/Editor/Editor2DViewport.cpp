
#include "Precomp.h"
#include "Editor2DViewport.h"
#include "Engine.h"
#include "UI/Core/Colorf.h"

Editor2DViewport::Editor2DViewport(Widget* parent) : EditorViewport(parent)
{
}

Editor2DViewport::~Editor2DViewport()
{
}

void Editor2DViewport::OnPaint(Canvas* canvas)
{
	Colorf background(230 / 255.0f, 230 / 255.0f, 230 / 255.0f);
	Colorf linecolor(220 / 255.0f, 220 / 255.0f, 220 / 255.0f);

	canvas->fillRect(Rect::xywh(0.0, 0.0, GetWidth(), GetHeight()), background);

	if (!engine)
		return;

	double w = GetWidth();
	double h = GetHeight();
	for (double y = 0.0; y < h; y += 10.0 * Zoom)
	{
		canvas->line(Point(0.0, y), Point(w - 1.0, y), linecolor);
	}
	for (double x = 0.0; x < w; x += 10.0 * Zoom)
	{
		canvas->line(Point(x, 0.0), Point(x, h - 1.0), linecolor);
	}
}

void Editor2DViewport::OnMouseMove(const Point& pos)
{
}

void Editor2DViewport::OnMouseDown(const Point& pos, int key)
{
	SetFocus();
	if (key == IK_RightMouse)
	{
		LockCursor();
		MouseIsMoving = true;
	}
}

void Editor2DViewport::OnMouseDoubleclick(const Point& pos, int key)
{
}

void Editor2DViewport::OnMouseUp(const Point& pos, int key)
{
	if (key == IK_RightMouse && MouseIsMoving)
	{
		UnlockCursor();
		MouseIsMoving = false;
	}
}

void Editor2DViewport::OnKeyDown(int key)
{
	if (key == IK_W)
	{
		MoveCamera(100.0f, 0.0f);
	}
	else if (key == IK_S)
	{
		MoveCamera(100.0f, 0.0f);
	}
	else if (key == IK_A)
	{
		MoveCamera(0.0f, -100.0f);
	}
	else if (key == IK_D)
	{
		MoveCamera(0.0f, 100.0f);
	}
}

void Editor2DViewport::OnKeyUp(int key)
{
}

void Editor2DViewport::OnRawMouseMove(int dx, int dy)
{
	if (MouseIsMoving)
	{
		MoveCamera((float)-dy, (float)dx);
	}
}

void Editor2DViewport::MoveCamera(float x, float y)
{
	Location.x += x;
	Location.y += y;
	Update();
}
