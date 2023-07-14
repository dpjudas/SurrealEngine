
#include "Precomp.h"
#include "VToolbar.h"

VToolbar::VToolbar(View* parent) : VBoxView(parent)
{
	addClass("vtoolbar");
}

VToolbarButton* VToolbar::addButton(std::string img, std::string text, std::function<void()> onClick)
{
	auto button = new VToolbarButton(this);
	button->addClass("vtoolbar-button");
	button->icon->setSrc(img);
	button->text->setText(text);
	if (onClick)
	{
		button->element->addEventListener("click", [=](Event* event) { onClick(); event->stopPropagation(); });
	}
	return button;
}
