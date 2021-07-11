
#include "Precomp.h"
#include "MenubarModal.h"
#include "Menubar.h"
#include "Menu.h"

MenubarModal::MenubarModal(Menubar* menubar, MenubarItem* openitem) : HBoxView(nullptr), menubar(menubar)
{
	addClass("menubarmodal");
	spacer = new View(this);
	spacer->addClass("menubarmodal-spacer");

	for (MenubarItem* src : menubar->menuItems)
	{
		auto item = new MenubarModalItem(this, src->alignRight);
		if (!item->alignRight)
			item->moveBefore(spacer);
		item->addClass("menubarmodal-item");
		item->setText(src->getText());
		item->setOpenCallback(src->getOpenCallback());
		if (src == openitem)
		{
			firstOpenMenuItem = item;
		}
		items.push_back(item);
	}

	updateLayout();
}

void MenubarModal::updateLayout()
{
	Rect box = menubar->element->getBoundingClientRect();
	element->setStyle("position", "absolute");
	element->setStyle("left", std::to_string(box.x) + "px");
	element->setStyle("top", std::to_string(box.y) + "px");
	element->setStyle("width", std::to_string(box.width) + "px");
	element->setStyle("height", std::to_string(box.height) + "px");
}

void MenubarModal::itemClicked(MenubarModalItem* item)
{
}

void MenubarModal::itemMouseEnter(MenubarModalItem* item)
{
	showMenu(item);
}

void MenubarModal::showMenu(MenubarModalItem* item)
{
	delete openMenu;
	openMenu = nullptr;

	for (auto i : items)
	{
		if (i == item)
		{
			i->open();
		}
		else
		{
			i->close();
		}
	}

	Rect box = item->element->getBoundingClientRect();
	openMenu = new Menu(this);
	openMenu->closeMenu = [=]() { closeModal(); };
	openMenu->addClass("menubarmodal-openmenu");
	if (item->alignRight)
		openMenu->setRightPosition(element->getBoundingClientRect().width - box.x - box.width, box.y + box.height - 1);
	else
		openMenu->setLeftPosition(box.x, box.y + box.height - 1);
	if (item->getOpenCallback())
	{
		item->getOpenCallback()(openMenu);
	}
}

void MenubarModal::onClose(Event* event)
{
	event->stopPropagation();
	closeModal();
}

void MenubarModal::onAttach()
{
	HBoxView::onAttach();
	parent()->element->addEventListener("click", [=](Event* event) { onClose(event); });
	if (firstOpenMenuItem)
	{
		showMenu(firstOpenMenuItem);
		firstOpenMenuItem = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////

MenubarModalItem::MenubarModalItem(MenubarModal* menubar, bool alignRight) : TextLabel(menubar), menubar(menubar), alignRight(alignRight)
{
	addClass("menubarmodalitem");
	element->addEventListener("click", [=](Event* event) { onClick(event); });
	element->addEventListener("mouseenter", [=](Event* event) { onMouseEnter(event); });
}

void MenubarModalItem::open()
{
	addClass("menubarmodalitem-open");
}

void MenubarModalItem::close()
{
	removeClass("menubarmodalitem-open");
}

void MenubarModalItem::onClick(Event* event)
{
	event->stopPropagation();
	menubar->itemClicked(this);
}

void MenubarModalItem::onMouseEnter(Event* event)
{
	menubar->itemMouseEnter(this);
}
