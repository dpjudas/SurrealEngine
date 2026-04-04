
#include "Precomp.h"
#include "EditorViewportHeader.h"
#include <surrealwidgets/core/canvas.h>
#include <surrealwidgets/core/colorf.h>

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
	canvas->drawText(GetFont(), { 7.0, 17.0 }, Title, Colorf::fromRgba8(226, 223, 219));
}
