
#include "Precomp.h"
#include "Widget.h"
#include "Colorf.h"
#include <stdexcept>

Widget::Widget(Widget* parent, WidgetType type) : Type(type)
{
	if (type != WidgetType::Child)
	{
		DispWindow = DisplayWindow::Create(this);
		DispCanvas = Canvas::create(DispWindow->GetRenderDevice());
	}

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
		if (DispWindow)
			DispWindow->SetWindowTitle(WindowTitle);
	}
}

Size Widget::GetSize() const
{
	return Geometry.size();
}

Rect Widget::GetFrameGeometry() const
{
	if (Type == WidgetType::Child)
	{
		return Geometry;
	}
	else
	{
		return DispWindow->GetWindowFrame();
	}
}

void Widget::SetFrameGeometry(const Rect& geometry)
{
	if (Type == WidgetType::Child)
	{
		Geometry = geometry;
		OnGeometryChanged();
	}
	else
	{
		DispWindow->SetWindowFrame(geometry);
	}
}

void Widget::Show()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->Show();
	}
}

void Widget::ShowFullscreen()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->ShowFullscreen();
	}
}

void Widget::ShowMaximized()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->ShowMaximized();
	}
}

void Widget::ShowMinimized()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->ShowMinimized();
	}
}

void Widget::ShowNormal()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->ShowNormal();
	}
}

void Widget::Hide()
{
	if (Type != WidgetType::Child)
	{
		if (DispWindow)
			DispWindow->Hide();
	}
}

void Widget::ActivateWindow()
{
	if (Type != WidgetType::Child)
	{
		DispWindow->Activate();
	}
}

void Widget::Close()
{
	OnClose();
}

void Widget::Update()
{
	Widget* w = Window();
	if (w)
	{
		w->DispWindow->Update();
	}
}

void Widget::Repaint()
{
	Widget* w = Window();
	w->DispCanvas->begin(Colorf(240/255.0f, 240/255.0f, 240/255.0f));
	w->Paint(DispCanvas.get());
	w->DispCanvas->end();
}

void Widget::Paint(Canvas* canvas)
{
	Point oldOrigin = canvas->getOrigin();
	canvas->pushClip(Geometry);
	canvas->setOrigin(oldOrigin + Geometry.topLeft());
	OnPaint(canvas);
	for (Widget* w = FirstChild(); w != nullptr; w = w->NextSibling())
	{
		if (w->Type == WidgetType::Child)
			w->Paint(canvas);
	}
	canvas->setOrigin(oldOrigin);
	canvas->popClip();
}

void Widget::SetFocus()
{
	Widget* window = Window();
	if (window)
	{
		if (window->FocusWidget)
			window->FocusWidget->OnLostFocus();
		window->FocusWidget = this;
		window->FocusWidget->OnSetFocus();
		window->ActivateWindow();
	}
}

void Widget::SetEnabled(bool value)
{
}

void Widget::LockCursor()
{
	Widget* w = Window();
	if (w && w->CaptureWidget != this)
	{
		w->CaptureWidget = this;
		w->DispWindow->LockCursor();
	}
}

void Widget::UnlockCursor()
{
	Widget* w = Window();
	if (w && w->CaptureWidget != nullptr)
	{
		w->CaptureWidget = nullptr;
		w->DispWindow->UnlockCursor();
	}
}

Widget* Widget::Window()
{
	for (Widget* w = this; w != nullptr; w = w->Parent())
	{
		if (w->DispWindow)
			return w;
	}
	return nullptr;
}

Widget* Widget::ChildAt(const Point& pos)
{
	for (Widget* cur = LastChild(); cur != nullptr; cur = cur->PrevSibling())
	{
		if (cur->Geometry.contains(pos))
		{
			Widget* cur2 = cur->ChildAt(pos - Geometry.topLeft());
			return cur2 ? cur2 : cur;
		}
	}
	return nullptr;
}

Point Widget::MapFrom(const Widget* parent, const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur == parent)
			return p;
		p -= cur->Geometry.topLeft();
	}
	throw std::runtime_error("MapFrom: not a parent of widget");
}

Point Widget::MapFromGlobal(const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur->DispWindow)
		{
			return p - cur->GetFrameGeometry().topLeft();
		}
		p -= cur->Geometry.topLeft();
	}
	throw std::runtime_error("MapFromGlobal: no window widget found");
}

Point Widget::MapTo(const Widget* parent, const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur == parent)
			return p;
		p += cur->Geometry.topLeft();
	}
	throw std::runtime_error("MapTo: not a parent of widget");
}

Point Widget::MapToGlobal(const Point& pos) const
{
	Point p = pos;
	for (const Widget* cur = this; cur != nullptr; cur = cur->Parent())
	{
		if (cur->DispWindow)
		{
			return cur->GetFrameGeometry().topLeft() + p;
		}
		p += cur->Geometry.topLeft();
	}
	throw std::runtime_error("MapFromGlobal: no window widget found");
}

void Widget::OnWindowPaint()
{
	Repaint();
}

void Widget::OnWindowMouseMove(const Point& pos)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnMouseMove(CaptureWidget->MapFrom(this, pos));
	}
	else
	{
		Widget* widget = ChildAt(pos);
		if (!widget)
			widget = this;
		widget->OnMouseMove(widget->MapFrom(this, pos));
	}
}

void Widget::OnWindowMouseDown(const Point& pos, EInputKey key)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnMouseDown(CaptureWidget->MapFrom(this, pos), key);
	}
	else
	{
		Widget* widget = ChildAt(pos);
		if (!widget)
			widget = this;
		widget->OnMouseDown(widget->MapFrom(this, pos), key);
	}
}

void Widget::OnWindowMouseDoubleclick(const Point& pos, EInputKey key)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnMouseDoubleclick(CaptureWidget->MapFrom(this, pos), key);
	}
	else
	{
		Widget* widget = ChildAt(pos);
		if (!widget)
			widget = this;
		widget->OnMouseDoubleclick(widget->MapFrom(this, pos), key);
	}
}

void Widget::OnWindowMouseUp(const Point& pos, EInputKey key)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnMouseUp(CaptureWidget->MapFrom(this, pos), key);
	}
	else
	{
		Widget* widget = ChildAt(pos);
		if (!widget)
			widget = this;
		widget->OnMouseUp(widget->MapFrom(this, pos), key);
	}
}

void Widget::OnWindowMouseWheel(const Point& pos, EInputKey key)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnMouseWheel(CaptureWidget->MapFrom(this, pos), key);
	}
	else
	{
		Widget* widget = ChildAt(pos);
		if (!widget)
			widget = this;
		widget->OnMouseWheel(widget->MapFrom(this, pos), key);
	}
}

void Widget::OnWindowRawMouseMove(int dx, int dy)
{
	if (CaptureWidget)
	{
		CaptureWidget->OnRawMouseMove(dx, dy);
	}
	else if (FocusWidget)
	{
		FocusWidget->OnRawMouseMove(dx, dy);
	}
}

void Widget::OnWindowKeyChar(std::string chars)
{
	if (FocusWidget)
		FocusWidget->OnKeyChar(chars);
}

void Widget::OnWindowKeyDown(EInputKey key)
{
	if (FocusWidget)
		FocusWidget->OnKeyDown(key);
}

void Widget::OnWindowKeyUp(EInputKey key)
{
	if (FocusWidget)
		FocusWidget->OnKeyUp(key);
}

void Widget::OnWindowGeometryChanged()
{
	Size size = DispWindow->GetClientSize();
	Geometry = Rect::xywh(0.0, 0.0, size.width, size.height);
	OnGeometryChanged();
}

void Widget::OnWindowClose()
{
	Close();
}

void Widget::OnWindowActivated()
{
}

void Widget::OnWindowDeactivated()
{
}

void Widget::OnWindowDpiScaleChanged()
{
}
