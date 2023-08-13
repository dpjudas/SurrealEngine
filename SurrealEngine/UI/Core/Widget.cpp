
#include "Precomp.h"
#include "Widget.h"
#include "Colorf.h"
#include <stdexcept>

Widget::Widget(Widget* parent, WidgetType type) : Type(type)
{
	SetParent(parent);
}

Widget::~Widget()
{
	while (LastChildObj)
		delete LastChildObj;

	DetachFromParent();
}

void Widget::SetParent(Widget* newParent)
{
	if (ParentObj != newParent)
	{
		if (ParentObj)
			DetachFromParent();

		if (newParent)
		{
			PrevSiblingObj = newParent->LastChildObj;
			if (PrevSiblingObj) PrevSiblingObj->NextSiblingObj = this;
			newParent->LastChildObj = this;
			if (!newParent->FirstChildObj) newParent->FirstChildObj = this;
			ParentObj = newParent;
		}
	}
}

void Widget::MoveBefore(Widget* sibling)
{
	if (sibling && sibling->ParentObj != ParentObj) throw std::runtime_error("Invalid sibling passed to Widget.MoveBefore");
	if (!ParentObj) throw std::runtime_error("Widget must have a parent before it can be moved");

	if (NextSiblingObj != sibling)
	{
		Widget* p = ParentObj;
		DetachFromParent();

		ParentObj = p;
		if (sibling)
		{
			NextSiblingObj = sibling;
			PrevSiblingObj = sibling->PrevSiblingObj;
			sibling->PrevSiblingObj = this;
			if (PrevSiblingObj) PrevSiblingObj->NextSiblingObj = this;
			if (ParentObj->FirstChildObj == sibling) ParentObj->FirstChildObj = this;
		}
		else
		{
			PrevSiblingObj = ParentObj->LastChildObj;
			if (PrevSiblingObj) PrevSiblingObj->NextSiblingObj = this;
			ParentObj->LastChildObj = this;
			if (!ParentObj->FirstChildObj) ParentObj->FirstChildObj = this;
		}
	}
}

void Widget::DetachFromParent()
{
	if (PrevSiblingObj)
		PrevSiblingObj->NextSiblingObj = NextSiblingObj;
	if (NextSiblingObj)
		NextSiblingObj->PrevSiblingObj = PrevSiblingObj;
	if (ParentObj)
	{
		if (ParentObj->FirstChildObj == this)
			ParentObj->FirstChildObj = NextSiblingObj;
		if (ParentObj->LastChildObj == this)
			ParentObj->LastChildObj = PrevSiblingObj;
	}
	PrevSiblingObj = nullptr;
	NextSiblingObj = nullptr;
	ParentObj = nullptr;
}

std::string Widget::GetWindowTitle() const
{
	return WindowTitle;
}

void Widget::SetWindowTitle(const std::string& text)
{
	if (WindowTitle != text)
	{
		WindowTitle = text;
		//if (DispWindow)
		//	DispWindow->SetWindowTitle(WindowTitle);
	}
}

Size Widget::GetSize() const
{
	if (Type == WidgetType::Child)
	{
		return Geometry.size();
	}
	else
	{
		const_cast<Widget*>(this)->NeedsWindow();
		return Size(DispWindow->SizeX, DispWindow->SizeY);
	}
}

Rect Widget::GetFrameGeometry() const
{
	if (Type == WidgetType::Child)
	{
		return Geometry;
	}
	else
	{
		const_cast<Widget*>(this)->NeedsWindow();
		//return DispWindow->GetGeometry();
		return {};
	}
}

void Widget::NeedsWindow()
{
	if (!DispWindow)
	{
		DispWindow = DisplayWindow::Create(this);
		DispWindow->OpenWindow((int)std::round(Geometry.width), (int)std::round(Geometry.height), false);
		DispCanvas = Canvas::create(DispWindow->GetRenderDevice());
	}
}

void Widget::SetFrameGeometry(const Rect& geometry)
{
	if (Geometry != geometry)
	{
		Geometry = geometry;
		//if (DispWindow)
		//	DispWindow->SetGeometry(geometry);
		OnGeometryChanged();
	}
}

void Widget::Show()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->Show();
	}
}

void Widget::ShowFullscreen()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->ShowFullscreen();
	}
}

void Widget::ShowMaximized()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->ShowMaximized();
	}
}

void Widget::ShowMinimized()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->ShowMinimized();
	}
}

void Widget::ShowNormal()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->ShowNormal();
	}
}

void Widget::Hide()
{
	if (Type != WidgetType::Child)
	{
		DispCanvas.reset();
		DispWindow.reset();
	}
}

void Widget::ActivateWindow()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->Activate();
	}
}

void Widget::Raise()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->Raise();
	}
}

void Widget::Lower()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->Lower();
	}
}

void Widget::Close()
{
	OnClose();
}

void Widget::Update()
{
	if (Type != WidgetType::Child)
	{
		NeedsWindow();
		// DispWindow->Update();
	}
}

void Widget::Repaint()
{
	DispCanvas->begin(Colorf(240/255.0f, 240/255.0f, 240/255.0f));
	OnPaint(DispCanvas.get());
	DispCanvas->end();
}

void Widget::SetFocus()
{
	Widget* window = Window();
	if (window)
	{
		window->FocusWidget->OnLostFocus();
		window->FocusWidget = this;
		window->FocusWidget->OnSetFocus();
		window->ActivateWindow();
	}
}

void Widget::SetEnabled(bool value)
{
}

Widget* Widget::Window() const
{
	return nullptr;
}

Widget* Widget::ChildAt(const Point& pos) const
{
	return nullptr;
}

Point Widget::MapFrom(const Widget* parent, const Point& pos) const
{
	return pos;
}

Point Widget::MapFromGlobal(const Point& pos) const
{
	return pos;
}

Point Widget::MapTo(const Widget* parent, const Point& pos) const
{
	return pos;
}

Point Widget::MapToGlobal(const Point& pos) const
{
	return pos;
}

void Widget::Key(DisplayWindow* window, std::string key)
{
	if (FocusWidget)
		FocusWidget->OnKeyChar(key);
}

void Widget::InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta)
{
}

void Widget::FocusChange(bool lost)
{
}

void Widget::MouseMove(float x, float y)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnMouseMove(CaptureWidget->MapFrom(this, Point(x, y)));
	}
	else
	{
		Widget* widget = ChildAt(Point(x, y));
		if (widget)
			widget->OnMouseMove(widget->MapFrom(this, Point(x, y)));
	}
}

bool Widget::MouseCursorVisible()
{
	return true;
}

void Widget::WindowClose(DisplayWindow* window)
{
	Close();
}
