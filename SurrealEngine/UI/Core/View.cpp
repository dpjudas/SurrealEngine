
#include "Precomp.h"
#include "View.h"
#include <stdexcept>

View::View(View* parent, std::string elementType)
{
	element = Element::createElement(elementType);
	addClass("view");
	setParent(parent);
}

View::View(View* parent, std::string ns, std::string elementType)
{
	element = Element::createElementNS(ns, elementType);
	element->setAttribute("xmlns", ns);
	addClass("view");
	setParent(parent);
}

View::View(View* parent, std::unique_ptr<Element> element) : element(std::move(element))
{
	addClass("view");
	setParent(parent);
}

View::~View()
{
	while (lastChildObj)
		delete lastChildObj;

	detachFromParent();
}

void View::setParent(View* newParent)
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

			parentObj->element->appendChild(element.get());

			onAttach(); // To do: only call this when attaching to the root, but then do it recursively
		}
	}
}

void View::moveBefore(View* sibling)
{
	if (sibling && sibling->parentObj != parentObj) throw std::runtime_error("Invalid sibling passed to View.moveBefore");
	if (!parentObj) throw std::runtime_error("View must have a parent before it can be moved");

	if (nextSiblingObj != sibling)
	{
		View* p = parentObj;
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

		if (nextSiblingObj)
			parentObj->element->insertBefore(element.get(), nextSiblingObj->element.get());
		else
			parentObj->element->appendChild(element.get());

		onAttach(); // To do: only call this when attaching to the root, but then do it recursively
	}
}

void View::detachFromParent()
{
	if (parentObj)
	{
		onDetach();
		parentObj->element->removeChild(element.get());
	}

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

void View::addClass(std::string name)
{
	classes.insert(name);
	updateClassAttribute();
}

void View::removeClass(std::string name)
{
	classes.erase(name);
	updateClassAttribute();
}

void View::updateClassAttribute()
{
	std::string attr;
	for (const std::string& cls : classes)
	{
		if (!attr.empty()) attr += " ";
		attr += cls;
	}
	element->setAttribute("class", attr);
}

void View::setVBoxLayout()
{
	element->setStyle("display", "flex");
	element->setStyle("flexDirection", "column");
}

void View::setHBoxLayout()
{
	element->setStyle("display", "flex");
	element->setStyle("flexDirection", "row");
}

void View::setExpanding()
{
	addClass("expanding");
}

void View::show()
{
	removeClass("hidden");
}

void View::hide()
{
	addClass("hidden");
}

void View::showModal()
{
#ifdef __EMSCRIPTEN__
	ModalLayer* layer = DocumentBodyView::get()->beginModal();
	setParent(layer);
#endif
}

void View::closeModal()
{
#ifdef __EMSCRIPTEN__
	setParent(nullptr);
	DocumentBodyView::get()->endModal();
	delete this;
#endif
}
