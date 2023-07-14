#pragma once

#include "UI/Core/View.h"

class LineEdit : public View
{
public:
	LineEdit(View* parent = nullptr);

	void setPasswordMode(bool value);
	void setText(std::string text);
	void setPlaceholder(std::string text);
	std::string getText();
	void setEnabled(bool value);
	bool getEnabled() const;

private:
	bool enabled = true;
};
