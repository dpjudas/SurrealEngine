
#pragma once

#include "../../core/widget.h"

enum class TextLabelAlignment
{
	Left,
	Center,
	Right
};

enum class TextLabelVerticalAlignment
{
	Top,
	Center,
	Bottom
};

class TextLabel : public Widget
{
public:
	TextLabel(Widget* parent = nullptr);

	void SetText(const std::string& value);
	const std::string& GetText() const;

	void SetTextAlignment(TextLabelAlignment alignment);
	TextLabelAlignment GetTextAlignment() const;

	void SetTextVerticalAlignment(TextLabelVerticalAlignment alignment);
	TextLabelVerticalAlignment GetTextVerticalAlignment() const;

	double GetPreferredWidth() override;
	double GetPreferredHeight() override;
	double GetPreferredHeight(double width) override;

protected:
	void OnPaint(Canvas* canvas) override;

private:
	std::string text;
	TextLabelAlignment textAlignment = TextLabelAlignment::Left;
	TextLabelVerticalAlignment textVerticalAlignment = TextLabelVerticalAlignment::Center;
};
