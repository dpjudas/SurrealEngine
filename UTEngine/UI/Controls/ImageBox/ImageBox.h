#pragma once

#include "UI/Core/View.h"

class ImageBox : public View
{
public:
	ImageBox(View* parent);

	void setSrc(std::string src);

	void setSize(const int width, const int height);

	void setClickHandler(const std::function<void()>& handler);

};
