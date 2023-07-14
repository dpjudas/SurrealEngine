#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/TextLabel/TextLabel.h"

class TextLabel;
class ImageBox;
class AppMenu;
class Menu;

class WebsiteHeader : public HBoxView
{
public:
	WebsiteHeader(View* parent);

	void setIcon(std::string src);
	void setTitle(std::string title);
	void setAccountMenuText(std::string text);

	std::function<void(AppMenu* menu)> onAppMenu;
	std::function<void(Menu* menu)> onAccountMenu;

private:
	void onMenu();
	void onAccount();

	std::string iconSrc;
	ImageBox* menuButton = nullptr;
	TextLabel* caption = nullptr;
	TextLabel* accountMenu = nullptr;
};
