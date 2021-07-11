#pragma once

#include "Element.h"

class BoxElement : public Element
{
public:
	BoxElement();

protected:
	double preferredWidth(Canvas* canvas) override;
	double preferredHeight(Canvas* canvas, double width) override;
	double firstBaselineOffset(Canvas* canvas, double width) override;
	double lastBaselineOffset(Canvas* canvas, double width) override;
	void renderContent(Canvas* canvas) override;

	bool vbox = true;
};
