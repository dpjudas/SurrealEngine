#include "widgets/scrollwidget/scrollwidget.h"
#include "widgets/scrollbar/scrollbar.h"
#include <algorithm>

ScrollWidget::ScrollWidget(Widget* parent) : Widget(parent)
{
	Container = new Widget(this);
	VScroll = new Scrollbar(this);
	VScroll->SetVertical();
	// Thumb drag: mirror the wheel path so both move the same container.
	VScroll->FuncScroll = [this]() { PositionContainer(); };
}

void ScrollWidget::OnGeometryChanged()
{
	PositionContainer();
}

void ScrollWidget::PositionContainer()
{
	double w = GetWidth();
	double h = GetHeight();
	if (w <= 0.0 || h <= 0.0)
		return; // not laid out yet - SetRanges rejects a zero view size, and there is nothing to place against

	double sw = VScroll->GetPreferredWidth();
	double viewW = std::max(0.0, w - sw);

	// The content is as tall as its layout wants, but never shorter than the viewport (so a page that fits
	// still fills the width and simply doesn't scroll).
	double contentH = std::max(Container->GetPreferredHeight(), h);

	VScroll->SetFrameGeometry(Rect::xywh(w - sw, 0.0, sw, h));
	VScroll->SetRanges(h, contentH); // view size, total size -> position runs [0, contentH - h]

	// GetPosition() is the number of pixels the content is scrolled up by; slide the container by -that.
	Container->SetFrameGeometry(Rect::xywh(0.0, -VScroll->GetPosition(), viewW, contentH));
}

bool ScrollWidget::OnMouseWheel(const Point& pos, InputKey key)
{
	const double step = 40.0; // roughly a settings row per wheel notch
	if (key == InputKey::MouseWheelUp)
		VScroll->SetPosition(std::max(VScroll->GetPosition() - step, 0.0));
	else if (key == InputKey::MouseWheelDown)
		VScroll->SetPosition(std::min(VScroll->GetPosition() + step, VScroll->GetMax()));
	else
		return false;

	PositionContainer();
	return true;
}
