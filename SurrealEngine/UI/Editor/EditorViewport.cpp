
#include "Precomp.h"
#include "EditorViewport.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"
#include "UI/Core/Colorf.h"

EditorViewport::EditorViewport(Widget* parent) : Widget(parent)
{
}

EditorViewport::~EditorViewport()
{
}

void EditorViewport::OnPaint(Canvas* canvas)
{
	canvas->fillRect(Rect::xywh(0.0, 0.0, GetWidth(), GetHeight()), Colorf(40 / 255.0f, 40 / 255.0f, 40 / 255.0f));

	if (!engine)
		return;

	Point topLeft = MapTo(Window(), Point(0.0, 0.0));

	canvas->begin3d();

	if (!engine->render)
		engine->render = std::make_unique<RenderSubsystem>(Window()->GetRenderDevice());

	engine->CameraLocation = Location;
	engine->CameraRotation = Rotation;
	engine->ViewportX = (int)std::round(topLeft.x);
	engine->ViewportY = (int)std::round(topLeft.y);
	engine->ViewportWidth = (int)std::round(GetWidth());
	engine->ViewportHeight = (int)std::round(GetHeight());
	engine->render->DrawEditorViewport();

	canvas->end3d();
}

void EditorViewport::OnMouseMove(const Point& pos)
{
}

void EditorViewport::OnMouseDown(const Point& pos, int key)
{
	SetFocus();
	if (key == IK_RightMouse)
	{
		LockCursor();
		IsLocked = true;
	}
}

void EditorViewport::OnMouseDoubleclick(const Point& pos, int key)
{
}

void EditorViewport::OnMouseUp(const Point& pos, int key)
{
	if (key == IK_RightMouse)
	{
		UnlockCursor();
		IsLocked = false;
	}
}

void EditorViewport::OnKeyDown(int key)
{
}

void EditorViewport::OnKeyUp(int key)
{

}

void EditorViewport::OnRawMouseMove(int dx, int dy)
{
	if (IsLocked)
	{
		Rotation.Yaw += dx * 10;
		Rotation.Pitch = clamp(Rotation.Pitch + dy * 10, -90 * 65536 / 360, 90 * 65536 / 360);
		Update();
	}
}
