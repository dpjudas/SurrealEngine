
#include "widgets/textlabel/textlabel.h"
#include "core/span_layout.h"

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
	SpanLayout spanlayout;
	spanlayout.AddText(text, GetFont());
	return spanlayout.FindPreferredSize(GetCanvas()).width + 1.0;
}

double TextLabel::GetPreferredHeight()
{
	SpanLayout spanlayout;
	spanlayout.AddText(text, GetFont());
	return spanlayout.FindPreferredSize(GetCanvas()).height + 1.0;
}

double TextLabel::GetPreferredHeight(double width)
{
	SpanLayout spanlayout;
	spanlayout.AddText(text, GetFont());
	spanlayout.Layout(GetCanvas(), width);
	return spanlayout.GetSize().height + 1.0;
}

void TextLabel::OnPaint(Canvas* canvas)
{
	/*
	double x = 0.0;
	if (textAlignment == TextLabelAlignment::Center)
	{
		x = (GetWidth() - canvas->measureText(GetFont(), text).width) * 0.5;
	}
	else if (textAlignment == TextLabelAlignment::Right)
	{
		x = GetWidth() - canvas->measureText(GetFont(), text).width;
	}

	FontMetrics metrics = canvas->getFontMetrics(GetFont());
	canvas->drawText(GetFont(), Point(x, (GetHeight() - metrics.height) * 0.5 + metrics.ascent), text, GetStyleColor("color"));
	*/

	SpanAlign align[3] = { SpanAlign::span_left, SpanAlign::span_center, SpanAlign::span_right };

	SpanLayout spanlayout;
	spanlayout.AddText(text, GetFont(), GetStyleColor("color"));
	spanlayout.Layout(GetCanvas(), GetWidth());
	spanlayout.SetAlign(align[(int)textAlignment]);
	spanlayout.SetPosition(Point(0.0, 0.0/*(GetHeight() - spanlayout.GetSize().height) * 0.5 */));
	spanlayout.DrawLayout(canvas);
}
