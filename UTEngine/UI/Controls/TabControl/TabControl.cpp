
#include "Precomp.h"
#include "TabControl.h"

TabControl::TabControl(View* parent) : VBoxView(parent)
{
	setupUi();
}

void TabControl::setupUi()
{
	tabs = new TabBar(this);

	widgetStack = new VBoxView(this);
	widgetStack->addClass("tabcontrol-widgetstack");
	widgetStack->setExpanding();

	addClass("tabcontrol");
}

void TabControl::addPage(std::string icon, std::string label, View* page)
{
	auto tab = new TabBarTab(tabs);
	tab->setText(label);
	tab->setIcon(icon);
	tab->element->addEventListener("click", [=](Event* event) { event->stopPropagation(); onPageTabClicked(page); });

	page->setExpanding();

	pages[tab] = std::unique_ptr<View>(page);
	if (!currentPage)
		showPage(page);
}

void TabControl::showPage(View* page)
{
	if (page != currentPage)
	{
		if (currentPage)
		{
			findTab(currentPage)->removeClass("selected");
			currentPage->setParent(nullptr);
		}
		currentPage = page;
		if (currentPage)
		{
			currentPage->setParent(widgetStack);
			findTab(currentPage)->addClass("selected");
		}
	}
}

void TabControl::onPageTabClicked(View* page)
{
	showPage(page);
}

TabBarTab* TabControl::findTab(View* page)
{
	for (auto& it : pages)
	{
		if (it.second.get() == page)
			return it.first;
	}
	return nullptr;
}
