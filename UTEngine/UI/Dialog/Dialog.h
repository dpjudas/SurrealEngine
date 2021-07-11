#pragma once

#include "UI/Core/View.h"
#include "DialogHeader.h"
#include "DialogButtonbar.h"
#include <cmath>

class Dialog : public VBoxView
{
public:
	Dialog(const std::string& title = std::string());

	void setTitle(std::string text);
	void setSize(double width, double height, bool fixedHeight = false);

	virtual void onClose(Event* event);

	DialogHeader* header = nullptr;
	VBoxView* centerView = nullptr;
	DialogButtonbar* buttonbar = nullptr;

private:
	void setupUi();

};
