#pragma once

#include "UI/Core/View.h"

class ImageBox;
class TextLabel;
class StatusbarItem;

class Statusbar : public HBoxView
{
public:
	Statusbar(View* parent);

	StatusbarItem* addItem(std::string text);
};

class StatusbarItem : public HBoxView
{
public:
	StatusbarItem(View* parent);

	ImageBox* icon = nullptr;
	TextLabel* text = nullptr;
};
