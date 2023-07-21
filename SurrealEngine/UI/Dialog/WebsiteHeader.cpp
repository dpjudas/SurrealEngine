
#include "Precomp.h"
#include "WebsiteHeader.h"
#include "DialogHeader.h"
#include "DialogHeaderButton.h"
#include "Dialog.h"
#include "AppMenu.h"
#include "UI/Controls/ImageBox/ImageBox.h"
#include "UI/MainWindow/Menu.h"

WebsiteHeader::WebsiteHeader(View* p) : HBoxView(p)
{
	menuButton = new ImageBox(this);
	caption = new TextLabel(this);
	accountMenu = new TextLabel(this);

	addClass("websiteheader");
	menuButton->addClass("websiteheader-menubutton");
	caption->addClass("websiteheader-caption");
	accountMenu->addClass("websiteheader-accountmenu");

	caption->setExpanding();

	menuButton->element->addEventListener("click", [this](Event* event) { event->stopPropagation(); onMenu(); });
	accountMenu->element->addEventListener("click", [this](Event* event) { event->stopPropagation(); onAccount(); });
}

void WebsiteHeader::setIcon(std::string icon)
{
	iconSrc = icon;
	menuButton->setSrc(iconSrc);
}

void WebsiteHeader::setTitle(std::string title)
{
	caption->setText(title);
}

void WebsiteHeader::setAccountMenuText(std::string text)
{
	accountMenu->setText(text);
}

void WebsiteHeader::onMenu()
{
	auto menu = new AppMenu(nullptr);
	menu->setIcon(iconSrc);
	if (onAppMenu)
		onAppMenu(menu);
	menu->showModal();
}

void WebsiteHeader::onAccount()
{
	Rect box = accountMenu->element->getBoundingClientRect();
	auto openMenu = new Menu(nullptr);
	openMenu->showModal();
	openMenu->setRightPosition(element->getBoundingClientRect().width - box.x - box.width, box.y + box.height - 1);
	openMenu->parent()->element->addEventListener("click", [=](Event* event) { event->stopPropagation(); openMenu->closeModal(); });
	openMenu->closeMenu = [=]() { openMenu->closeModal(); };
	if (onAccountMenu)
	{
		onAccountMenu(openMenu);
	}
}
