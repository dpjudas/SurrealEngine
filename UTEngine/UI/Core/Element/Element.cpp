
#include "Precomp.h"
#include "Element.h"
#include "Event.h"
#include "WindowFrame.h"
#include "Canvas.h"
#include "BoxElement.h"
#include "ComputedBorder.h"

std::unique_ptr<Element> Element::createElement(std::string elementType)
{
	return std::make_unique<BoxElement>();
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
}

void Element::insertBefore(Element* newNode, Element* referenceNode)
{
}

void Element::removeChild(Element* element)
{
}

void Element::setAttribute(std::string name, std::string value)
{
	attributes[name] = value;
}

void Element::removeAttribute(std::string name)
{
	auto it = attributes.find(name);
	if (it != attributes.end())
		attributes.erase(it);
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

Element* Element::findElementAt(const Point& pos) const
{
	for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
	{
		if (element->geometry().borderBox().contains(pos))
		{
			return element->findElementAt(pos - element->geometry().contentPos());
		}
	}
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
	return {};
}

void Element::renderStyle(Canvas* canvas)
{
}
