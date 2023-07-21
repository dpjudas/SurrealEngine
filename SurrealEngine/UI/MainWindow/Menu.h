#pragma once

#include "UI/Core/View.h"

class MenuItem;
class MenuItemSeparator;
class ImageBox;
class TextLabel;

class Menu : public VBoxView
{
public:
	Menu(View* parent);

	void setLeftPosition(double x, double y);
	void setRightPosition(double x, double y);
	MenuItem* addItem(std::string icon, std::string text, std::function<void()> onClick = {});
	MenuItemSeparator* addSeparator();

	std::function<void()> closeMenu;
};

class MenuItem : public HBoxView
{
public:
	MenuItem(View* parent);

	ImageBox* icon = nullptr;
	TextLabel* text = nullptr;

private:
	void onMouseEnter(Event* event);
	void onMouseLeave(Event* event);
};

class MenuItemSeparator : public View
{
public:
	MenuItemSeparator(View* parent) : View(parent)
	{
		addClass("menuitemseparator");
	}
};
