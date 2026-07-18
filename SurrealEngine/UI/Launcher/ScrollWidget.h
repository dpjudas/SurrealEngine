#pragma once

#include <surrealwidgets/core/widget.h>

class Scrollbar;

// A vertical scroll viewport. Parent your content to GetContainer() and give that container a layout; the
// container is sized to the viewport width and to whatever height its layout needs, then slid under this
// widget's clip rect by a scrollbar on the right (mouse wheel and thumb drag both drive it). Used to keep
// tall settings pages - the VR tab in particular - inside a fixed-size launcher window instead of running
// off the bottom.
class ScrollWidget : public Widget
{
public:
	ScrollWidget(Widget* parent);

	// The widget your scrollable content goes in. Add children to it and call SetLayout on it, exactly as
	// you would on a plain page.
	Widget* GetContainer() const { return Container; }

protected:
	void OnGeometryChanged() override;
	bool OnMouseWheel(const Point& pos, InputKey key) override;

private:
	// Places the container at the current scroll offset and refreshes the scrollbar's range. Split from
	// OnGeometryChanged so the scrollbar callback can reposition the container without re-reading geometry.
	void PositionContainer();

	Widget* Container = nullptr;
	Scrollbar* VScroll = nullptr;
};
