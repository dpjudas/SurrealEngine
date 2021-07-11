
#include "Precomp.h"
#include "AppMenu.h"
#include "DialogHeader.h"
#include "DialogHeaderButton.h"
#include "Dialog.h"
#include "UI/Controls/ImageBox/ImageBox.h"

AppMenu::AppMenu(View* p) : VBoxView(p)
{
	addClass("appmenu");

	menuButton = new ImageBox(this);
	menuButton->addClass("appmenu-menubutton");

	title = new TextLabel(this);
	title->addClass("appmenu-title");
	title->setText("Apps");

	itemList = new View(this);
	itemList->addClass("appmenu-itemlist");

	element->addEventListener("click", [this](Event* event) { event->stopPropagation(); });
	menuButton->element->addEventListener("click", [this](Event* event) { event->stopPropagation(); closeModal(); });
}

void AppMenu::setIcon(std::string src)
{
	menuButton->setSrc(src);
}

AppMenuItem* AppMenu::addItem(std::string icon, std::string text, std::function<void()> onClick)
{
	auto item = new AppMenuItem(itemList);
	if (!icon.empty())
		item->icon->setSrc(icon);
	item->text->setText(text);
	item->element->addEventListener("click", [=](Event* event)
	{
		event->stopPropagation();
		closeModal();
		if (onClick)
			onClick();
	});
	return item;
}

void AppMenu::onAttach()
{
	VBoxView::onAttach();
	parent()->element->addEventListener("click", [=](Event* event) { event->stopPropagation(); closeModal(); });
}
