
#include "Precomp.h"
#include "EditorViewport.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"

EditorViewport::EditorViewport(Widget* parent) : Widget(parent)
{
}

EditorViewport::~EditorViewport()
{
}

void EditorViewport::OnPaint(Canvas* canvas)
{
	if (!engine)
		return;

	Point topLeft = MapTo(Window(), Point(0.0, 0.0));

	if (!engine->render)
		engine->render = std::make_unique<RenderSubsystem>(Window()->GetRenderDevice());

	engine->CameraLocation = vec3(0.0f, 0.0f, 600.0f);
	engine->CameraRotation = Rotator(0, 0, 0);
	engine->ViewportX = (int)std::round(topLeft.x);
	engine->ViewportY = (int)std::round(topLeft.y);
	engine->ViewportWidth = (int)std::round(GetWidth());
	engine->ViewportHeight = (int)std::round(GetHeight());
	engine->render->DrawEditorViewport();
}
