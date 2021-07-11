#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/TextLabel/TextLabel.h"
#include "DockHeaderButton.h"

class TextLabel;
class DockHeaderButton;

class DockHeader : public HBoxView
{
public:
	DockHeader(View* parent, std::function<void()> onClose);

	TextLabel* caption = nullptr;
	DockHeaderButton* closeButton = nullptr;

private:
	static const std::string CloseIconSvg;
};
