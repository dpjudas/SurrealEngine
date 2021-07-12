#pragma once

#include "UI/Core/View.h"

class ListView;

class ObjectViewerPage : public VBoxView
{
public:
	ObjectViewerPage(View* parent);

	ListView* listview = nullptr;
};
