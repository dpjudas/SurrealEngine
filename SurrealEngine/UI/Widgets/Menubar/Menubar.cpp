
#include "Precomp.h"
#include "Menubar.h"
#include "UI/Core/Colorf.h"

Menubar::Menubar(Widget* parent) : Widget(parent)
{
}

Menubar::~Menubar()
{
}

void Menubar::OnPaint(Canvas* canvas)
{
	canvas->drawText(Point(16.0, 21.0), Colorf(0.0f, 0.0f, 0.0f), "File      Edit      View      Tools      Window     Help");
}
