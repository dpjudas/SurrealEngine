#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/ImageBox/ImageBox.h"
#include "UI/Controls/TextLabel/TextLabel.h"

class TextLabel;
class ImageBox;
class AppMenuItem;

class AppMenu : public VBoxView
{
public:
	AppMenu(View* parent);

	void setIcon(std::string src);
	AppMenuItem* addItem(std::string icon, std::string text, std::function<void()> onClick = {});

private:
	void onAttach() override;

	ImageBox* menuButton = nullptr;
	TextLabel* title = nullptr;
	View* itemList = nullptr;
};

class AppMenuItem : public HBoxView
{
public:
	AppMenuItem(View* parent) : HBoxView(parent)
	{
		addClass("appmenuitem");
		icon = new ImageBox(this);
		icon->addClass("appmenuitem-icon");
		text = new TextLabel(this);
		text->addClass("appmenuitem-text");
	}

	ImageBox* icon = nullptr;
	TextLabel* text = nullptr;
};
