
#include "Precomp.h"
#include "Toolbar.h"

Toolbar::Toolbar(View* parent) : HBoxView(parent)
{
	addClass("toolbar");
}

ToolbarButton* Toolbar::addButton(std::string img, std::string text, std::function<void()> onClick)
{
	auto button = new ToolbarButton(this);
	button->addClass("toolbar-button");
	button->icon->setSrc(img);
	button->text->setText(text);
	if (onClick)
	{
		button->element->addEventListener("click", [=](Event* event) { event->stopPropagation(); if (button->getEnabled()) onClick(); });
	}
	return button;
}

ToolbarSeparator* Toolbar::addSeparator()
{
	auto sep = new ToolbarSeparator(this);
	sep->addClass("toolbar-sep");
	return sep;
}

#ifdef __EMSCRIPTEN__
ToolbarDropdown* Toolbar::addDropdown(std::string label, std::vector<std::string> options)
{
	auto dropdown = new ToolbarDropdown(this);
	dropdown->addClass("toolbar-dropdown");
	dropdown->label->setText(label);
	for (const std::string& option : options)
		dropdown->combobox->addItem(option);
	return dropdown;
}
#endif

ToolbarTextInput* Toolbar::addTextInput(std::string label, std::string text)
{
	auto input = new ToolbarTextInput(this);
	input->addClass("toolbar-input");
	input->label->setText(label);
	input->edit->setText(text);
	return input;
}

ToolbarButton* Toolbar::addRadioButton(std::string img)
{
	auto button = new ToolbarButton(this);
	button->addClass("toolbar-radiobutton");
	button->icon->setSrc(img);
	return button;
}

void Toolbar::addView(View* view)
{
	view->setParent(this);
	view->addClass("toolbar-view");
}
