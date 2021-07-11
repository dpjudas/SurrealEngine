
#include "Precomp.h"
#include "DialogButtonbar.h"
#include "DialogButton.h"

DialogButtonbar::DialogButtonbar(View* parent) : HBoxView(parent)
{
	addClass("dialogbuttonbar");
	spacer = new View(this);
	spacer->addClass("dialogbuttonbar-spacer");
}

void DialogButtonbar::setFlat(bool value)
{
	if (value)
		addClass("flat");
	else
		removeClass("flat");
}

DialogButton* DialogButtonbar::addButton(std::string icon, std::string text, std::function<void()> callback, bool leftAlign)
{
	DialogButton* button = new DialogButton(this);
	if (leftAlign)
		button->moveBefore(spacer);
	button->addClass("dialogbuttonbar-button");
	if (!icon.empty())
		button->setIcon(icon);
	button->setText(text);
	button->element->addEventListener("click", [=](Event* e) { e->stopPropagation(); callback(); });
	return button;
}

View* DialogButtonbar::addSeparator()
{
	View* sep = new View(this);
	sep->addClass("dialogbuttonbar-sep");
	return sep;
}
