
#include "Precomp.h"
#include "Element.h"
#include "Event.h"
#include "WindowFrame.h"
#include "Canvas.h"
#include "ComputedBorder.h"
#include "ElementStyle.h"

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
		style = nullptr;
		setNeedsLayout();
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
		style = nullptr;
		setNeedsLayout();
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
	if (name == "width" && value.size() > 2 && value.substr(value.size() - 2) == "px")
	{
		fixedWidth = std::atof(value.substr(0, value.size() - 2).c_str());
	}
	else if (name == "height" && value.size() > 2 && value.substr(value.size() - 2) == "px")
	{
		fixedHeight = std::atof(value.substr(0, value.size() - 2).c_str());
	}
	else if ((name == "padding-left" || name =="paddingLeft") && value.size() > 2 && value.substr(value.size() - 2) == "px")
	{
		paddingLeft = std::atof(value.substr(0, value.size() - 2).c_str());
	}
}

void Element::click()
{
	dispatchEvent("click");
}

void Element::focus()
{
}

void Element::dispatchEvent(std::string name, Event* e, bool bubbles)
{
	Element* currentTarget = this;
	while (true)
	{
		for (auto& handler : currentTarget->eventListeners[name])
		{
			handler(e);
			if (e->stopImmediatePropagationFlag)
				break;
		}

		currentTarget = currentTarget->parent();
		if (!bubbles || !currentTarget || e->stopPropagationFlag || e->stopImmediatePropagationFlag)
			break;
	}

	if (!e->preventDefaultFlag)
	{
		defaultAction(e);
	}
}

void Element::dispatchEvent(std::string name, bool bubbles)
{
	Event e;
	dispatchEvent(name, &e, bubbles);
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
	return toRootPos(geometry().contentBox().topLeft()).y;
}

double Element::clientLeft() const
{
	return toRootPos(geometry().contentBox().topLeft()).x;
}

double Element::clientWidth() const
{
	return geometry().contentWidth;
}

double Element::clientHeight() const
{
	return geometry().contentHeight;
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
	return geometry().scrollX;
}

double Element::scrollTop() const
{
	return geometry().scrollY;
}

double Element::scrollWidth() const
{
	return std::max(geometry().scrollWidth - geometry().contentWidth, 0.0);
}

double Element::scrollHeight() const
{
	return std::max(geometry().scrollHeight - geometry().contentHeight, 0.0);
}

void Element::scrollTo(double x, double y)
{
	double maxX = std::max(geometry().scrollWidth - geometry().contentWidth, 0.0);
	double maxY = std::max(geometry().scrollHeight - geometry().contentHeight, 0.0);
	double posX = std::max(std::min(x, maxX), 0.0);
	double posY = std::max(std::min(y, maxY), 0.0);
	if (posX != geometry().scrollX || posY != geometry().scrollY)
	{
		elementgeometry.scrollX = posX;
		elementgeometry.scrollY = posY;
		setNeedsRender();
	}
}

void Element::scrollBy(double x, double y)
{
	scrollTo(geometry().scrollX + x, geometry().scrollY + y);
}

void Element::setScrollLeft(double x)
{
	elementgeometry.scrollX = std::max(std::min(x, elementgeometry.scrollWidth - elementgeometry.contentWidth), 0.0);
}

void Element::setScrollTop(double y)
{
	elementgeometry.scrollY = std::max(std::min(y, elementgeometry.scrollHeight - elementgeometry.contentHeight), 0.0);
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

Element* Element::findElementAt(Point pos)
{
	pos = pos + geometry().scrollPos();
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
	needs_layout = true;
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

Point Element::toRootPos(const Point& pos) const
{
	if (parent())
		return parent()->toRootPos(geometry().contentBox().pos() + pos);
	else
		return pos;
}

Point Element::fromRootPos(const Point& pos) const
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
	layoutContent(canvas);
	renderStyle(canvas);
	renderScrollbar(canvas);

	bool needsClipping = overflow();
	if (needsClipping)
		canvas->pushClip(geometry().contentBox());

	Point origin = canvas->getOrigin();
	canvas->setOrigin(origin + geometry().contentBox().pos() - geometry().scrollPos());
	renderContent(canvas);
	canvas->setOrigin(origin);

	if (needsClipping)
		canvas->popClip();
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
	if (fixedWidth >= 0)
	{
		return fixedWidth;
	}
	else if (!innerText.empty())
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
			ComputedBorder border = element->computedBorder();
			w += element->preferredWidth(canvas) + border.left + border.right;
		}
		return w;
	}
}

double Element::preferredHeight(Canvas* canvas, double width)
{
	if (fixedHeight >= 0)
	{
		return fixedHeight;
	}
	else if (!innerText.empty())
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
			ComputedBorder border = element->computedBorder();
			h = std::max(element->preferredHeight(canvas, width) + border.top + border.bottom, h);
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

void Element::layoutContent(Canvas* canvas)
{
	if (needsLayout())
	{
		if (isClass("vbox"))
		{
			double width = geometry().contentWidth;

			double stretchheight = 0.0;
			if (!overflow())
			{
				double totalheight = 0;
				int expandingcount = 0;
				for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
				{
					ComputedBorder border = element->computedBorder();
					totalheight += element->preferredHeight(canvas, width - border.left - border.right) + border.top + border.bottom;
					if (element->isClass("expanding"))
						expandingcount++;
				}
				stretchheight = expandingcount > 0 ? (geometry().contentHeight - totalheight) / expandingcount : 0.0;
			}
			else
			{
				width -= scrollbarWidth();
			}

			double y = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				ComputedBorder border = element->computedBorder();

				ElementGeometry childpos;
				childpos.paddingLeft = border.left;
				childpos.paddingTop = border.top;
				childpos.paddingRight = border.right;
				childpos.paddingBottom = border.bottom;
				childpos.contentX = border.left;
				childpos.contentY = y + childpos.paddingTop;
				childpos.contentWidth = width - border.left - border.right;
				childpos.contentHeight = element->preferredHeight(canvas, childpos.contentWidth);
				if (element->isClass("expanding"))
					childpos.contentHeight = std::max(childpos.contentHeight + stretchheight, 0.0);
				element->setGeometry(childpos);

				y += childpos.contentHeight + childpos.paddingTop + childpos.paddingBottom;
			}

			elementgeometry.scrollWidth = width;
			elementgeometry.scrollHeight = y;
		}
		else
		{
			double height = geometry().contentHeight;

			double stretchwidth = 0.0;
			if (!overflow())
			{
				double totalwidth = 0;
				int expandingcount = 0;
				for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
				{
					ComputedBorder border = element->computedBorder();
					totalwidth += element->preferredWidth(canvas) + border.left + border.right;
					if (element->isClass("expanding"))
						expandingcount++;
				}
				stretchwidth = expandingcount > 0 ? (geometry().contentWidth - totalwidth) / expandingcount : 0.0;
			}
			else
			{
				height -= scrollbarWidth();
			}

			double x = 0.0;
			for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
			{
				ComputedBorder border = element->computedBorder();

				ElementGeometry childpos;
				childpos.paddingLeft = border.left;
				childpos.paddingTop = border.top;
				childpos.paddingRight = border.right;
				childpos.paddingBottom = border.bottom;
				childpos.contentX = x + border.left;
				childpos.contentY = border.top;
				childpos.contentWidth = element->preferredWidth(canvas);
				childpos.contentHeight = height - border.top - border.bottom;
				if (element->isClass("expanding"))
					childpos.contentWidth = std::max(childpos.contentWidth + stretchwidth, 0.0);
				element->setGeometry(childpos);

				x += childpos.contentWidth + childpos.paddingLeft + childpos.paddingRight;
			}

			elementgeometry.scrollWidth = x;
			elementgeometry.scrollHeight = height;
		}
		clearNeedsLayout();
	}
}

void Element::renderContent(Canvas* canvas)
{
	for (Element* element = firstChild(); element != nullptr; element = element->nextSibling())
	{
		element->render(canvas);
	}

	if (!innerText.empty())
	{
		Rect box = canvas->measureText(innerText);
		canvas->drawText({ 0.0f, (lineHeight() - box.height) * 0.5 }, color(), innerText);
	}
}

void Element::renderStyle(Canvas* canvas)
{
	Colorf bg = backgroundColor();
	if (bg.a != 0.0f)
	{
		canvas->fillRect(geometry().paddingBox(), bg);
	}
}

void Element::renderScrollbar(Canvas* canvas)
{
	if (overflow())
	{
		const ElementGeometry& g = geometry();
		Colorf trackBackground = Colorf::fromRgba8(250, 250, 250);
		Colorf thumbBackground = Colorf::fromRgba8(200, 200, 200);
		if (isClass("vbox"))
		{
			Rect scrollbarBox = { g.contentX + g.contentWidth - scrollbarWidth(), g.contentY, scrollbarWidth(), g.contentHeight };
			Rect trackBox = { scrollbarBox.x + thumbMargin(), scrollbarBox.y + thumbMargin(), scrollbarBox.width - (thumbMargin() * 2), scrollbarBox.height - (thumbMargin() * 2) };
			Rect thumbBox = trackBox;

			double scHeight = g.scrollHeight;
			if (scHeight > 0)
			{
				double scY = g.scrollY;
				double thumbTop = std::max(scY / scHeight, 0.0);
				double thumbBottom = std::max(std::min((scY + g.contentHeight) / scHeight, 1.0), thumbTop);
				thumbBox.y = trackBox.y + thumbTop * trackBox.height;
				thumbBox.height = (thumbBottom - thumbTop) * trackBox.height;
			}

			canvas->fillRect(scrollbarBox, trackBackground);
			canvas->fillRect(thumbBox, thumbBackground);
		}
		else if (isClass("hbox"))
		{
			Rect scrollbarBox = { g.contentX, g.contentY + g.contentHeight - scrollbarWidth(), g.contentWidth, scrollbarWidth() };
			Rect trackBox = { scrollbarBox.x + thumbMargin(), scrollbarBox.y + thumbMargin(), scrollbarBox.width - (thumbMargin() * 2), scrollbarBox.height - (thumbMargin() * 2) };
			Rect thumbBox = trackBox;

			double scWidth = g.scrollWidth;
			if (scWidth > 0)
			{
				double scX = g.scrollX;
				double thumbLeft = std::max(scX / scWidth, 0.0);
				double thumbRight = std::max(std::min((scX + g.contentWidth) / scWidth, 1.0), thumbLeft);
				thumbBox.x = trackBox.x + thumbLeft * trackBox.width;
				thumbBox.width = (thumbRight - thumbLeft) * trackBox.width;
			}

			canvas->fillRect(scrollbarBox, trackBackground);
			canvas->fillRect(thumbBox, thumbBackground);
		}
	}
}

ElementStyle* Element::activeStyle()
{
	if (style)
		return style;

	style = window()->getStyle(this);
	return style;
}
