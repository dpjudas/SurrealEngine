#pragma once

#include "UI/Core/View.h"

class DialogButton;

class DialogButtonbar : public HBoxView
{
public:
	DialogButtonbar(View* parent);

	void setFlat(bool value);

	DialogButton* addButton(std::string icon, std::string text, std::function<void()> callback, bool leftAlign = false);
	DialogButton* addButton(std::string text, std::function<void()> callback, bool leftAlign = false) { return addButton({}, std::move(text), std::move(callback), leftAlign); }
	View* addSeparator();

	View* spacer = nullptr;
};
