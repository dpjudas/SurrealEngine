
#include "Precomp.h"
#include "LineEdit.h"

LineEdit::LineEdit(View* parent) : View(parent, "input")
{
	addClass("lineedit");
	element->setAttribute("type", "text");
}

void LineEdit::setPasswordMode(bool value)
{
	element->setAttribute("type", value ? "password" : "text");
}

void LineEdit::setText(std::string text)
{
	element->setAttribute("value", text);
}

void LineEdit::setPlaceholder(std::string text)
{
	element->setAttribute("placeholder", text);
}

std::string LineEdit::getText()
{
	return element->getValue();
}

void LineEdit::setEnabled(bool value)
{
	if (enabled != value)
	{
		if (value)
		{
			element->removeAttribute("disabled");
		}
		else
		{
			element->setAttribute("disabled", "");
		}
		enabled = value;
	}
}

bool LineEdit::getEnabled() const
{
	return enabled;
}
