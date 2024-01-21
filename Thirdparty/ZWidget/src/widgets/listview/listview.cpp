
#include "widgets/listview/listview.h"
#include "widgets/scrollbar/scrollbar.h"

ListView::ListView(Widget* parent) : Widget(parent)
{
	SetStyleClass("listview");

	scrollbar = new Scrollbar(this);
	scrollbar->FuncScroll = [=]() { OnScrollbarScroll(); };
}

void ListView::AddItem(const std::string& text)
{
	items.push_back(text);
	Update();
}

void ListView::Activate()
{
	if (OnActivated)
		OnActivated();
}

void ListView::SetSelectedItem(int index)
{
	if (selectedItem != index && index >= 0 && index < items.size())
	{
		selectedItem = index;
		if (OnChanged) OnChanged(selectedItem);
		Update();
	}
}

void ListView::ScrollToItem(int index)
{
	double itemHeight = 20.0;
	double y = itemHeight * index;
	if (y < scrollbar->GetPosition())
	{
		scrollbar->SetPosition(y);
	}
	else if (y + itemHeight > scrollbar->GetPosition() + GetHeight())
	{
		scrollbar->SetPosition(std::max(y + itemHeight - GetHeight(), 0.0));
	}
}

void ListView::OnScrollbarScroll()
{
	Update();
}

void ListView::OnGeometryChanged()
{
	double w = GetWidth();
	double h = GetHeight();
	double sw = scrollbar->GetPreferredWidth();
	scrollbar->SetFrameGeometry(Rect::xywh(w - sw, 0.0, sw, h));
	scrollbar->SetRanges(h, items.size() * 20.0);
}

void ListView::OnPaint(Canvas* canvas)
{
	double y = -scrollbar->GetPosition();
	double x = 2.0;
	double w = GetWidth() - scrollbar->GetPreferredWidth() - 2.0;
	double h = 20.0;

	Colorf textColor = GetStyleColor("color");
	Colorf selectionColor = GetStyleColor("selection-color");

	int index = 0;
	for (const std::string& item : items)
	{
		double itemY = y;
		if (itemY + h >= 0.0 && itemY < GetHeight())
		{
			if (index == selectedItem)
			{
				canvas->fillRect(Rect::xywh(x - 2.0, itemY, w, h), selectionColor);
			}
			canvas->drawText(Point(x, y + 15.0), textColor, item);
		}
		y += h;
		index++;
	}
}

bool ListView::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();

	if (key == InputKey::LeftMouse)
	{
		int index = (int)((pos.y - 5.0 + scrollbar->GetPosition()) / 20.0);
		if (index >= 0 && (size_t)index < items.size())
		{
			SetSelectedItem(index);
			ScrollToItem(selectedItem);
		}
	}
	return true;
}

bool ListView::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	if (key == InputKey::LeftMouse)
	{
		Activate();
	}
	return true;
}

bool ListView::OnMouseWheel(const Point& pos, InputKey key)
{
	if (key == InputKey::MouseWheelUp)
	{
		scrollbar->SetPosition(std::max(scrollbar->GetPosition() - 20.0, 0.0));
	}
	else if (key == InputKey::MouseWheelDown)
	{
		scrollbar->SetPosition(std::min(scrollbar->GetPosition() + 20.0, scrollbar->GetMax()));
	}
	return true;
}

void ListView::OnKeyDown(InputKey key)
{
	if (key == InputKey::Down)
	{
		if (selectedItem + 1 < (int)items.size())
		{
			SetSelectedItem(selectedItem + 1);
		}
		ScrollToItem(selectedItem);
	}
	else if (key == InputKey::Up)
	{
		if (selectedItem > 0)
		{
			SetSelectedItem(selectedItem - 1);
		}
		ScrollToItem(selectedItem);
	}
	else if (key == InputKey::Enter)
	{
		Activate();
	}
}
