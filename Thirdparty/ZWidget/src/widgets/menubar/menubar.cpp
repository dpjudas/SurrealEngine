
#include "widgets/menubar/menubar.h"
#include "core/colorf.h"

Menubar::Menubar(Widget* parent) : Widget(parent)
{
	SetStyleClass("menubar");
}

Menubar::~Menubar()
{
}

MenubarItem* Menubar::AddItem(std::string text, std::function<void(Menu* menu)> onOpen, bool alignRight)
{
	auto item = new MenubarItem(this, text, alignRight);
	item->SetOpenCallback(std::move(onOpen));
	menuItems.push_back(item);
	OnGeometryChanged();
	return item;
}

void Menubar::ShowMenu(MenubarItem* item)
{
	CloseMenu();
	if (item->GetOpenCallback())
	{
		openMenu = new Menu(this);
		openMenu->onCloseMenu = [=]() { CloseMenu(); };
		item->GetOpenCallback()(openMenu);
		if (item->AlignRight)
			openMenu->SetRightPosition(item->MapToGlobal(Point(item->GetWidth(), item->GetHeight())));
		else
			openMenu->SetLeftPosition(item->MapToGlobal(Point(0.0, item->GetHeight())));
		openMenu->Show();
	}
}

void Menubar::CloseMenu()
{
	//delete openMenu;
	//openMenu = nullptr;
}

void Menubar::OnGeometryChanged()
{
	double w = GetWidth();
	double h = GetHeight();
	double left = 0.0;
	double right = w;
	for (MenubarItem* item : menuItems)
	{
		double itemwidth = item->GetPreferredWidth();
		itemwidth += 16.0;
		if (!item->AlignRight)
		{
			item->SetFrameGeometry(left, 0.0, itemwidth, h);
			left += itemwidth;
		}
		else
		{
			right -= itemwidth;
			item->SetFrameGeometry(right, 0.0, itemwidth, h);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

MenubarItem::MenubarItem(Menubar* menubar, std::string text, bool alignRight) : Widget(menubar), menubar(menubar), text(text), AlignRight(alignRight)
{
	SetStyleClass("menubaritem");
}

bool MenubarItem::OnMouseDown(const Point& pos, InputKey key)
{
	menubar->ShowMenu(this);
	return true;
}

bool MenubarItem::OnMouseUp(const Point& pos, InputKey key)
{
	return true;
}

void MenubarItem::OnMouseMove(const Point& pos)
{
	if (GetStyleState().empty())
	{
		SetStyleState("hover");
	}
}

void MenubarItem::OnMouseLeave()
{
	SetStyleState("");
}

double MenubarItem::GetPreferredWidth() const
{
	Canvas* canvas = GetCanvas();
	return canvas->measureText(text).width;
}

void MenubarItem::OnPaint(Canvas* canvas)
{
	double x = (GetWidth() - canvas->measureText(text).width) * 0.5;
	canvas->drawText(Point(x, 21.0), GetStyleColor("color"), text);
}

/////////////////////////////////////////////////////////////////////////////

Menu::Menu(Widget* parent) : Widget(parent, WidgetType::Popup)
{
	SetStyleClass("menu");
}

void Menu::SetLeftPosition(const Point& pos)
{
	SetFrameGeometry(Rect::xywh(pos.x, pos.y, GetPreferredWidth() + GetNoncontentLeft() + GetNoncontentRight(), GetPreferredHeight() + GetNoncontentTop() + GetNoncontentBottom()));
}

void Menu::SetRightPosition(const Point& pos)
{
	SetFrameGeometry(Rect::xywh(pos.x - GetWidth() - GetNoncontentLeft() - GetNoncontentRight(), pos.y, GetWidth() + GetNoncontentLeft() + GetNoncontentRight(), GetHeight() + GetNoncontentTop() + GetNoncontentBottom()));
}

MenuItem* Menu::AddItem(std::shared_ptr<Image> icon, std::string text, std::function<void()> onClick)
{
	auto item = new MenuItem(this);
	if (icon)
		item->icon->SetImage(icon);
	item->text->SetText(text);
	/*
	item->element->addEventListener("click", [=](Event* event)
		{
			event->stopPropagation();
			if (onCloseMenu)
				onCloseMenu();
			if (onClick)
				onClick();
		});
	*/
	return item;
}

MenuItemSeparator* Menu::AddSeparator()
{
	auto sep = new MenuItemSeparator(this);
	return sep;
}

double Menu::GetPreferredWidth() const
{
	return 200.0;
}

double Menu::GetPreferredHeight() const
{
	double h = 0.0;
	for (Widget* item = FirstChild(); item != nullptr; item = item->NextSibling())
	{
		h += 20.0;
	}
	return h;
}

void Menu::OnGeometryChanged()
{
	double w = GetWidth();
	double h = GetHeight();
	double y = 0.0;
	for (Widget* item = FirstChild(); item != nullptr; item = item->NextSibling())
	{
		item->SetFrameGeometry(Rect::xywh(0.0, y, w, 20.0));
		y += 20.0;
	}
}

/////////////////////////////////////////////////////////////////////////////

MenuItem::MenuItem(Widget* parent) : Widget(parent)
{
	SetStyleClass("menuitem");
	icon = new ImageBox(this);
	text = new TextLabel(this);
}

void MenuItem::OnMouseMove(const Point& pos)
{
	if (GetStyleState().empty())
	{
		SetStyleState("hover");
	}
}

void MenuItem::OnMouseLeave()
{
	SetStyleState("");
}

void MenuItem::OnGeometryChanged()
{
	double iconwidth = icon->GetPreferredWidth();
	double iconheight = icon->GetPreferredHeight();
	double w = GetWidth();
	double h = GetHeight();
	icon->SetFrameGeometry(Rect::xywh(0.0, (h - iconheight) * 0.5, iconwidth, iconheight));
	text->SetFrameGeometry(Rect::xywh(iconwidth, 0.0, w - iconwidth, h));
}

/////////////////////////////////////////////////////////////////////////////

MenuItemSeparator::MenuItemSeparator(Widget* parent) : Widget(parent)
{
	SetStyleClass("menuitemseparator");
}
