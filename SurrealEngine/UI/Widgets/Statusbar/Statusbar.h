
#pragma once

#include "UI/Core/Widget.h"

class Statusbar : public Widget
{
public:
	Statusbar(Widget* parent);
	~Statusbar();

protected:
	void OnPaint(Canvas* canvas) override;
};
