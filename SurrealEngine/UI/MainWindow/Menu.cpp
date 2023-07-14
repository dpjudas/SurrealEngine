
#include "Precomp.h"
#include "Menu.h"
#include "UI/Controls/ImageBox/ImageBox.h"
#include "UI/Controls/TextLabel/TextLabel.h"

Menu::Menu(View* parent) : VBoxView(parent)
{
	addClass("menu");
}

void Menu::setLeftPosition(double x, double y)
{
	element->setStyle("left", std::to_string(x) + "px");
	element->setStyle("top", std::to_string(y) + "px");
}

void Menu::setRightPosition(double x, double y)
{
	element->setStyle("right", std::to_string(x) + "px");
	element->setStyle("top", std::to_string(y) + "px");
}

MenuItem* Menu::addItem(std::string icon, std::string text, std::function<void()> onClick)
{
	auto item = new MenuItem(this);
	item->addClass("menu-item");
	if (!icon.empty())
		item->icon->setSrc(icon);
	item->text->setText(text);
	item->element->addEventListener("click", [=](Event* event)
	{
		event->stopPropagation();
		if (closeMenu)
			closeMenu();
		if (onClick)
			onClick();
	});
	return item;
}

MenuItemSeparator* Menu::addSeparator()
{
	auto sep = new MenuItemSeparator(this);
	sep->addClass("menu-sep");
	return sep;
}

/////////////////////////////////////////////////////////////////////////////

MenuItem::MenuItem(View* parent) : HBoxView(parent)
{
	icon = new ImageBox(this);
	text = new TextLabel(this);

	addClass("menuitem");
	icon->addClass("menuitem-icon");
	text->addClass("menuitem-text");

	element->addEventListener("mouseenter", [=](Event* event) { onMouseEnter(event); });
	element->addEventListener("mouseleave", [=](Event* event) { onMouseLeave(event); });
}

void MenuItem::onMouseEnter(Event* event)
{
	element->setStyle("background", "rgb(200,200,200)");
}

void MenuItem::onMouseLeave(Event* event)
{
	element->setStyle("background", "none");
}
