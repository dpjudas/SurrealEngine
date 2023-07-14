
#include "Precomp.h"
#include "BorderLayoutView.h"

BorderLayoutView::BorderLayoutView(View* parent) : View(parent)
{
	vbox = new VBoxView(this);
	topSplitter = new BorderLayoutSplitter(BorderLayoutSplitterPos::Top, vbox);
	centerRow = new HBoxView(vbox);
	leftSplitter = new BorderLayoutSplitter(BorderLayoutSplitterPos::Left, centerRow);
	rightSplitter = new BorderLayoutSplitter(BorderLayoutSplitterPos::Right, centerRow);
	bottomSplitter = new BorderLayoutSplitter(BorderLayoutSplitterPos::Bottom, vbox);

	setExpanding();
	centerRow->setExpanding();

	addClass("borderlayout");
	vbox->addClass("borderlayout-vbox");
	centerRow->addClass("borderlayout-centerrow");

	topSplitter->hide();
	leftSplitter->hide();
	rightSplitter->hide();
	bottomSplitter->hide();
}

void BorderLayoutView::showTopView()
{
	if (top)
	{
		top->show();
		topSplitter->show();
	}
}

void BorderLayoutView::showLeftView()
{
	if (left)
	{
		left->show();
		leftSplitter->show();
	}
}

void BorderLayoutView::showRightView()
{
	if (right)
	{
		right->show();
		rightSplitter->show();
	}
}

void BorderLayoutView::showBottomView()
{
	if (bottom)
	{
		bottom->show();
		bottomSplitter->show();
	}
}

void BorderLayoutView::hideTopView()
{
	if (top)
	{
		top->hide();
		topSplitter->hide();
	}
}

void BorderLayoutView::hideLeftView()
{
	if (left)
	{
		left->hide();
		leftSplitter->hide();
	}
}

void BorderLayoutView::hideRightView()
{
	if (right)
	{
		right->hide();
		rightSplitter->hide();
	}
}

void BorderLayoutView::hideBottomView()
{
	if (bottom)
	{
		bottom->hide();
		bottomSplitter->hide();
	}
}

void BorderLayoutView::setTopView(View* view, double height)
{
	delete top;
	top = view;
	if (top)
	{
		top->setParent(vbox);
		top->moveBefore(topSplitter);
		top->element->setStyle("height", std::to_string(height) + "px");
		topSplitter->show();
		topSplitter->size = height;
	}
	else
	{
		topSplitter->hide();
	}
}

void BorderLayoutView::setLeftView(View* view, double width)
{
	delete left;
	left = view;
	if (left)
	{
		left->setParent(centerRow);
		left->moveBefore(leftSplitter);
		left->element->setStyle("width", std::to_string(width) + "px");
		leftSplitter->show();
		leftSplitter->size = width;
	}
	else
	{
		leftSplitter->hide();
	}
}

void BorderLayoutView::setCenterView(View* view)
{
	delete center;
	center = view;
	if (center)
	{
		center->setParent(centerRow);
		center->moveBefore(rightSplitter);
		center->setExpanding();
	}
}

void BorderLayoutView::setRightView(View* view, double width)
{
	delete right;
	right = view;
	if (right)
	{
		right->setParent(centerRow);
		right->moveBefore(nullptr);
		right->element->setStyle("width", std::to_string(width) + "px");
		rightSplitter->show();
		rightSplitter->size = width;
	}
	else
	{
		rightSplitter->hide();
	}
}

void BorderLayoutView::setBottomView(View* view, double height)
{
	delete bottom;
	bottom = view;
	if (bottom)
	{
		bottom->setParent(vbox);
		bottom->moveBefore(nullptr);
		bottom->element->setStyle("height", std::to_string(height) + "px");
		bottomSplitter->show();
		bottomSplitter->size = height;
	}
	else
	{
		bottomSplitter->hide();
	}
}

View* BorderLayoutView::takeTopView()
{
	View* view = top;
	top = nullptr;
	if (view)
	{
		view->setParent(nullptr);
		topSplitter->hide();
	}
	return view;
}

View* BorderLayoutView::takeLeftView()
{
	View* view = left;
	left = nullptr;
	if (view)
	{
		view->setParent(nullptr);
		leftSplitter->hide();
	}
	return view;
}

View* BorderLayoutView::takeCenterView()
{
	View* view = center;
	center = nullptr;
	if (view)
		view->setParent(nullptr);
	return view;
}

View* BorderLayoutView::takeRightView()
{
	View* view = right;
	right = nullptr;
	if (view)
	{
		view->setParent(nullptr);
		rightSplitter->hide();
	}
	return view;
}

View* BorderLayoutView::takeBottomView()
{
	View* view = bottom;
	bottom = nullptr;
	if (view)
	{
		view->setParent(nullptr);
		bottomSplitter->hide();
	}
	return view;
}

/////////////////////////////////////////////////////////////////////////////

BorderLayoutSplitter::BorderLayoutSplitter(BorderLayoutSplitterPos pos, View* parent) : View(parent), pos(pos)
{
	if (pos == BorderLayoutSplitterPos::Top || pos == BorderLayoutSplitterPos::Bottom)
		addClass("vborderlayoutsplitter");
	else
		addClass("borderlayoutsplitter");
}
