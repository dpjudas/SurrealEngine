
#include "widgets/textlabel/textlabel.h"

TextLabel::TextLabel(Widget* parent) : Widget(parent)
{
}

void TextLabel::SetText(const std::string& value)
{
	if (text != value)
	{
		text = value;
		Update();
	}
}

const std::string& TextLabel::GetText() const
{
	return text;
}

void TextLabel::SetTextAlignment(TextLabelAlignment alignment)
{
	if (textAlignment != alignment)
	{
		textAlignment = alignment;
		Update();
	}
}

TextLabelAlignment TextLabel::GetTextAlignment() const
{
	return textAlignment;
}

double TextLabel::GetPreferredWidth()
{
	Canvas* canvas = GetCanvas();
	return canvas->measureText(text).width;
}

double TextLabel::GetPreferredHeight()
{
	return 20.0;
}

void TextLabel::OnPaint(Canvas* canvas)
{
	double x = 0.0;
	if (textAlignment == TextLabelAlignment::Center)
	{
		x = (GetWidth() - canvas->measureText(text).width) * 0.5;
	}
	else if (textAlignment == TextLabelAlignment::Right)
	{
		x = GetWidth() - canvas->measureText(text).width;
	}

	FontMetrics metrics = canvas->getFontMetrics();
	canvas->drawText(Point(x, (GetHeight() - metrics.height) * 0.5 + metrics.ascent), GetStyleColor("color"), text);
}
