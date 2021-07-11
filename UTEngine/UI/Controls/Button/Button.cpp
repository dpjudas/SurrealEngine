
#include "Precomp.h"
#include "Button.h"
#include "UI/Controls/ImageBox/ImageBox.h"
#include "UI/Controls/TextLabel/TextLabel.h"

Button::Button(View* parent) : View(parent, "button")
{
	addClass("button");
	label = new TextLabel(this);
}

void Button::setIcon(std::string src)
{
	if (!image)
	{
		image = new ImageBox(this);
		image->moveBefore(label);
	}
	image->setSrc(src);
}

void Button::setText(std::string text)
{
	label->setText(text);
}

void Button::setEnabled(bool value)
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

bool Button::getEnabled() const
{
	return enabled;
}
