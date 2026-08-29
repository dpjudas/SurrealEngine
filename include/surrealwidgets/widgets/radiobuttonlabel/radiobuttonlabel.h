
#pragma once

#include "../../core/widget.h"
#include <vector>

class RadioButtonLabel;

class RadioGroup
{
public:
	std::function<void(RadioButtonLabel* button)> FuncClicked;

private:
	std::vector<RadioButtonLabel*> buttons;

	friend class RadioButtonLabel;
};

class RadioButtonLabel : public Widget
{
public:
	RadioButtonLabel(RadioGroup* group = nullptr, Widget* parent = nullptr);

	void SetGroup(RadioGroup* group);

	void SetText(const std::string& value);
	const std::string& GetText() const;

	void SetChecked(bool value);
	bool GetChecked() const;

	void Click();

	double GetPreferredWidth() override;
	double GetPreferredHeight() override;

	std::function<void(bool)> FuncChanged;

protected:
	void OnPaint(Canvas* canvas) override;
	bool OnMouseDown(const Point& pos, InputKey key) override;
	bool OnMouseUp(const Point& pos, InputKey key) override;
	void OnMouseLeave() override;
	void OnKeyUp(InputKey key) override;
	void OnMouseMove(const Point& pos) override;

private:
	Size GetRadioButtonSize();

	RadioGroup* group = nullptr;
	std::string text;
	bool checked = false;
	bool mouseDownActive = false;
};
