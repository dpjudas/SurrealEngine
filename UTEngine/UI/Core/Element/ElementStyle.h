#pragma once

#include "Colorf.h"
#include "ComputedBorder.h"
#include <set>
#include <string>

class Element;

class ElementStyle
{
public:
	ElementStyle(std::set<std::string> classes);

	ComputedBorder computedNoncontent(Element* element);
	ComputedBorder computedMargin(Element* element);
	ComputedBorder computedBorder(Element* element);
	ComputedBorder computedPadding(Element* element);
	bool overflow(Element* element) { return values.overflow; }
	Colorf color(Element* element);
	double lineHeight(Element* element);
	Colorf backgroundColor(Element* element) { return values.backgroundColor; }
	double scrollbarWidth(Element* element) { return values.scrollbarWidth; }
	double thumbMargin(Element* element) { return values.thumbMargin; }
	Colorf borderColorLeft(Element* element) { return values.borderColorLeft; }
	Colorf borderColorTop(Element* element) { return values.borderColorTop; }
	Colorf borderColorRight(Element* element) { return values.borderColorRight; }
	Colorf borderColorBottom(Element* element) { return values.borderColorBottom; }

private:
	bool isClass(const char* cls) { return classes.find(cls) != classes.end(); }

	std::set<std::string> classes;

	struct
	{
		Colorf color = Colorf::fromRgba8(0, 0, 0);
		double lineHeight = 20;
		ComputedBorder margin;
		ComputedBorder border;
		ComputedBorder padding;
		double scrollbarWidth = 16;
		double thumbMargin = 4;
		Colorf borderColorLeft = Colorf::transparent();
		Colorf borderColorTop = Colorf::transparent();
		Colorf borderColorRight = Colorf::transparent();
		Colorf borderColorBottom = Colorf::transparent();
		Colorf backgroundColor = Colorf::transparent();
		bool overflow = false;
	} values;

	struct
	{
		bool color = true;
		bool lineHeight = true;
	} inherit;
};
