
#include "Precomp.h"
#include "DialogHeaderButton.h"

DialogHeaderButton::DialogHeaderButton(View* parent, std::string path) : SvgBox(parent)
{
	addClass("dialogheaderbutton");
	setViewbox(0, 0, 10, 10);
	addElement("path", {{ "d", path }});
}
