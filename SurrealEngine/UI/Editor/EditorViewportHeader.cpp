
#include "Precomp.h"
#include "EditorViewportHeader.h"
#include <zwidget/core/canvas.h>
#include <zwidget/core/colorf.h>

EditorViewportHeader::EditorViewportHeader(Widget* parent) : Widget(parent)
{
}

void EditorViewportHeader::SetTitle(const std::string& title)
{
	if (Title != title)
	{
		Title = title;
		Update();
	}
}

void EditorViewportHeader::OnPaint(Canvas* canvas)
{
	canvas->drawText({ 7.0, 17.0 }, Colorf::fromRgba8(226, 223, 219), Title);
}
