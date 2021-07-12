#pragma once

#include "UI/Core/View.h"

class ListView;

class DisassemblyPage : public VBoxView
{
public:
	DisassemblyPage(View* parent);

	ListView* listview = nullptr;
};
