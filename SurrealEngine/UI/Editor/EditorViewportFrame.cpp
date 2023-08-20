
#include "Precomp.h"
#include "EditorViewportFrame.h"
#include "EditorViewportHeader.h"
#include "Editor2DViewport.h"
#include "Editor3DViewport.h"
#include "UI/Core/Colorf.h"

EditorViewportFrame::EditorViewportFrame(Widget* parent) : Widget(parent)
{
	Header = new EditorViewportHeader(this);
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
	Header->SetTitle("Top");
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
	Header->SetTitle("Front");
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
	Header->SetTitle("Side");
	OnGeometryChanged();
}

void EditorViewportFrame::Set3DMode()
{
	delete Viewport; Viewport = nullptr;
	Viewport = new Editor3DViewport(this);
	Header->SetTitle("3D");
	OnGeometryChanged();
}

void EditorViewportFrame::OnPaint(Canvas* canvas)
{
	Colorf bordercolor(200 / 255.0f, 200 / 255.0f, 200 / 255.0f);
	double headerHeight = 24.0;

	double w = GetWidth();
	double h = GetHeight();
	canvas->fillRect(Rect::xywh(0.0, headerHeight, w, 1.0), bordercolor);
	canvas->fillRect(Rect::xywh(0.0, h - 1.0, w, 1.0), bordercolor);
	canvas->fillRect(Rect::xywh(0.0, headerHeight, 1.0, h - headerHeight), bordercolor);
	canvas->fillRect(Rect::xywh(w - 1.0, headerHeight, 1.0, h - headerHeight), bordercolor);
}

void EditorViewportFrame::OnGeometryChanged()
{
	double headerHeight = 24.0;
	Header->SetFrameGeometry(Rect::xywh(1.0, 0.0, GetWidth() - 2.0, headerHeight));
	if (Viewport)
		Viewport->SetFrameGeometry(Rect::xywh(1.0, 1.0 + headerHeight, GetWidth() - 2.0, GetHeight() - headerHeight - 2.0));
}
