#pragma once

#include "UI/Core/View.h"
#include "UObject/UObject.h"
#include <list>

class ListView;

class LogPage : public VBoxView
{
public:
	LogPage(View* parent);

	void update();

	ListView* listview = nullptr;
	std::list<std::string>::iterator it;
};
