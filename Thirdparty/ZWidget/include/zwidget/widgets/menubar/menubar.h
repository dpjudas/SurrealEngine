
#pragma once

#include "../../core/widget.h"
#include "../textlabel/textlabel.h"
#include "../imagebox/imagebox.h"
#include <vector>

class Menu;
class MenubarItem;
class MenuItem;
class MenuItemSeparator;

class Menubar : public Widget
{
public:
	Menubar(Widget* parent);
	~Menubar();

	MenubarItem* AddItem(std::string text, std::function<void(Menu* menu)> onOpen, bool alignRight = false);

protected:
	void OnGeometryChanged() override;

private:
	void ShowMenu(MenubarItem* item);
	void CloseMenu();

	std::vector<MenubarItem*> menuItems;
	Menu* openMenu = nullptr;

	friend class MenubarItem;
};

class MenubarItem : public Widget
{
public:
	MenubarItem(Menubar* menubar, std::string text, bool alignRight);

	void SetOpenCallback(std::function<void(Menu* menu)> callback) { onOpen = std::move(callback); }
	const std::function<void(Menu* menu)>& GetOpenCallback() const { return onOpen; }

	double GetPreferredWidth() const;

	bool AlignRight = false;

protected:
	void OnPaint(Canvas* canvas) override;
	bool OnMouseDown(const Point& pos, InputKey key) override;
	bool OnMouseUp(const Point& pos, InputKey key) override;
	void OnMouseMove(const Point& pos) override;
	void OnMouseLeave() override;

private:
	Menubar* menubar = nullptr;
	std::function<void(Menu* menu)> onOpen;
	std::string text;
};

class Menu : public Widget
{
public:
	Menu(Widget* parent);

	void SetLeftPosition(const Point& globalPos);
	void SetRightPosition(const Point& globalPos);
	MenuItem* AddItem(std::shared_ptr<Image> icon, std::string text, std::function<void()> onClick = {});
	MenuItemSeparator* AddSeparator();

	double GetPreferredWidth() const;
	double GetPreferredHeight() const;

protected:
	void OnGeometryChanged() override;

	std::function<void()> onCloseMenu;

	friend class Menubar;
};

class MenuItem : public Widget
{
public:
	MenuItem(Widget* parent);

	ImageBox* icon = nullptr;
	TextLabel* text = nullptr;

protected:
	void OnMouseMove(const Point& pos) override;
	void OnMouseLeave() override;
	void OnGeometryChanged() override;
};

class MenuItemSeparator : public Widget
{
public:
	MenuItemSeparator(Widget* parent);
};
