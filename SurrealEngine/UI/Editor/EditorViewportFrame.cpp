
#include "Precomp.h"
#include "EditorViewportFrame.h"
#include "Editor2DViewport.h"
#include "Editor3DViewport.h"
#include "UI/Core/Colorf.h"

EditorViewportFrame::EditorViewportFrame(Widget* parent) : Widget(parent)
{
}

EditorViewportFrame::~EditorViewportFrame()
{
}

void EditorViewportFrame::SetTopMode()
{
	delete Viewport; Viewport = nullptr;
	Coords coords;
	coords.Origin = vec3(0.0f);
	coords.XAxis = vec3(1.0f, 0.0f, 0.0f);
	coords.YAxis = vec3(0.0f, 1.0f, 0.0f);
	coords.ZAxis = vec3(0.0f, 0.0f, 1.0f);
	Viewport = new Editor2DViewport(coords, this);
	OnGeometryChanged();
}

void EditorViewportFrame::SetFrontMode()
{
	delete Viewport; Viewport = nullptr;
	Coords coords;
	coords.Origin = vec3(0.0f);
	coords.XAxis = vec3(1.0f, 0.0f, 0.0f);
	coords.YAxis = vec3(0.0f, 0.0f, -1.0f);
	coords.ZAxis = vec3(0.0f, 1.0f, 0.0f);
	Viewport = new Editor2DViewport(coords, this);
	OnGeometryChanged();
}

void EditorViewportFrame::SetSideMode()
{
	delete Viewport; Viewport = nullptr;
	Coords coords;
	coords.Origin = vec3(0.0f);
	coords.XAxis = vec3(0.0f, 1.0f, 0.0f);
	coords.YAxis = vec3(0.0f, 0.0f, -1.0f);
	coords.ZAxis = vec3(1.0f, 0.0f, 0.0f);
	Viewport = new Editor2DViewport(coords, this);
	OnGeometryChanged();
}

void EditorViewportFrame::Set3DMode()
{
	delete Viewport; Viewport = nullptr;
	Viewport = new Editor3DViewport(this);
	OnGeometryChanged();
}

void EditorViewportFrame::OnPaint(Canvas* canvas)
{
	Colorf bordercolor(200 / 255.0f, 200 / 255.0f, 200 / 255.0f);

	double w = GetWidth();
	double h = GetHeight();
	canvas->fillRect(Rect::xywh(0.0, 0.0, w, 1.0), bordercolor);
	canvas->fillRect(Rect::xywh(0.0, h - 1.0, w, 1.0), bordercolor);
	canvas->fillRect(Rect::xywh(0.0, 0.0, 1.0, h), bordercolor);
	canvas->fillRect(Rect::xywh(w - 1.0, 0.0, 1.0, h), bordercolor);
}

void EditorViewportFrame::OnGeometryChanged()
{
	if (Viewport)
		Viewport->SetFrameGeometry(Rect::xywh(1.0, 1.0, GetWidth() - 2.0, GetHeight() - 2.0));
}
