
#include "Precomp.h"
#include "Element.h"
#include "Event.h"
#include "WindowFrame.h"
#include "Canvas.h"
#include "ComputedBorder.h"

std::unique_ptr<Element> Element::createElement(std::string elementType)
{
	return std::make_unique<Element>();
}

std::unique_ptr<Element> Element::createElementNS(std::string ns, std::string elementType)
{
	return createElement(elementType);
}

Element::Element()
{
}

Element::~Element()
{
}

void Element::appendChild(Element* element)
{
	element->setParent(this);
	setNeedsLayout();
}

void Element::insertBefore(Element* newNode, Element* referenceNode)
{
	newNode->setParent(this);
	newNode->moveBefore(referenceNode);
	setNeedsLayout();
}

void Element::removeChild(Element* element)
{
	element->detachFromParent();
	setNeedsLayout();
}

void Element::setAttribute(std::string name, std::string value)
{
	if (name == "class")
	{
		classes.clear();
		size_t pos = 0;
		while (pos < value.size())
		{
			pos = value.find_first_not_of(" \t", pos);
			if (pos == std::string::npos)
				break;

			size_t pos2 = value.find_first_of(" \t", pos);
			if (pos2 == std::string::npos)
				pos2 = value.size();

			if (pos < pos2)
				classes.insert(value.substr(pos, pos2 - pos));

			pos = pos2;
		}
	}
	else
	{
		attributes[name] = value;
	}
	setNeedsLayout();
}

void Element::removeAttribute(std::string name)
{
	if (name == "class")
	{
		classes.clear();
	}
	else
	{
		auto it = attributes.find(name);
		if (it != attributes.end())
			attributes.erase(it);
	}
	setNeedsLayout();
}

void Element::setStyle(std::string name, std::string value)
{
}

void Element::click()
{
	dispatchEvent("click");
}

void Element::focus()
{
}

void Element::dispatchEvent(std::string name, bool bubbles)
{
	Event e;
	for (auto& handler : eventListeners[name])
	{
		handler(&e);
		if (e.stopPropagationFlag)
			break;
	}
	if (!e.preventDefaultFlag)
	{
		defaultAction(&e);
	}
}

void Element::addEventListener(std::string name, std::function<void(Event* event)> handler)
{
	eventListeners[name].push_back(std::move(handler));
}

Rect Element::getBoundingClientRect()
{
	Rect box = geometry().contentBox();
	Point tl = toRootPos(box.topLeft());
	Point br = toRootPos(box.bottomRight());
	return Rect::ltrb(tl.x, tl.y, br.x, br.y);
}

double Element::clientTop() const
{
	return 0;
}

double Element::clientLeft() const
{
	return 0;
}

double Element::clientWidth() const
{
	return 0;
}

double Element::clientHeight() const
{
	return 0;
}

double Element::offsetLeft() const
{
	return 0;
}

double Element::offsetTop() const
{
	return 0;
}

double Element::offsetWidth() const
{
	return 0;
}

double Element::offsetHeight() const
{
	return 0;
}

double Element::scrollLeft() const
{
	return 0;
}

double Element::scrollTop() const
{
	return 0;
}

double Element::scrollWidth() const
{
	return 0;
}

double Element::scrollHeight() const
{
	return 0;
}

void Element::scrollTo(double x, double y)
{
}

void Element::scrollBy(double x, double y)
{
}

void Element::setScrollLeft(double x)
{
}

void Element::setScrollTop(double y)
{
}

std::string Element::getValue()
{
	return {};
}

void Element::setInnerText(const std::string& text)
{
	innerText = text;
	setNeedsLayout();
}

void Element::setInnerHTML(const std::string& text)
{
}

std::string Element::getInnerText()
{
	return std::string();
}

std::string Element::getInnerHTML()
{
	return std::string();
}

Element* Element::findElementAt(const Point& pos)
{
	for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
	{
		if (element->geometry().borderBox().contains(pos))
		{
			return element->findElementAt(pos - element->geometry().contentPos());
		}
	}
	return this;
}

bool Element::needsLayout() const
{
	return needs_layout;
}

void Element::clearNeedsLayout()
{
	needs_layout = false;
}

void Element::setNeedsLayout()
{
	Element* super = parent();
	if (super)
		super->setNeedsLayout();
	else
		setNeedsRender();
}

const ElementGeometry& Element::geometry() const
{
	return elementgeometry;
}

void Element::setGeometry(const ElementGeometry& geometry)
{
	if (elementgeometry.contentBox() != geometry.contentBox())
	{
		elementgeometry = geometry;
		setNeedsLayout();
	}
}

void Element::setNeedsRender()
{
	WindowFrame* w = window();
	if (w)
		w->setNeedsRender();
}

Point Element::toRootPos(const Point& pos)
{
	if (parent())
		return parent()->toRootPos(geometry().contentBox().pos() + pos);
	else
		return pos;
}

Point Element::fromRootPos(const Point& pos)
{
	if (parent())
		return parent()->fromRootPos(pos) - geometry().contentBox().pos();
	else
		return pos;
}

const WindowFrame* Element::window() const
{
	Element* super = parent();
	if (super)
		return super->window();
	else
		return viewwindow;
}

WindowFrame* Element::window()
{
	Element* super = parent();
	if (super)
		return super->window();
	else
		return viewwindow;
}

void Element::render(Canvas* canvas)
{
	renderStyle(canvas);
	Point origin = canvas->getOrigin();
	canvas->setOrigin(origin + geometry().contentBox().pos());
	renderContent(canvas);
	canvas->setOrigin(origin);
}

ComputedBorder Element::computedBorder()
{
	ComputedBorder border;
	border.left = 5;
	border.top = 5;
	border.right = 5;
	border.bottom = 5;
	return border;
}

void Element::renderStyle(Canvas* canvas)
{
	canvas->fillRect(geometry().paddingBox(), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));
}

void Element::setParent(Element* newParent)
{
	if (parentObj != newParent)
	{
		if (parentObj)
			detachFromParent();

		if (newParent)
		{
			prevSiblingObj = newParent->lastChildObj;
			if (prevSiblingObj) prevSiblingObj->nextSiblingObj = this;
			newParent->lastChildObj = this;
			if (!newParent->firstChildObj) newParent->firstChildObj = this;
			parentObj = newParent;
		}
	}
}

void Element::moveBefore(Element* sibling)
{
	if (sibling && sibling->parentObj != parentObj) throw std::runtime_error("Invalid sibling passed to Element.moveBefore");
	if (!parentObj) throw std::runtime_error("View must have a parent before it can be moved");

	if (nextSiblingObj != sibling)
	{
		Element* p = parentObj;
		detachFromParent();

		parentObj = p;
		if (sibling)
		{
			nextSiblingObj = sibling;
			prevSiblingObj = sibling->prevSiblingObj;
			sibling->prevSiblingObj = this;
			if (prevSiblingObj) prevSiblingObj->nextSiblingObj = this;
			if (parentObj->firstChildObj == sibling) parentObj->firstChildObj = this;
		}
		else
		{
			prevSiblingObj = parentObj->lastChildObj;
			if (prevSiblingObj) prevSiblingObj->nextSiblingObj = this;
			parentObj->lastChildObj = this;
			if (!parentObj->firstChildObj) parentObj->firstChildObj = this;
		}
	}
}

void Element::detachFromParent()
{
	if (prevSiblingObj)
		prevSiblingObj->nextSiblingObj = nextSiblingObj;
	if (nextSiblingObj)
		nextSiblingObj->prevSiblingObj = prevSiblingObj;
	if (parentObj)
	{
		if (parentObj->firstChildObj == this)
			parentObj->firstChildObj = nextSiblingObj;
		if (parentObj->lastChildObj == this)
			parentObj->lastChildObj = prevSiblingObj;
	}
	prevSiblingObj = nullptr;
	nextSiblingObj = nullptr;
	parentObj = nullptr;
}

double Element::preferredWidth(Canvas* canvas)
{
	if (!innerText.empty())
	{
		return canvas->measureText(innerText).width;
	}
	else if (isClass("vbox"))
	{
		double w = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			ComputedBorder border = element->computedBorder();
			w = std::max(element->preferredWidth(canvas) + border.left + border.right, w);
		}
		return w;
	}
	else
	{
		double w = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			w += element->preferredWidth(canvas);
		}
		return w;
	}
}

double Element::preferredHeight(Canvas* canvas, double width)
{
	if (!innerText.empty())
	{
		return lineHeight();
	}
	else if (isClass("vbox"))
	{
		double h = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			ComputedBorder border = element->computedBorder();
			h += element->preferredHeight(canvas, width - border.left - border.right) + border.top + border.bottom;
		}
		return h;
	}
	else
	{
		double h = 0.0;
		for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
		{
			h = std::max(element->preferredHeight(canvas, width), h);
		}
		return h;
	}
}

double Element::firstBaselineOffset(Canvas* canvas, double width)
{
	if (isClass("vbox"))
	{
		if (firstChild())
		{
			ComputedBorder border = firstChild()->computedBorder();
			return firstChild()->firstBaselineOffset(canvas, width) + border.top;
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if (firstChild())
			return firstChild()->firstBaselineOffset(canvas, width);
		else
			return 0.0f;
	}
}

double Element::lastBaselineOffset(Canvas* canvas, double width)
{
	if (isClass("vbox"))
	{
		if (firstChild())
		{
			double h = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				if (element != lastChild())
				{
					ComputedBorder border = element->computedBorder();
					h += element->preferredHeight(canvas, width) + border.top + border.bottom;
				}
			}
			return h + lastChild()->lastBaselineOffset(canvas, width);
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if (firstChild())
		{
			double h = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				if (element != lastChild())
					h += element->preferredHeight(canvas, width);
			}
			return h + lastChild()->lastBaselineOffset(canvas, width);
		}
		else
		{
			return 0.0f;
		}
	}
}

void Element::renderContent(Canvas* canvas)
{
	if (needsLayout())
	{
		if (isClass("vbox"))
		{
			double width = geometry().contentWidth;
			double y = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				ComputedBorder border = element->computedBorder();

				ElementGeometry childpos;
				childpos.borderLeft = border.left;
				childpos.borderTop = border.top;
				childpos.borderRight = border.right;
				childpos.borderBottom = border.bottom;
				childpos.contentX = border.left;
				childpos.contentY = y + childpos.borderTop;
				childpos.contentWidth = std::min(element->preferredWidth(canvas), width - border.left - border.right);
				childpos.contentHeight = element->preferredHeight(canvas, childpos.contentWidth);
				element->setGeometry(childpos);

				y += childpos.contentHeight + childpos.borderTop + childpos.borderBottom;
			}
		}
		else
		{
			double height = geometry().contentHeight;
			double x = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				ComputedBorder border = element->computedBorder();

				ElementGeometry childpos;
				childpos.borderLeft = border.left;
				childpos.borderTop = border.top;
				childpos.borderRight = border.right;
				childpos.borderBottom = border.bottom;
				childpos.contentX = x + border.left;
				childpos.contentY = border.top;
				childpos.contentWidth = element->preferredWidth(canvas);
				childpos.contentHeight = element->preferredHeight(canvas, childpos.contentWidth);
				element->setGeometry(childpos);

				x += childpos.contentWidth + childpos.borderLeft + childpos.borderRight;
			}
		}
		clearNeedsLayout();
	}

	for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
	{
		element->render(canvas);
	}

	if (!innerText.empty())
	{
		canvas->drawText({ 0.0f, 0.0f }, color(), innerText);
	}
}
