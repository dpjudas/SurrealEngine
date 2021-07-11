
#include "Precomp.h"
#include "Dialog.h"

Dialog::Dialog(const std::string& title) : VBoxView(nullptr)
{
	setupUi();
	setTitle(title);
}

void Dialog::setupUi()
{
	addClass("dialog");
	header = new DialogHeader(this);
	header->addClass("dialog-header");
	centerView = new VBoxView(this);
	centerView->addClass("dialog-centerview");
	centerView->setExpanding();
	buttonbar = new DialogButtonbar(this);
	buttonbar->addClass("dialog-buttonbar");
	element->addEventListener("click", [](Event* event) { event->stopPropagation(); });
	setSize(500, 250);
}

void Dialog::setTitle(std::string text)
{
	header->caption->setText(text);
}

void Dialog::setSize(double width, double height, bool fixedHeight)
{
	element->setStyle("left", "calc(50vw - " + std::to_string(std::round(width * 0.5)) + "px)");
	element->setStyle("top", "calc(50vh - " + std::to_string(std::round(height * 0.5)) + "px)");
	element->setStyle("width", std::to_string(width) + "px");
	if (fixedHeight)
	{
		element->setStyle("height", std::to_string(height) + "px");
	}
}

void Dialog::onClose(Event* event)
{
	closeModal();
}
