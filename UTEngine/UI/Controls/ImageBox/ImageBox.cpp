
#include "Precomp.h"
#include "ImageBox.h"

ImageBox::ImageBox(View* parent) : View(parent, "img")
{
	addClass("imagebox");
	element->setStyle("visibility", "hidden");
}

void ImageBox::setSrc(std::string src)
{
	if (!src.empty())
	{
		element->setAttribute("src", src);
		element->setStyle("visibility", "inherit");
	}
	else
	{
		element->removeAttribute("src");
		element->setStyle("visibility", "hidden");
	}
}

void ImageBox::setSize(const int width, const int height)
{
	element->setStyle("width", std::to_string(width) + "px");
	element->setStyle("height", std::to_string(height) + "px");
}

void ImageBox::setClickHandler(const std::function<void()>& handler)
{
	element->addEventListener("click", [=](Event* e) { e->stopPropagation(); handler(); });
}
