
#include "Precomp.h"
#include "DockHeaderButton.h"

DockHeaderButton::DockHeaderButton(View* parent, std::string svg) : ImageBox(parent)
{
	addClass("dockheaderbutton");
	setSrc("data:image/svg+xml;base64, " + svg);
}
