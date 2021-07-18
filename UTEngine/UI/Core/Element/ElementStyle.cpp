
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

	if (isClass("tabbartab"))
	{
		values.border.top = 6;
		values.border.bottom = 6;
		values.border.left = 15;
		values.border.right = 15;

		if (isClass("selected"))
		{
			// values.borderColor = Colorf::fromRgba8(255, 255, 255);
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
		values.backgroundColor = Colorf::fromRgba8(255, 255, 255);
	}

	if (isClass("toolbar"))
	{
		values.border.top = 5;
		values.border.right = 5;
		values.border.bottom = 5;
		values.border.left = 10;
		values.lineHeight = 30;
		inherit.lineHeight = false;
	}
	if (isClass("toolbarbutton"))
	{
		values.border.right = 10;
	}

	if (isClass("menubar") || isClass("menubarmodal"))
	{
		values.border.left = 8;
		values.border.right = 8;
		values.lineHeight = 30;
		inherit.lineHeight = false;
	}
	if (isClass("menubaritem") || isClass("menubarmodalitem"))
	{
		values.border.top = 3;
		values.border.bottom = 3;
		values.border.left = 9;
		values.border.right = 9;
		values.lineHeight = 24;
		inherit.lineHeight = false;
	}

	if (isClass("listviewheader"))
	{
		values.border.left = 5;
		values.border.right = 5;
		values.lineHeight = 24;
		inherit.lineHeight = false;
		values.backgroundColor = Colorf::fromRgba8(240, 240, 240);
	}
	if (isClass("listview-headersplitterline"))
	{
		values.border.left = 1;
		values.backgroundColor = Colorf::fromRgba8(200, 200, 200);
	}
	if (isClass("listviewbody"))
	{
		values.backgroundColor = Colorf::fromRgba8(255, 255, 255);
		values.overflow = true;
	}
	if (isClass("listviewitem"))
	{
		values.border.left = 5;
		values.border.right = 5;
		values.lineHeight = 24;
		inherit.lineHeight = false;

		if (isClass("selected"))
		{
			values.backgroundColor = Colorf::fromRgba8(240, 232, 255);
		}
	}

	if (isClass("statusbaritem"))
	{
		values.border.left = 5;
		values.border.right = 5;
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

ComputedBorder ElementStyle::computedBorder(Element* element)
{
	ComputedBorder border = values.border;
	if (element->paddingLeft >= 0)
		border.left = element->paddingLeft;
	return border;
}
