
#include "Precomp.h"
#include "TextLabel.h"

TextLabel::TextLabel(View* parent) : View(parent)
{
	addClass("textlabel");
}

void TextLabel::setCenterAlign()
{
	element->setStyle("textAlign", "center");
}

void TextLabel::setText(std::string text)
{
	element->setInnerText(text);
}

std::string TextLabel::getText()
{
	return element->getInnerText();
}

void TextLabel::setHtml(std::string html)
{
	element->setInnerHTML(html);
}

std::string TextLabel::getHtml()
{
	return element->getInnerHTML();
}

void TextLabel::setClickHandler(const std::function<void()>& handler)
{
	element->addEventListener("click", [=](Event* e) { e->stopPropagation(); handler(); });
}
