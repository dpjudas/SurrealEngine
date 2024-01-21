
#pragma once

#include "../../core/widget.h"
#include <vector>
#include <functional>

class Scrollbar;

class ListView : public Widget
{
public:
	ListView(Widget* parent = nullptr);

	void AddItem(const std::string& text);
	int GetSelectedItem() const { return selectedItem; }
	void SetSelectedItem(int index);
	void ScrollToItem(int index);

	void Activate();

	std::function<void(int)> OnChanged;
	std::function<void()> OnActivated;

protected:
	void OnPaint(Canvas* canvas) override;
	bool OnMouseDown(const Point& pos, InputKey key) override;
	bool OnMouseDoubleclick(const Point& pos, InputKey key) override;
	bool OnMouseWheel(const Point& pos, InputKey key) override;
	void OnKeyDown(InputKey key) override;
	void OnGeometryChanged() override;
	void OnScrollbarScroll();

	Scrollbar* scrollbar = nullptr;

	std::vector<std::string> items;
	int selectedItem = 0;
};
