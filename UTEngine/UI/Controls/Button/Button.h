#pragma once

#include "UI/Core/View.h"

class ImageBox;
class TextLabel;

class Button : public View
{
public:
	Button(View* parent);

	void setIcon(std::string src);
	void setText(std::string text);
	void setEnabled(bool value);
	bool getEnabled() const;

	bool enabled = true;

private:
	ImageBox* image = nullptr;
	TextLabel* label = nullptr;
};
