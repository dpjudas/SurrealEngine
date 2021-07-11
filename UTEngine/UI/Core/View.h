#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include "Element/Element.h"

class View
{
public:
	View(View* parent, std::string elementType = "div");
	View(View* parent, std::string ns, std::string elementType);
	View(View* parent, std::unique_ptr<Element> element);
	virtual ~View();

	void setParent(View* parent);
	void moveBefore(View* sibling);

	void addClass(std::string name);
	void removeClass(std::string name);

	void setVBoxLayout();
	void setHBoxLayout();
	void setExpanding();

	void show();
	void hide();

	void showModal();
	void closeModal();

	virtual void onAttach() { }
	virtual void onDetach() { }

	std::unique_ptr<Element> element;

	View* parent() const { return parentObj; }
	View* prevSibling() const { return prevSiblingObj; }
	View* nextSibling() const { return nextSiblingObj; }
	View* firstChild() const { return firstChildObj; }
	View* lastChild() const { return lastChildObj; }

private:
	void detachFromParent();
	void updateClassAttribute();

	View* parentObj = nullptr;
	View* prevSiblingObj = nullptr;
	View* nextSiblingObj = nullptr;
	View* firstChildObj = nullptr;
	View* lastChildObj = nullptr;

	std::set<std::string> classes;
};

class VBoxView : public View
{
public:
	VBoxView(View* parent) : View(parent)
	{
		addClass("vbox");
	}
};

class HBoxView : public View
{
public:
	HBoxView(View* parent) : View(parent)
	{
		addClass("hbox");
	}
};
