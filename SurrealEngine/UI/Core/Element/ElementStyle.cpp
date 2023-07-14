
#include "Precomp.h"
#include "ElementStyle.h"
#include "Element.h"
#include "ComputedBorder.h"
#include "Canvas.h"

ElementStyle::ElementStyle(std::set<std::string> initclasses) : classes(std::move(initclasses))
{
	if (isClass("debuggerwindow"))
	{
		values.backgroundColor = Colorf::fromRgba8(240, 240, 240);
	}

	if (isClass("tabbar"))
	{
		if (isClass("tabsbottom"))
		{
			values.border.top = 1;
			values.borderColorTop = Colorf::fromRgba8(200, 200, 200);
		}
		else
		{
			values.border.bottom = 1;
			values.borderColorBottom = Colorf::fromRgba8(200, 200, 200);
		}
	}

	if (isClass("tabcontrol-marginleft"))
	{
		values.margin.left = 7;
	}

	if (isClass("tabcontrol-marginright"))
	{
		values.margin.right = 7;
	}

	if (isClass("tabcontrolleft"))
	{
		values.margin.top = 0;
		values.margin.right = 0;
	}

	if (isClass("tabcontrolright"))
	{
		values.margin.top = 0;
		values.margin.left = 0;
	}

	if (isClass("tabbartab"))
	{
		if (isClass("tabsbottom"))
		{
			values.margin.top = -1;
			values.border.bottom = 1;
		}
		else
		{
			values.margin.bottom = -1;
			values.border.top = 1;
		}

		/*if (isClass("firstlefttab"))
		{
			values.margin.left = -1;
		}*/

		values.border.left = 1;
		values.border.right = 1;
		values.padding.top = 6;
		values.padding.bottom = 6;
		values.padding.left = 15;
		values.padding.right = 15;

		if (isClass("selected"))
		{
			if (isClass("tabsbottom"))
			{
				values.borderColorTop = Colorf::fromRgba8(255, 255, 255);
				values.borderColorBottom = Colorf::fromRgba8(200, 200, 200);
			}
			else
			{
				values.borderColorTop = Colorf::fromRgba8(200, 200, 200);
				values.borderColorBottom = Colorf::fromRgba8(255, 255, 255);
			}

			values.borderColorLeft = Colorf::fromRgba8(200, 200, 200);
			values.borderColorRight = Colorf::fromRgba8(200, 200, 200);
			values.backgroundColor = Colorf::fromRgba8(255, 255, 255);
		}
	}
	if (isClass("tabbartab-label"))
	{
		values.lineHeight = 20;
		inherit.lineHeight = false;
	}
	if (isClass("tabcontrol-widgetstack"))
	{
		if (isClass("tabsbottom"))
		{
			values.border.top = 1;
			values.borderColorTop = Colorf::fromRgba8(200, 200, 200);
		}
		else
		{
			values.border.bottom = 1;
			values.borderColorBottom = Colorf::fromRgba8(200, 200, 200);
		}

		values.border.left = 1;
		values.border.right = 1;
		values.borderColorLeft = Colorf::fromRgba8(200, 200, 200);
		values.borderColorRight = Colorf::fromRgba8(200, 200, 200);
		values.backgroundColor = Colorf::fromRgba8(255, 255, 255);
	}

	if (isClass("toolbar"))
	{
		values.padding.top = 5;
		values.padding.right = 5;
		values.padding.bottom = 5;
		values.padding.left = 10;
		values.lineHeight = 30;
		inherit.lineHeight = false;
	}
	if (isClass("toolbarbutton"))
	{
		values.padding.right = 10;
	}

	if (isClass("menubar") || isClass("menubarmodal"))
	{
		values.padding.left = 8;
		values.padding.right = 8;
		values.lineHeight = 30;
		inherit.lineHeight = false;
	}
	if (isClass("menubaritem") || isClass("menubarmodalitem"))
	{
		values.padding.top = 3;
		values.padding.bottom = 3;
		values.padding.left = 9;
		values.padding.right = 9;
		values.lineHeight = 24;
		inherit.lineHeight = false;
	}

	if (isClass("listviewheader"))
	{
		values.padding.left = 5;
		values.padding.right = 5;
		values.lineHeight = 24;
		inherit.lineHeight = false;
		values.backgroundColor = Colorf::fromRgba8(240, 240, 240);
	}
	if (isClass("listview-headersplitterline"))
	{
		values.padding.left = 1;
		values.backgroundColor = Colorf::fromRgba8(200, 200, 200);
	}
	if (isClass("listviewbody"))
	{
		values.backgroundColor = Colorf::fromRgba8(255, 255, 255);
		values.overflow = true;
	}
	if (isClass("listviewitem"))
	{
		values.padding.left = 5;
		values.padding.right = 5;
		values.lineHeight = 24;
		inherit.lineHeight = false;

		if (isClass("selected"))
		{
			values.backgroundColor = Colorf::fromRgba8(240, 232, 255);
		}
	}

	if (isClass("statusbar"))
	{
		values.padding.left = 5;
		values.padding.right = 5;
		values.padding.top = 5;
		values.padding.bottom = 5;
	}

	if (isClass("statusbaritem"))
	{
		values.padding.left = 5;
		values.padding.right = 5;
	}
}

Colorf ElementStyle::color(Element* element)
{
	return (inherit.color && element->parent()) ? element->parent()->color() : values.color;
}

double ElementStyle::lineHeight(Element* element)
{
	return (inherit.lineHeight && element->parent()) ? element->parent()->lineHeight() : values.lineHeight;
}

ComputedBorder ElementStyle::computedNoncontent(Element* element)
{
	ComputedBorder margin = computedMargin(element);
	ComputedBorder border = computedBorder(element);
	ComputedBorder padding = computedPadding(element);
	ComputedBorder nc;
	nc.left = margin.left + border.left + padding.left;
	nc.top = margin.top + border.top + padding.top;
	nc.right = margin.right + border.right + padding.right;
	nc.bottom = margin.bottom + border.bottom + padding.bottom;
	return nc;
}

ComputedBorder ElementStyle::computedMargin(Element* element)
{
	return values.margin;
}

ComputedBorder ElementStyle::computedBorder(Element* element)
{
	return values.border;
}

ComputedBorder ElementStyle::computedPadding(Element* element)
{
	ComputedBorder padding = values.padding;
	if (element->paddingLeft >= 0)
		padding.left = element->paddingLeft;
	return padding;
}
