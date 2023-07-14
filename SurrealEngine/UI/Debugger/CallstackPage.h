#pragma once

#include "UI/Core/View.h"
#include "UObject/UObject.h"

class ListView;
class ListViewItem;
class Frame;

class CallstackPage : public VBoxView
{
public:
	CallstackPage(View* parent);

	void onActivated(ListViewItem* item);
	void updateList();

	std::function<void(Frame* frame)> activated;

	ListView* listview = nullptr;
};
