#pragma once

#include "UI/Core/View.h"

class ListViewItem;

class ListViewItemView : public HBoxView
{
public:
	ListViewItemView(ListViewItem* item, View* parent);

	View* getColumnView(size_t index);
	void setColumnView(size_t index, View* view);

	ListViewItem* getItem() const { return item; }

private:
	ListViewItem* item = nullptr;
	std::vector<View*> columns;
};
