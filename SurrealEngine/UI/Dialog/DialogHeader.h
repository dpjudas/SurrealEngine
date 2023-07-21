#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/TextLabel/TextLabel.h"
#include "DialogHeaderButton.h"

class TextLabel;
class DialogHeaderButton;

class DialogHeader : public HBoxView
{
public:
	DialogHeader(View* parent);

	TextLabel* caption = nullptr;
	DialogHeaderButton* closeButton = nullptr;

	static const std::string CloseIconSvg;
	static const std::string MaximizedIconSvg;
	static const std::string MaximizeIconSvg;
	static const std::string MinimizeIconSvg;
};
