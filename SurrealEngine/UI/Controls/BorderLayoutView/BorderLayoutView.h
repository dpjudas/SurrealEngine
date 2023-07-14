#pragma once

#include "UI/Core/View.h"

class BorderLayoutSplitter;

class BorderLayoutView : public View
{
public:
	BorderLayoutView(View* parent = nullptr);

	void setTopView(View* view, double height);
	void setLeftView(View* view, double width);
	void setCenterView(View* view);
	void setRightView(View* view, double width);
	void setBottomView(View* view, double height);

	void removeTopView() { setTopView(nullptr, 0); }
	void removeLeftView() { setLeftView(nullptr, 0); }
	void removeCenterView() { setCenterView(nullptr); }
	void removeRightView() { setRightView(nullptr, 0); }
	void removeBottomView() { setBottomView(nullptr, 0); }

	void showTopView();
	void showLeftView();
	void showRightView();
	void showBottomView();

	void hideTopView();
	void hideLeftView();
	void hideRightView();
	void hideBottomView();

	View* takeTopView();
	View* takeLeftView();
	View* takeCenterView();
	View* takeRightView();
	View* takeBottomView();

	View* topView() const { return top; }
	View* leftView() const { return left; }
	View* centerView() const { return center; }
	View* rightView() const { return right; }
	View* bottomView() const { return bottom; }

private:
	View* top = nullptr;
	View* left = nullptr;
	View* center = nullptr;
	View* right = nullptr;
	View* bottom = nullptr;
	VBoxView* vbox = nullptr;
	HBoxView* centerRow = nullptr;
	BorderLayoutSplitter* topSplitter = nullptr;
	BorderLayoutSplitter* leftSplitter = nullptr;
	BorderLayoutSplitter* rightSplitter = nullptr;
	BorderLayoutSplitter* bottomSplitter = nullptr;
};

enum class BorderLayoutSplitterPos
{
	Top,
	Bottom,
	Left,
	Right
};

class BorderLayoutSplitter : public View
{
public:
	BorderLayoutSplitter(BorderLayoutSplitterPos pos, View* parent = nullptr);

	BorderLayoutSplitterPos pos = {};
	double size = 0;
};
