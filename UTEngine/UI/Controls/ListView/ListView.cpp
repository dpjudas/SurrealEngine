
#include "Precomp.h"
#include "ListView.h"
#include "ListViewItem.h"
#include "UI/Controls/TextLabel/TextLabel.h"
#include "UI/MainWindow/Menu.h"

ListView::ListView(View* parent) : VBoxView(parent)
{
	addClass("listview");
	header = new ListViewHeader(this);
	header->addClass("listview-header");
	body = new ListViewBody(this);
	body->setExpanding();
	body->element->setTabIndex(0);
	header->addClass("listview-body");
	root.reset(new RootListViewItem(this));
	body->element->addEventListener("click", [=](Event* e) { onBodyClick(e); });
	body->element->addEventListener("scroll", [=](Event* e) { onBodyScroll(e); });
	body->element->addEventListener("focus", [=](Event* e) { onBodyFocus(e); });
	body->element->addEventListener("blur", [=](Event* e) { onBodyBlur(e); });
	body->element->addEventListener("keydown", [=](Event* e) { onBodyKeyDown(e); });
}

ListView::~ListView()
{
	root.reset();
}

void ListView::onBodyFocus(Event* event)
{
	addClass("focused");
	if (!focusedItem())
		focusItem(selectedItem());
}

void ListView::onBodyBlur(Event* event)
{
	removeClass("focused");
}

void ListView::onBodyKeyDown(Event* event)
{
	int keyCode = event->getKeyCode();
	bool processed = true;
	if (keyCode == 13) // Enter
	{
		if (activated && focusedItem())
			activated(focusedItem());
	}
	else if (keyCode == 33) // Page up
	{
		ListViewItem* item = focusedItem();
		if (item && item->view)
		{
			double pageHeight = body->element->clientHeight();
			double offsetHeight = item->view->element->offsetHeight();
			if (offsetHeight >= 1)
			{
				int itemsPerPage = (int)(pageHeight / offsetHeight);
				for (int i = 0; i < itemsPerPage; i++)
				{
					ListViewItem* prev = item->prevOpenItem();
					if (prev)
						item = prev;
					else
						break;
				}

				focusItem(item);
				selectItem(item);
			}
		}
	}
	else if (keyCode == 34) // Page down
	{
		ListViewItem* item = focusedItem();
		if (item && item->view)
		{
			double pageHeight = body->element->clientHeight();
			double offsetHeight = item->view->element->offsetHeight();
			if (offsetHeight >= 1)
			{
				int itemsPerPage = (int)(pageHeight / offsetHeight);
				for (int i = 0; i < itemsPerPage; i++)
				{
					ListViewItem* next = item->nextOpenItem();
					if (next)
						item = next;
					else
						break;
				}

				focusItem(item);
				selectItem(item);
			}
		}
	}
	else if (keyCode == 35) // End
	{
		ListViewItem* item = rootItem()->lastChild();
		while (item && item->lastChild())
			item = item->lastChild();
		if (item)
		{
			focusItem(item);
			selectItem(item);
		}
	}
	else if (keyCode == 36) // Home
	{
		ListViewItem* item = rootItem()->firstChild();
		if (item)
		{
			focusItem(item);
			selectItem(item);
		}
	}
	else if (keyCode == 38) // Arrow up
	{
		ListViewItem* item = focusedItem() ? focusedItem()->prevOpenItem() : nullptr;
		if (item)
		{
			focusItem(item);
			selectItem(item);
		}
	}
	else if (keyCode == 40) // Arrow down
	{
		ListViewItem* item = focusedItem() ? focusedItem()->nextOpenItem() : nullptr;
		if (item)
		{
			focusItem(item);
			selectItem(item);
		}
	}
	else
	{
		// printf("keycode: %d\n", keyCode);
		processed = false;
	}

	if (processed)
	{
		event->preventDefault();
		event->stopPropagation();
	}
}

void ListView::onBodyClick(Event* event)
{
	event->stopPropagation();
	event->preventDefault();
	selectItem(nullptr);
}

void ListView::onBodyScroll(Event* event)
{
	if (scroll)
		scroll();
}

void ListView::onItemClick(Event* event, ListViewItem* item)
{
	int detail = event->getDetail();
	event->stopPropagation();

	body->element->focus();
	addClass("focused");
	focusItem(item);
	selectItem(item);

	if (detail == 1)
	{
		if (clicked)
			clicked(item);
	}
	else if (detail == 2)
	{
		if (doubleClicked)
			doubleClicked(item);

		if (activated)
			activated(item);
	}
}

void ListView::onItemContextMenu(Event* event, ListViewItem* item)
{
	event->stopPropagation();
	event->preventDefault();
	focusItem(item);
	selectItem(item);
	if (onContextMenu)
	{
		double clientX = event->getClientX();
		double clientY = event->getClientY();
		auto openMenu = new Menu(nullptr);
		onContextMenu(item, openMenu);
		openMenu->showModal();
		openMenu->setLeftPosition(clientX, clientY);
		openMenu->parent()->element->addEventListener("click", [=](Event* event) { event->stopPropagation(); openMenu->closeModal(); });
		openMenu->closeMenu = [=]() { openMenu->closeModal(); };
	}
}

void ListView::setAlternatingRowColors(bool enable)
{
	if (enable)
		body->addClass("alternate");
	else
		body->removeClass("alternate");
}

void ListView::scrollToItem(ListViewItem* item, ScrollToHint hint)
{
	if (item->view)
	{
		double pageHeight = body->element->clientHeight();
		double offsetTop = item->view->element->offsetTop();
		double offsetHeight = item->view->element->offsetHeight();
		if (hint == ScrollToHint::ensureVisible)
		{
			double scrollTop = body->element->scrollTop();
			if (offsetTop < scrollTop)
				body->element->setScrollTop(std::max(offsetTop, 0.0));
			else if (offsetTop + offsetHeight > scrollTop + pageHeight)
				body->element->setScrollTop(std::max(offsetTop + offsetHeight - pageHeight, 0.0));
		}
		else if (hint == ScrollToHint::positionAtTop)
		{
			body->element->setScrollTop(std::max(offsetTop, 0.0));
		}
		else if (hint == ScrollToHint::positionAtBottom)
		{
			body->element->setScrollTop(std::max(offsetTop + offsetHeight - pageHeight, 0.0));
		}
		else if (hint == ScrollToHint::positionAtCenter)
		{
			body->element->setScrollTop(std::max(offsetTop + (offsetHeight - pageHeight) / 2.0, 0.0));
		}
	}
}

double ListView::scrollTop() const
{
	return body->element->scrollTop();
}

double ListView::scrollPage() const
{
	return body->element->clientHeight();
}

double ListView::scrollMax() const
{
	return body->element->scrollHeight();
}

void ListView::setScrollTop(double y)
{
	body->element->setScrollTop(y);
}

void ListView::focusItem(ListViewItem* item)
{
	if (curFocusItem != item)
	{
		if (curFocusItem && curFocusItem->view)
			curFocusItem->view->removeClass("focused");
		curFocusItem = item;
		if (curFocusItem && curFocusItem->view)
			curFocusItem->view->addClass("focused");
		if (curFocusItem)
			scrollToItem(curFocusItem);
	}
}

void ListView::selectItem(ListViewItem* item)
{
	if (curSelectedItem != item)
	{
		if (curSelectedItem && curSelectedItem->view)
			curSelectedItem->view->removeClass("selected");
		curSelectedItem = item;
		if (curSelectedItem && curSelectedItem->view)
			curSelectedItem->view->addClass("selected");
		if (selectionChanged)
			selectionChanged();
	}
}

void ListView::clearList()
{
	if (root != nullptr)
		root->removeAllChildren();
}

void ListView::addColumn(std::string name, double width, bool expanding)
{
	ListViewHeader::Column col;
	if (!header->columns.empty())
	{
		col.splitter = new HBoxView(header);
		col.splitter->addClass("listview-headersplitter");
		col.splitter->element->setStyle("width", "10px");
		auto v = new View(col.splitter);
		v->addClass("listview-headersplitterline");
	}
	col.label = new TextLabel(header);
	col.label->addClass("listview-headerlabel");
	col.label->setText(name);
	col.label->element->setStyle("width", std::to_string(width) + "px");
	if (expanding)
		col.label->element->setStyle("flexGrow", "1");
	col.width = width;
	col.expanding = expanding;
	header->columns.push_back(col);
}

std::vector<ListViewItem*> ListView::selectedItems()
{
	std::vector<ListViewItem*> result;
	if (curSelectedItem)
		result.push_back(curSelectedItem);
	return result;
}

ListViewItem* ListView::selectedItem()
{
	auto items = selectedItems();
	return (items.size() == 1) ? items.front() : nullptr;
}

void ListView::openItem(ListViewItem* item)
{
	if (item != root.get())
	{
		item->openFlag = true;
		onItemAttached(item);
	}
}

void ListView::closeItem(ListViewItem* item)
{
	if (item != root.get())
	{
		onItemDetached(item);
		item->openFlag = false;
	}
}

void ListView::onItemAttached(ListViewItem* item)
{
	if (item != root.get())
	{
		createItemView(item);

		if (item->isOpen())
		{
			for (ListViewItem* cur = item->firstChild(); cur != nullptr; cur = cur->nextSibling())
			{
				onItemAttached(cur);
			}
		}
	}
}

void ListView::onItemDetached(ListViewItem* item)
{
	if (item != root.get() && item->view)
	{
		if (item->isOpen())
		{
			for (ListViewItem* cur = item->firstChild(); cur != nullptr; cur = cur->nextSibling())
			{
				onItemDetached(cur);
			}
		}

		delete item->view;
		item->view = nullptr;
	}
}

void ListView::createItemView(ListViewItem* item)
{
	item->view = new ListViewItemView(item, body);
	item->view->element->addEventListener("click", [=](Event* e) { onItemClick(e, item); });
	item->view->element->addEventListener("contextmenu", [=](Event* e) { onItemContextMenu(e, item); });

	ListViewItem* nextItem = item;
	while (nextItem && !nextItem->nextSibling())
		nextItem = nextItem->parent();
	if (nextItem)
		nextItem = nextItem->nextSibling();
	if (nextItem && nextItem->view)
		item->view->moveBefore(nextItem->view);

	for (size_t i = 0; i < header->columns.size(); i++)
	{
		item->updateColumn(i);

		View* columnView = item->view->getColumnView(i);
		if (!columnView)
		{
			columnView = new View(item->view);
			item->view->setColumnView(i, columnView);
		}
	}
}

void ListView::onColumnViewChanged(ListViewItemView *itemview, size_t i)
{
	double splitterSize = (i + 1 < header->columns.size()) ? 10 : 0;

	View* columnView = itemview->getColumnView(i);

	if (i == 0)
	{
		int depth = 0;
		ListViewItem* p = itemview->getItem()->parent();
		while (p && p != root.get())
		{
			depth++;
			p = p->parent();
		}

		if (i == 0 && depth > 0)
		{
			double padding = depth * 24;
			double width = std::max(header->columns[i].width - padding, 0.0);
			columnView->element->setStyle("width", std::to_string(width + splitterSize) + "px");
			columnView->element->setStyle("padding-left", std::to_string(padding) + "px");
		}
		else
		{
			columnView->element->setStyle("width", std::to_string(header->columns[i].width + splitterSize) + "px");
		}
	}
	else
	{
		columnView->element->setStyle("width", std::to_string(header->columns[i].width + splitterSize) + "px");
	}

	if (header->columns[i].expanding)
		columnView->element->setStyle("flexGrow", "1");
}
