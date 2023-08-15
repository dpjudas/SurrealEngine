
#pragma once

#include "UI/Core/Widget.h"

class ToolbarButton : public Widget
{
public:
	ToolbarButton(Widget* parent);
	~ToolbarButton();

protected:
	void OnPaint(Canvas* canvas) override;
};
