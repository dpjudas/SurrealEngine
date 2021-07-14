#pragma once

#include "UI/Core/View.h"
#include "UObject/UObject.h"

class ListView;

class ObjectViewerPage : public VBoxView
{
public:
	ObjectViewerPage(View* parent);

	void setObject(UObject* obj);

	ListView* listview = nullptr;
};
