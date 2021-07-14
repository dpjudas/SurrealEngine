#pragma once

#include "UI/Core/View.h"

class ListView;
class UFunction;

class DisassemblyPage : public VBoxView
{
public:
	DisassemblyPage(View* parent);

	void setFunction(UFunction* func);

	ListView* listview = nullptr;
};
