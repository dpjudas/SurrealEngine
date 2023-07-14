#pragma once

#include "UI/Core/View.h"

class TextLabel : public View
{
public:
	TextLabel(View* parent);

	void setCenterAlign();

	void setText(std::string text);
	std::string getText();

	void setHtml(std::string html);
	std::string getHtml();

	void setClickHandler(const std::function<void()>& handler);
};
