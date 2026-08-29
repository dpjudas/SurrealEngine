
#include "widgets/radiobuttonlabel/radiobuttonlabel.h"

RadioButtonLabel::RadioButtonLabel(RadioGroup* group, Widget* parent) : Widget(parent)
{
	SetStyleClass("radiobutton-label");
	SetGroup(group);
}

void RadioButtonLabel::SetGroup(RadioGroup* newGroup)
{
	if (group != newGroup)
	{
		if (group)
			std::erase(group->buttons, this);
		group = newGroup;
		if (group)
			group->buttons.push_back(this);
	}
}

void RadioButtonLabel::SetText(const std::string& value)
{
	if (text != value)
	{
		text = value;
		Update();
	}
}

const std::string& RadioButtonLabel::GetText() const
{
	return text;
}

void RadioButtonLabel::SetChecked(bool value)
{
	if (value != checked)
	{
		checked = value;
		Update();
	}
}

Size RadioButtonLabel::GetRadioButtonSize()
{
	if (auto image = GetStyleImage("checked-image"))
	{
		double w = GetStyleDouble("checked-image-width");
		double h = GetStyleDouble("checked-image-height");
		if (w == 0.0)
			w = (double)image->GetWidth();
		if (h == 0.0)
			h = (double)image->GetHeight();
		return { w, h };
	}
	else
	{
		return { 12.0, 12.0 };
	}
}

bool RadioButtonLabel::GetChecked() const
{
	return checked;
}

double RadioButtonLabel::GetPreferredWidth()
{
	const auto canvas = GetCanvas();
	return GetRadioButtonSize().width + canvas->measureText(GetFont(), text).width + 1.0;
}

double RadioButtonLabel::GetPreferredHeight()
{
	return std::max(GetRadioButtonSize().height, GetCanvas()->getFontMetrics(GetFont()).height);
}

void RadioButtonLabel::OnPaint(Canvas* canvas)
{
	Size s = GetRadioButtonSize();

	FontMetrics metrics = canvas->getFontMetrics(GetFont());
	double baseline = (GetHeight() - metrics.height) * 0.5 + metrics.ascent;

	double borderwidth = GridFitSize(1.0);
	double outerboxsize = GridFitSize(10.0);
	double center = GridFitPoint(baseline - outerboxsize * 0.5);
	double innerboxsize = outerboxsize - 2.0 * borderwidth;
	double checkedsize = innerboxsize - 2.0 * borderwidth;

	if (checked)
	{
		if (auto image = GetStyleImage("checked-image"))
		{
			canvas->drawImage(image, Rect(0.0, center - s.height * 0.5 - GetStyleDouble("checked-align"), s.width, s.height));
		}
		else
		{
			canvas->fillRect(Rect::xywh(0.0, center - 6.0 * borderwidth, outerboxsize, outerboxsize), GetStyleColor("checked-outer-border-color"));
			canvas->fillRect(Rect::xywh(1.0 * borderwidth, center - 5.0 * borderwidth, innerboxsize, innerboxsize), GetStyleColor("checked-inner-border-color"));
			canvas->fillRect(Rect::xywh(2.0 * borderwidth, center - 4.0 * borderwidth, checkedsize, checkedsize), GetStyleColor("checked-color"));
		}
	}
	else
	{
		if (auto image = GetStyleImage("unchecked-image"))
		{
			canvas->drawImage(image, Rect(0.0, center - s.height * 0.5 - GetStyleDouble("unchecked-align"), s.width, s.height));
		}
		else
		{
			canvas->fillRect(Rect::xywh(0.0, center - 6.0 * borderwidth, outerboxsize, outerboxsize), GetStyleColor("unchecked-outer-border-color"));
			canvas->fillRect(Rect::xywh(1.0 * borderwidth, center - 5.0 * borderwidth, innerboxsize, innerboxsize), GetStyleColor("unchecked-inner-border-color"));
		}
	}

	canvas->drawText(GetFont(), Point(s.width + 2.0, baseline), text, GetStyleColor("color"));
}

bool RadioButtonLabel::OnMouseDown(const Point& pos, InputKey key)
{
	mouseDownActive = true;
	SetFocus();
	return true;
}

bool RadioButtonLabel::OnMouseUp(const Point& pos, InputKey key)
{
	if (mouseDownActive)
	{
		Click();
	}
	mouseDownActive = false;
	return true;
}

void RadioButtonLabel::OnMouseLeave()
{
	mouseDownActive = false;
	SetStyleState("");
}

void RadioButtonLabel::OnMouseMove(const Point& Pos)
{
	if (GetStyleState().empty())
	{
		SetStyleState("hover");
	}
}

void RadioButtonLabel::OnKeyUp(InputKey key)
{
	if (key == InputKey::Space)
		Click();
}

void RadioButtonLabel::Click()
{
	if (!checked)
	{
		// Change state
		checked = true;
		if (group)
		{
			for (RadioButtonLabel* button : group->buttons)
			{
				if (button != this)
					button->checked = false;
			}
		}

		// Redraw screen
		Update();

		// Inform callbacks listening

		if (FuncChanged)
			FuncChanged(checked);

		if (group)
		{
			// Tell other buttons they are no longer checked
			for (RadioButtonLabel* button : group->buttons)
			{
				if (button != this && button->FuncChanged)
					button->FuncChanged(button->checked);
			}

			// Tell the group 
			if (group->FuncClicked)
				group->FuncClicked(this);
		}
	}
}
