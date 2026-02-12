
#include "Precomp.h"
#include "Editor3DViewport.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include <zwidget/core/colorf.h>

Editor3DViewport::Editor3DViewport(Widget* parent) : EditorViewport(parent)
{
}

Editor3DViewport::~Editor3DViewport()
{
}

void Editor3DViewport::UpdateMovement()
{
	float timeElapsed = CalcTimeElapsed();

	vec3 moveVec(0.0f);
	float speed = 500.0f;

	if (moveForwards)
		moveVec.x += speed;
	if (moveBackwards)
		moveVec.x -= speed;
	if (moveUp)
		moveVec.z += speed;
	if (moveDown)
		moveVec.z -= speed;
	if (moveRight)
		moveVec.y += speed;
	if (moveLeft)
		moveVec.y -= speed;

	if (LastLocation != Location || LastRotation != Rotation || moveVec != vec3(0.0f))
	{
		moveVec *= timeElapsed;
		MoveCamera(moveVec.x, moveVec.y, moveVec.z);

		LastLocation = Location;
		LastRotation = Rotation;
		Update();
	}
	else
	{
		lastTime = 0;
	}
}

void Editor3DViewport::OnPaint(Canvas* canvas)
{
	UpdateMovement();

	canvas->fillRect(Rect::xywh(0.0, 0.0, GetWidth(), GetHeight()), Colorf::fromRgba8(0, 0, 0));

	if (!engine)
		return;

	Point topLeft = MapTo(Window(), Point(0.0, 0.0));

	canvas->begin3d();

	if (!engine->render)
		engine->render = std::make_unique<RenderSubsystem>(static_cast<RenderDeviceCanvas*>(canvas)->GetRenderDevice());

	engine->CameraLocation = Location;
	engine->CameraRotation = Rotation;
	engine->ViewportX = (int)std::round(topLeft.x * GetDpiScale());
	engine->ViewportY = (int)std::round(topLeft.y * GetDpiScale());
	engine->ViewportWidth = (int)std::round(GetWidth() * GetDpiScale());
	engine->ViewportHeight = (int)std::round(GetHeight() * GetDpiScale());
	engine->render->DrawEditorViewport();

	canvas->end3d();
}

void Editor3DViewport::OnMouseMove(const Point& pos)
{
}

bool Editor3DViewport::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();
	if (key == InputKey::RightMouse)
	{
		LockCursor();
		MouseIsPanning = true;
	}
	else if (key == InputKey::MiddleMouse)
	{
		LockCursor();
		MouseIsMoving = true;
	}
	return true;
}

bool Editor3DViewport::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	return true;
}

bool Editor3DViewport::OnMouseUp(const Point& pos, InputKey key)
{
	if (key == InputKey::RightMouse && MouseIsPanning)
	{
		UnlockCursor();
		MouseIsPanning = false;
	}
	else if (key == InputKey::MiddleMouse && MouseIsMoving)
	{
		UnlockCursor();
		MouseIsMoving = false;
	}
	return true;
}

void Editor3DViewport::OnKeyDown(InputKey key)
{
	if (key == InputKey::W)
	{
		moveForwards = true;
		Update();
	}
	else if (key == InputKey::S)
	{
		moveBackwards = true;
		Update();
	}
	else if (key == InputKey::A)
	{
		moveLeft = true;
		Update();
	}
	else if (key == InputKey::D)
	{
		moveRight = true;
		Update();
	}
	else if (key == InputKey::R)
	{
		moveUp = true;
		Update();
	}
	else if (key == InputKey::F)
	{
		moveDown = true;
		Update();
	}
}

void Editor3DViewport::OnKeyUp(InputKey key)
{
	if (key == InputKey::W)
	{
		moveForwards = false;
		Update();
	}
	else if (key == InputKey::S)
	{
		moveBackwards = false;
		Update();
	}
	else if (key == InputKey::A)
	{
		moveLeft = false;
		Update();
	}
	else if (key == InputKey::D)
	{
		moveRight = false;
		Update();
	}
	else if (key == InputKey::R)
	{
		moveUp = false;
		Update();
	}
	else if (key == InputKey::F)
	{
		moveDown = false;
		Update();
	}
}

void Editor3DViewport::OnRawMouseMove(int dx, int dy)
{
	if (MouseIsPanning)
	{
		Rotation.Yaw += dx * 15;
		Rotation.Pitch = clamp(Rotation.Pitch - dy * 15, -90 * 65536 / 360, 90 * 65536 / 360);
		Update();
	}
	else if (MouseIsMoving)
	{
		MoveCamera((float)-dy, (float)dx, 0.0f);
		Update();
	}
}

void Editor3DViewport::MoveCamera(float x, float y, float z)
{
	vec3 axisX, axisY, axisZ;
	Coords::Rotation(Rotation).GetAxes(axisX, axisY, axisZ);
	Location += axisX * x;
	Location += axisY * y;
	Location += axisZ * z;
}

float Editor3DViewport::CalcTimeElapsed()
{
	using namespace std::chrono;

	uint64_t currentTime = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
	if (lastTime == 0)
		lastTime = currentTime;

	uint64_t deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	return clamp(deltaTime / 1'000'000.0f, 0.0f, 1.0f);
}
