#pragma once

#include "UI/Core/View.h"

class ListView;
class Frame;

class LocalsPage : public VBoxView
{
public:
	LocalsPage(View* parent);

	void setFrame(Frame* frame);

	ListView* listview = nullptr;
};
