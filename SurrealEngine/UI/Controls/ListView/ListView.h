#pragma once

#include "UI/Core/View.h"
#include "ListViewItem.h"
#include <list>

class TextLabel;
class ListViewHeader;
class ListViewBody;
class Menu;

enum class ScrollToHint
{
	ensureVisible,
	positionAtTop,
	positionAtBottom,
	positionAtCenter
};

class ListView : public VBoxView
{
public:
	ListView(View* parent);
	~ListView();

	void clearList();

	void addColumn(std::string name, double width, bool expanding = false);
	ListViewItem* rootItem() { return root.get(); }
	std::vector<ListViewItem*> selectedItems();
	ListViewItem* selectedItem();
	ListViewItem* focusedItem() { return curFocusItem; }

	void setAlternatingRowColors(bool enable);

	void scrollToItem(ListViewItem* item, ScrollToHint hint = ScrollToHint::ensureVisible);

	double scrollTop() const;
	double scrollPage() const;
	double scrollMax() const;
	void setScrollTop(double y);

	void selectItem(ListViewItem* item);
	void focusItem(ListViewItem* item);

	std::function<void(ListViewItem*)> activated;
	std::function<void(ListViewItem*)> clicked;
	std::function<void(ListViewItem*)> doubleClicked;
	std::function<void(ListViewItem*, Menu*)> onContextMenu;
	std::function<void(ListViewItem*)> collapsed;
	std::function<void(ListViewItem*)> expanded;
	std::function<void()> selectionChanged;
	std::function<void()> scroll;

private:
	void onBodyClick(Event* event);
	void onBodyScroll(Event* event);
	void onBodyFocus(Event* event);
	void onBodyBlur(Event* event);
	void onBodyKeyDown(Event* event);
	void onItemClick(Event* event, ListViewItem* item);
	void onItemContextMenu(Event* event, ListViewItem* item);
	void onItemAttached(ListViewItem* item);
	void onItemDetached(ListViewItem* item);
	void openItem(ListViewItem* item);
	void closeItem(ListViewItem* item);
	void createItemView(ListViewItem* item);
	void onColumnViewChanged(ListViewItemView* itemview, size_t index);

	ListViewHeader* header = nullptr;
	ListViewBody* body = nullptr;
	std::unique_ptr<ListViewItem> root;
	ListViewItem* curSelectedItem = nullptr;
	ListViewItem* curFocusItem = nullptr;

	friend class ListViewItem;
	friend class ListViewItemView;
};

class ListViewHeader : public HBoxView
{
public:
	ListViewHeader(View* parent) : HBoxView(parent)
	{
		addClass("listviewheader");
	}

	struct Column
	{
		TextLabel* label = nullptr;
		double width = 0.0;
		bool expanding = false;
		View* splitter = nullptr;
	};

	std::vector<Column> columns;
};

class ListViewBody : public VBoxView
{
public:
	ListViewBody(View* parent) : VBoxView(parent)
	{
		addClass("listviewbody");
	}
};
