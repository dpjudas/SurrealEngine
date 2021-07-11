
#include "Precomp.h"
#include "BoxElement.h"
#include "ComputedBorder.h"

BoxElement::BoxElement()
{
}

double BoxElement::preferredWidth(Canvas* canvas)
{
	if (vbox)
	{
		double w = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			ComputedBorder border = element->computedBorder();
			w = std::max(element->preferredWidth(canvas) + border.left + border.right, w);
		}
		return w;
	}
	else
	{
		double w = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			w = std::max(element->preferredWidth(canvas), w);
		}
		return w;
	}
}

double BoxElement::preferredHeight(Canvas* canvas, double width)
{
	if (vbox)
	{
		double h = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			ComputedBorder border = element->computedBorder();
			h += element->preferredHeight(canvas, width - border.left - border.right) + border.top + border.bottom;
		}
		return h;
	}
	else
	{
		double h = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			h += element->preferredHeight(canvas, width);
		}
		return h;
	}
}

double BoxElement::firstBaselineOffset(Canvas* canvas, double width)
{
	if (vbox)
	{
		if (firstChild())
		{
			ComputedBorder border = firstChild()->computedBorder();
			return firstChild()->firstBaselineOffset(canvas, width) + border.top;
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if (firstChild())
			return firstChild()->firstBaselineOffset(canvas, width);
		else
			return 0.0f;
	}
}

double BoxElement::lastBaselineOffset(Canvas* canvas, double width)
{
	if (vbox)
	{
		if (firstChild())
		{
			double h = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				if (element != lastChild())
				{
					ComputedBorder border = element->computedBorder();
					h += element->preferredHeight(canvas, width) + border.top + border.bottom;
				}
			}
			return h + lastChild()->lastBaselineOffset(canvas, width);
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if (firstChild())
		{
			double h = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				if (element != lastChild())
					h += element->preferredHeight(canvas, width);
			}
			return h + lastChild()->lastBaselineOffset(canvas, width);
		}
		else
		{
			return 0.0f;
		}
	}
}

void BoxElement::renderContent(Canvas* canvas)
{
	if (needsLayout())
	{
		if (vbox)
		{
			double width = geometry().contentWidth;
			double y = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				ComputedBorder border = element->computedBorder();

				ElementGeometry childpos;
				childpos.marginTop = 0.0f;
				childpos.marginBottom = 5.0f;
				childpos.borderLeft = border.left;
				childpos.borderTop = border.top;
				childpos.borderRight = border.right;
				childpos.borderBottom = border.bottom;
				childpos.contentX = border.left;
				childpos.contentY = y + childpos.borderTop;
				childpos.contentWidth = std::min(element->preferredWidth(canvas), width - border.left - border.right);
				childpos.contentHeight = element->preferredHeight(canvas, childpos.contentWidth);
				element->setGeometry(childpos);

				y += childpos.contentHeight + childpos.borderTop + childpos.borderBottom + childpos.marginBottom;
			}
		}
		else
		{
			double width = geometry().contentWidth;
			double y = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				ElementGeometry childpos;
				childpos.contentY = y;
				childpos.contentWidth = width;
				childpos.contentHeight = element->preferredHeight(canvas, width);
				element->setGeometry(childpos);

				y += childpos.contentHeight;
			}
		}
		clearNeedsLayout();
	}

	for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
	{
		element->render(canvas);
	}
}
