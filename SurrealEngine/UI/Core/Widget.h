#pragma once

#include <string>
#include <memory>
#include "Canvas.h"
#include "Rect.h"
#include "Window/Window.h"

class Canvas;

enum class WidgetType
{
	Child,
	Window,
	Popup
};

class Widget : DisplayWindowHost
{
public:
	Widget(Widget* parent = nullptr, WidgetType type = WidgetType::Child);
	virtual ~Widget();

	void SetParent(Widget* parent);
	void MoveBefore(Widget* sibling);

	std::string GetWindowTitle() const;
	void SetWindowTitle(const std::string& text);

	// Icon GetWindowIcon() const;
	// void SetWindowIcon(const Icon& icon);

	// Widget content box
	Size GetSize() const;
	double GetWidth() const { return GetSize().width; }
	double GetHeight() const { return GetSize().height; }

	// Widget frame box
	Rect GetFrameGeometry() const;
	void SetFrameGeometry(const Rect& geometry);
	void SetFrameGeometry(double x, double y, double width, double height) { SetFrameGeometry(Rect::xywh(x, y, width, height)); }

	void Show();
	void ShowFullscreen();
	void ShowMaximized();
	void ShowMinimized();
	void ShowNormal();
	void Hide();

	void ActivateWindow();
	void Raise();
	void Lower();

	void Close();

	void Update();
	void Repaint();

	void SetFocus();
	void SetEnabled(bool value);
	void SetDisabled(bool value) { SetEnabled(!value); }
	void SetHidden(bool value) { if (value) Hide(); else Show(); }

	Widget* Window();
	Widget* ChildAt(double x, double y) { return ChildAt(Point(x, y)); }
	Widget* ChildAt(const Point& pos);

	Widget* Parent() const { return ParentObj; }
	Widget* PrevSibling() const { return PrevSiblingObj; }
	Widget* NextSibling() const { return NextSiblingObj; }
	Widget* FirstChild() const { return FirstChildObj; }
	Widget* LastChild() const { return LastChildObj; }

	Point MapFrom(const Widget* parent, const Point& pos) const;
	Point MapFromGlobal(const Point& pos) const;
	Point MapFromParent(const Point& pos) const { return MapFrom(Parent(), pos); }

	Point MapTo(const Widget* parent, const Point& pos) const;
	Point MapToGlobal(const Point& pos) const;
	Point MapToParent(const Point& pos) const { return MapTo(Parent(), pos); }

	void TickWindow() { DispWindow->Tick(); } // To do: remove this. The entire Window subsystem should tick as a single thing instead.
	RenderDevice* GetRenderDevice() { return DispWindow->GetRenderDevice(); }

protected:
	virtual void OnPaint(Canvas* canvas) { }
	virtual void OnMouseMove(const Point& pos) { }
	virtual void OnMouseDown(const Point& pos, int key) { }
	virtual void OnMouseDoubleclick(const Point& pos, int key) { }
	virtual void OnMouseUp(const Point& pos, int key) { }
	virtual void OnKeyChar(std::string chars) { }
	virtual void OnKeyDown(int key) { }
	virtual void OnKeyUp(int key) { }
	virtual void OnGeometryChanged() { }
	virtual void OnClose() { delete this; }
	virtual void OnSetFocus() { }
	virtual void OnLostFocus() { }
	virtual void OnEnabled() { }
	virtual void OnDisabled() { }

private:
	void DetachFromParent();
	void NeedsWindow();

	void Paint(Canvas* canvas);

	// DisplayWindowHost
	void Key(DisplayWindow* window, std::string key) override;
	void InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta) override;
	void FocusChange(bool lost) override;
	void MouseMove(float x, float y) override;
	bool MouseCursorVisible() override;
	void WindowClose(DisplayWindow* window) override;

	WidgetType Type = {};

	Widget* ParentObj = nullptr;
	Widget* PrevSiblingObj = nullptr;
	Widget* NextSiblingObj = nullptr;
	Widget* FirstChildObj = nullptr;
	Widget* LastChildObj = nullptr;

	Rect Geometry = Rect::xywh(-1.0, -1.0, 0.0, 0.0);

	std::string WindowTitle;
	std::unique_ptr<DisplayWindow> DispWindow;
	std::unique_ptr<Canvas> DispCanvas;
	Widget* FocusWidget = nullptr;
	Widget* CaptureWidget = nullptr;

	Widget(const Widget&) = delete;
	Widget& operator=(const Widget&) = delete;
};
