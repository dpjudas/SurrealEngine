#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/ImageBox/ImageBox.h"
#include "UI/Controls/TextLabel/TextLabel.h"

class TabBar;
class TabBarTab;

enum class TabControlBorderStyle
{
	none,
	left,
	right
};

enum class TabBarPosition
{
	top,
	bottom
};

class TabControl : public VBoxView
{
public:
	TabControl(View* parent);

	void setBorderStyle(TabControlBorderStyle style);
	void setBarPosition(TabBarPosition pos);
	void addPage(std::string icon, std::string label, View* page);
	void showPage(View* page);

	std::function<void()> apply;

private:
	TabBarTab* findTab(View* page);
	void onPageTabClicked(View* page);
	void setupUi();

	TabControlBorderStyle borderstyle = TabControlBorderStyle::none;
	TabBarPosition barpos = TabBarPosition::top;
	TabBar* tabs = nullptr;
	std::map<TabBarTab*, std::unique_ptr<View>> pages;
	View* currentPage = nullptr;

	VBoxView* widgetStack = nullptr;
};

class TabBar : public HBoxView
{
public:
	TabBar(View* parent) : HBoxView(parent) { addClass("tabbar"); }
};

class TabBarTab : public HBoxView
{
public:
	TabBarTab(View* parent) : HBoxView(parent)
	{
		icon = new ImageBox(this);
		label = new TextLabel(this);

		addClass("tabbartab");
		icon->addClass("tabbartab-icon");
		label->addClass("tabbartab-label");
	}

	void setText(const std::string& text)
	{
		label->setText(text);
	}

	void setIcon(const std::string& src)
	{
		icon->setSrc(src);
	}

private:
	ImageBox* icon = nullptr;
	TextLabel* label = nullptr;
};
