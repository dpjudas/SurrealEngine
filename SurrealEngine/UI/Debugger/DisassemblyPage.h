#pragma once

#include "UI/Core/View.h"

class ListView;
class UStruct;

class DisassemblyPage : public VBoxView
{
public:
	DisassemblyPage(View* parent);

	void setFunction(UStruct* func);

	ListView* listview = nullptr;
};
