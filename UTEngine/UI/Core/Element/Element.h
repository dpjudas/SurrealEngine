#pragma once

#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <set>
#include <map>
#include "Event.h"
#include "Rect.h"
#include "ElementGeometry.h"

class Canvas;
class WindowFrame;
class ComputedBorder;

class Colorf
{
public:
	Colorf() = default;
	Colorf(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) { }

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
};

class Element
{
public:
	static std::unique_ptr<Element> createElement(std::string elementType);
	static std::unique_ptr<Element> createElementNS(std::string ns, std::string elementType);

	Element();
	virtual ~Element();

	void appendChild(Element* element);
	void insertBefore(Element* newNode, Element* referenceNode);
	void removeChild(Element* element);

	void setAttribute(std::string name, std::string value);
	void removeAttribute(std::string name);

	void setStyle(std::string name, std::string value);

	void addEventListener(std::string name, std::function<void(Event* event)> handler);
	void dispatchEvent(std::string name, bool bubbles = true);
	void dispatchEvent(std::string name, Event* event, bool bubbles = true);

	void click();
	void focus();
	void setTabIndex(int index) { setAttribute("tabindex", std::to_string(index)); }
	void removeTabIndex() { removeAttribute("tabindex"); }

	std::string getValue();

	void setInnerText(const std::string& text);
	void setInnerHTML(const std::string& text);
	std::string getInnerText();
	std::string getInnerHTML();

	Rect getBoundingClientRect();

	double clientTop() const;
	double clientLeft() const;
	double clientWidth() const;
	double clientHeight() const;

	double offsetLeft() const;
	double offsetTop() const;
	double offsetWidth() const;
	double offsetHeight() const;

	double scrollLeft() const;
	double scrollTop() const;
	double scrollWidth() const;
	double scrollHeight() const;

	void scrollTo(double x, double y);
	void scrollBy(double x, double y);
	void setScrollLeft(double x);
	void setScrollTop(double y);

	//

	virtual void defaultAction(Event* event) { }
	virtual double preferredWidth(Canvas* canvas);
	virtual double preferredHeight(Canvas* canvas, double width);
	virtual double firstBaselineOffset(Canvas* canvas, double width);
	virtual double lastBaselineOffset(Canvas* canvas, double width);
	virtual void renderContent(Canvas* canvas);

	Element* parent() const { return parentObj; }
	Element* prevSibling() const { return prevSiblingObj; }
	Element* nextSibling() const { return nextSiblingObj; }
	Element* firstChild() const { return firstChildObj; }
	Element* lastChild() const { return lastChildObj; }

	Element* findElementAt(const Point& pos);
	bool needsLayout() const;
	void clearNeedsLayout();
	void setNeedsLayout();
	const ElementGeometry& geometry() const;
	void setGeometry(const ElementGeometry& geometry);
	void setNeedsRender();
	Point toRootPos(const Point& pos);
	Point fromRootPos(const Point& pos);
	const WindowFrame* window() const;
	WindowFrame* window();
	void render(Canvas* canvas);

	void renderStyle(Canvas* canvas);
	ComputedBorder computedBorder();
	bool overflow();
	Colorf color();
	double lineHeight();

	bool isClass(const char* cls) { return classes.find(cls) != classes.end(); }
	double fixedWidth = -1;
	double fixedHeight = -1;
	double paddingLeft = -1;

private:
	void layoutContent(Canvas* canvas);
	void setParent(Element* newParent);
	void moveBefore(Element* sibling);
	void detachFromParent();

	WindowFrame* viewwindow = nullptr;
	bool needs_layout = true;
	bool needs_render = true;
	ElementGeometry elementgeometry;
	std::map<std::string, std::vector<std::function<void(Event* event)>>> eventListeners;
	std::map<std::string, std::string> attributes;
	std::set<std::string> classes;
	std::string innerText;

	Element* parentObj = nullptr;
	Element* prevSiblingObj = nullptr;
	Element* nextSiblingObj = nullptr;
	Element* firstChildObj = nullptr;
	Element* lastChildObj = nullptr;

	friend class WindowFrame;
	friend class WindowFrameImpl;
};
