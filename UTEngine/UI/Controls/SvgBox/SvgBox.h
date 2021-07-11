#pragma once

#include "UI/Core/View.h"

class SvgBox : public View
{
public:
	SvgBox(View* parent);

	void setViewbox(double x, double y, double width, double height);
	void clear();
	void addElement(std::string tag, std::map<std::string, std::string> attributes);

private:
#ifdef __EMSCRIPTEN__
	emscripten::val createElement(std::string tag, std::map<std::string, std::string> attributes);
#endif

	static const std::string xmlns;
};
