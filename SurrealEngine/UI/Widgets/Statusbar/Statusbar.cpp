
#include "Precomp.h"
#include "Statusbar.h"
#include "UI/Core/Colorf.h"

Statusbar::Statusbar(Widget* parent) : Widget(parent)
{
}

Statusbar::~Statusbar()
{
}

void Statusbar::OnPaint(Canvas* canvas)
{
	canvas->drawText(Point(16.0, 21.0), Colorf(0.0f, 0.0f, 0.0f), "Ready");
}
