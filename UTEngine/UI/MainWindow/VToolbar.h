#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/TextLabel/TextLabel.h"
#include "UI/Controls/ImageBox/ImageBox.h"

class VToolbarButton;

class VToolbar : public VBoxView
{
public:
	VToolbar(View* parent);

	VToolbarButton* addButton(std::string img, std::string text, std::function<void()> onClick);
};

class VToolbarButton : public VBoxView
{
public:
	VToolbarButton(View* parent) : VBoxView(parent)
	{
		icon = new ImageBox(this);
		text = new TextLabel(this);

		addClass("vtoolbarbutton");
		icon->addClass("vtoolbarbutton-icon");
		text->addClass("vtoolbarbutton-text");

		element->addEventListener("mouseenter", [=](Event* event) { onMouseEnter(event); });
		element->addEventListener("mouseleave", [=](Event* event) { onMouseLeave(event); });
	}

	void onMouseEnter(Event* event)
	{
		element->setStyle("background", "rgb(89,134,247)");
	}

	void onMouseLeave(Event* event)
	{
		element->setStyle("background", "none");
	}

	ImageBox* icon = nullptr;
	TextLabel* text = nullptr;
};
