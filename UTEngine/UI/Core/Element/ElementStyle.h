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

	ComputedBorder computedBorder(Element* element);
	bool overflow(Element* element) { return values.overflow; }
	Colorf color(Element* element);
	double lineHeight(Element* element);
	Colorf backgroundColor(Element* element) { return values.backgroundColor; }
	double scrollbarWidth(Element* element) { return values.scrollbarWidth; }
	double thumbMargin(Element* element) { return values.thumbMargin; }

private:
	bool isClass(const char* cls) { return classes.find(cls) != classes.end(); }

	std::set<std::string> classes;

	struct
	{
		Colorf color = Colorf::fromRgba8(0, 0, 0);
		double lineHeight = 20;
		ComputedBorder border;
		double scrollbarWidth = 16;
		double thumbMargin = 4;
		Colorf backgroundColor = Colorf::transparent();
		bool overflow = false;
	} values;

	struct
	{
		bool color = true;
		bool lineHeight = true;
	} inherit;
};
