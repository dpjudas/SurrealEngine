
#include "Precomp.h"
#include "ListViewItemView.h"
#include "ListViewItem.h"
#include "ListView.h"

ListViewItemView::ListViewItemView(ListViewItem* item, View* parent) : HBoxView(parent), item(item)
{
	addClass("listviewitem");
}

View* ListViewItemView::getColumnView(size_t index)
{
	if (index < columns.size())
		return columns[index];
	else
		return nullptr;
}

void ListViewItemView::setColumnView(size_t index, View* view)
{
	if (index >= columns.size())
		columns.resize(index + 1);

	if (columns[index] != view)
	{
		delete columns[index];
		columns[index] = view;
		view->setParent(this);
		view->moveBefore(index + 1 < columns.size() ? columns[index + 1] : nullptr);
		item->listview()->onColumnViewChanged(this, index);
	}
}
