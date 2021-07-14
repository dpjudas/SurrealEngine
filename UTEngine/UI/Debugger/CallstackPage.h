#pragma once

#include "UI/Core/View.h"
#include "UObject/UObject.h"

class ListView;

class CallstackPage : public VBoxView
{
public:
	CallstackPage(View* parent);

	void updateList();

	ListView* listview = nullptr;
};
