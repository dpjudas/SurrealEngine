
#include "win32_display_window.h"
#include <windowsx.h>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC		((USHORT) 0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE	((USHORT) 0x02)
#endif

#ifndef HID_USAGE_GENERIC_JOYSTICK
#define HID_USAGE_GENERIC_JOYSTICK	((USHORT) 0x04)
#endif

#ifndef HID_USAGE_GENERIC_GAMEPAD
#define HID_USAGE_GENERIC_GAMEPAD	((USHORT) 0x05)
#endif

#ifndef RIDEV_INPUTSINK
#define RIDEV_INPUTSINK	(0x100)
#endif

Win32DisplayWindow::Win32DisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, Win32DisplayWindow* owner) : WindowHost(windowHost), PopupWindow(popupWindow)
{
	Windows.push_front(this);
	WindowsIterator = Windows.begin();

	WNDCLASSEX classdesc = {};
	classdesc.cbSize = sizeof(WNDCLASSEX);
	classdesc.hInstance = GetModuleHandle(0);
	classdesc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
	classdesc.lpszClassName = L"ZWidgetWindow";
	classdesc.lpfnWndProc = &Win32DisplayWindow::WndProc;
	RegisterClassEx(&classdesc);

	// Microsoft logic at its finest:
	// WS_EX_DLGMODALFRAME hides the sysmenu icon
	// WS_CAPTION shows the caption (yay! actually a flag that does what it says it does!)
	// WS_SYSMENU shows the min/max/close buttons
	// WS_THICKFRAME makes the window resizable

	DWORD style = 0, exstyle = 0;
	if (popupWindow)
	{
		exstyle = WS_EX_NOACTIVATE;
		style = WS_POPUP;
	}
	else
	{
		exstyle = WS_EX_APPWINDOW | WS_EX_DLGMODALFRAME;
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	}
	CreateWindowEx(exstyle, L"ZWidgetWindow", L"", style, 0, 0, 100, 100, owner ? owner->WindowHandle : 0, 0, GetModuleHandle(0), this);
}

Win32DisplayWindow::~Win32DisplayWindow()
{
	if (WindowHandle)
	{
		DestroyWindow(WindowHandle);
		WindowHandle = 0;
	}

	Windows.erase(WindowsIterator);
}

void Win32DisplayWindow::SetWindowTitle(const std::string& text)
{
	SetWindowText(WindowHandle, to_utf16(text).c_str());
}

void Win32DisplayWindow::SetBorderColor(uint32_t bgra8)
{
	bgra8 = bgra8 & 0x00ffffff;
	DwmSetWindowAttribute(WindowHandle, 34/*DWMWA_BORDER_COLOR*/, &bgra8, sizeof(uint32_t));
}

void Win32DisplayWindow::SetCaptionColor(uint32_t bgra8)
{
	bgra8 = bgra8 & 0x00ffffff;
	DwmSetWindowAttribute(WindowHandle, 35/*DWMWA_CAPTION_COLOR*/, &bgra8, sizeof(uint32_t));
}

void Win32DisplayWindow::SetCaptionTextColor(uint32_t bgra8)
{
	bgra8 = bgra8 & 0x00ffffff;
	DwmSetWindowAttribute(WindowHandle, 36/*DWMWA_TEXT_COLOR*/, &bgra8, sizeof(uint32_t));
}

void Win32DisplayWindow::SetWindowFrame(const Rect& box)
{
	double dpiscale = GetDpiScale();
	SetWindowPos(WindowHandle, nullptr, (int)std::round(box.x * dpiscale), (int)std::round(box.y * dpiscale), (int)std::round(box.width * dpiscale), (int)std::round(box.height * dpiscale), SWP_NOACTIVATE | SWP_NOZORDER);
}

void Win32DisplayWindow::SetClientFrame(const Rect& box)
{
	double dpiscale = GetDpiScale();

	RECT rect = {};
	rect.left = (int)std::round(box.x * dpiscale);
	rect.top = (int)std::round(box.y * dpiscale);
	rect.right = rect.left + (int)std::round(box.width * dpiscale);
	rect.bottom = rect.top + (int)std::round(box.height * dpiscale);

	DWORD style = (DWORD)GetWindowLongPtr(WindowHandle, GWL_STYLE);
	DWORD exstyle = (DWORD)GetWindowLongPtr(WindowHandle, GWL_EXSTYLE);
	AdjustWindowRectExForDpi(&rect, style, FALSE, exstyle, GetDpiForWindow(WindowHandle));

	SetWindowPos(WindowHandle, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOZORDER);
}

void Win32DisplayWindow::Show()
{
	ShowWindow(WindowHandle, PopupWindow ? SW_SHOWNA : SW_SHOW);
}

void Win32DisplayWindow::ShowFullscreen()
{
	HDC screenDC = GetDC(0);
	int width = GetDeviceCaps(screenDC, HORZRES);
	int height = GetDeviceCaps(screenDC, VERTRES);
	ReleaseDC(0, screenDC);
	SetWindowLongPtr(WindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW);
	SetWindowLongPtr(WindowHandle, GWL_STYLE, WS_OVERLAPPED);
	SetWindowPos(WindowHandle, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	Fullscreen = true;
}

void Win32DisplayWindow::ShowMaximized()
{
	ShowWindow(WindowHandle, SW_SHOWMAXIMIZED);
}

void Win32DisplayWindow::ShowMinimized()
{
	ShowWindow(WindowHandle, SW_SHOWMINIMIZED);
}

void Win32DisplayWindow::ShowNormal()
{
	ShowWindow(WindowHandle, SW_NORMAL);
}

void Win32DisplayWindow::Hide()
{
	ShowWindow(WindowHandle, SW_HIDE);
}

void Win32DisplayWindow::Activate()
{
	if (!PopupWindow)
		SetFocus(WindowHandle);
}

void Win32DisplayWindow::ShowCursor(bool enable)
{
}

void Win32DisplayWindow::LockCursor()
{
	if (!MouseLocked)
	{
		MouseLocked = true;
		GetCursorPos(&MouseLockPos);
		::ShowCursor(FALSE);

		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid.usUsage = HID_USAGE_GENERIC_MOUSE;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = WindowHandle;
		RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
	}
}

void Win32DisplayWindow::UnlockCursor()
{
	if (MouseLocked)
	{
		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid.usUsage = HID_USAGE_GENERIC_MOUSE;
		rid.dwFlags = RIDEV_REMOVE;
		rid.hwndTarget = 0;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));

		MouseLocked = false;
		SetCursorPos(MouseLockPos.x, MouseLockPos.y);
		::ShowCursor(TRUE);
	}
}

void Win32DisplayWindow::CaptureMouse()
{
	SetCapture(WindowHandle);
}

void Win32DisplayWindow::ReleaseMouseCapture()
{
	ReleaseCapture();
}

void Win32DisplayWindow::Update()
{
	InvalidateRect(WindowHandle, nullptr, FALSE);
}

bool Win32DisplayWindow::GetKeyState(InputKey key)
{
	return ::GetKeyState((int)key) & 0x8000; // High bit (0x8000) means key is down, Low bit (0x0001) means key is sticky on (like Caps Lock, Num Lock, etc.)
}

void Win32DisplayWindow::SetCursor(StandardCursor cursor)
{
	if (cursor != CurrentCursor)
	{
		CurrentCursor = cursor;
		UpdateCursor();
	}
}

Rect Win32DisplayWindow::GetWindowFrame() const
{
	RECT box = {};
	GetWindowRect(WindowHandle, &box);
	double dpiscale = GetDpiScale();
	return Rect(box.left / dpiscale, box.top / dpiscale, (box.right - box.left) / dpiscale, (box.bottom - box.top) / dpiscale);
}

Point Win32DisplayWindow::MapFromGlobal(const Point& pos) const
{
	double dpiscale = GetDpiScale();
	POINT point = {};
	point.x = (LONG)std::round(pos.x / dpiscale);
	point.y = (LONG)std::round(pos.y / dpiscale);
	ScreenToClient(WindowHandle, &point);
	return Point(point.x * dpiscale, point.y * dpiscale);
}

Point Win32DisplayWindow::MapToGlobal(const Point& pos) const
{
	double dpiscale = GetDpiScale();
	POINT point = {};
	point.x = (LONG)std::round(pos.x * dpiscale);
	point.y = (LONG)std::round(pos.y * dpiscale);
	ClientToScreen(WindowHandle, &point);
	return Point(point.x / dpiscale, point.y / dpiscale);
}

Size Win32DisplayWindow::GetClientSize() const
{
	RECT box = {};
	GetClientRect(WindowHandle, &box);
	double dpiscale = GetDpiScale();
	return Size(box.right / dpiscale, box.bottom / dpiscale);
}

int Win32DisplayWindow::GetPixelWidth() const
{
	RECT box = {};
	GetClientRect(WindowHandle, &box);
	return box.right;
}

int Win32DisplayWindow::GetPixelHeight() const
{
	RECT box = {};
	GetClientRect(WindowHandle, &box);
	return box.bottom;
}

double Win32DisplayWindow::GetDpiScale() const
{
	return GetDpiForWindow(WindowHandle) / 96.0;
}

std::string Win32DisplayWindow::GetClipboardText()
{
	BOOL result = OpenClipboard(WindowHandle);
	if (result == FALSE)
		throw std::runtime_error("Unable to open clipboard");

	HANDLE handle = GetClipboardData(CF_UNICODETEXT);
	if (handle == 0)
	{
		CloseClipboard();
		return std::string();
	}

	std::wstring::value_type* data = (std::wstring::value_type*)GlobalLock(handle);
	if (data == 0)
	{
		CloseClipboard();
		return std::string();
	}
	std::string str = from_utf16(data);
	GlobalUnlock(handle);

	CloseClipboard();
	return str;
}

void Win32DisplayWindow::SetClipboardText(const std::string& text)
{
	std::wstring text16 = to_utf16(text);

	BOOL result = OpenClipboard(WindowHandle);
	if (result == FALSE)
		throw std::runtime_error("Unable to open clipboard");

	result = EmptyClipboard();
	if (result == FALSE)
	{
		CloseClipboard();
		throw std::runtime_error("Unable to empty clipboard");
	}

	unsigned int length = (unsigned int)((text16.length() + 1) * sizeof(std::wstring::value_type));
	HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, length);
	if (handle == 0)
	{
		CloseClipboard();
		throw std::runtime_error("Unable to allocate clipboard memory");
	}

	void* data = GlobalLock(handle);
	if (data == 0)
	{
		GlobalFree(handle);
		CloseClipboard();
		throw std::runtime_error("Unable to lock clipboard memory");
	}
	memcpy(data, text16.c_str(), length);
	GlobalUnlock(handle);

	HANDLE data_result = SetClipboardData(CF_UNICODETEXT, handle);

	if (data_result == 0)
	{
		GlobalFree(handle);
		CloseClipboard();
		throw std::runtime_error("Unable to set clipboard data");
	}

	CloseClipboard();
}

void Win32DisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels)
{
	BITMAPV5HEADER header = {};
	header.bV5Size = sizeof(BITMAPV5HEADER);
	header.bV5Width = width;
	header.bV5Height = -height;
	header.bV5Planes = 1;
	header.bV5BitCount = 32;
	header.bV5Compression = BI_BITFIELDS;
	header.bV5AlphaMask = 0xff000000;
	header.bV5RedMask = 0x00ff0000;
	header.bV5GreenMask = 0x0000ff00;
	header.bV5BlueMask = 0x000000ff;
	header.bV5SizeImage = width * height * sizeof(uint32_t);
	header.bV5CSType = LCS_sRGB;

	HDC dc = PaintDC;
	if (dc != 0)
	{
		int result = SetDIBitsToDevice(dc, 0, 0, width, height, 0, 0, 0, height, pixels, (const BITMAPINFO*)&header, BI_RGB);
		ReleaseDC(WindowHandle, dc);
	}
}

LRESULT Win32DisplayWindow::OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	LPARAM result = 0;
	if (DwmDefWindowProc(WindowHandle, msg, wparam, lparam, &result))
		return result;

	if (msg == WM_INPUT)
	{
		bool hasFocus = GetFocus() != 0;

		HRAWINPUT handle = (HRAWINPUT)lparam;
		UINT size = 0;
		UINT result = GetRawInputData(handle, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));
		if (result == 0 && size > 0)
		{
			size *= 2;
			std::vector<uint8_t*> buffer(size);
			result = GetRawInputData(handle, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));
			if (result >= 0)
			{
				RAWINPUT* rawinput = (RAWINPUT*)buffer.data();
				if (rawinput->header.dwType == RIM_TYPEMOUSE)
				{
					if (hasFocus)
						WindowHost->OnWindowRawMouseMove(rawinput->data.mouse.lLastX, rawinput->data.mouse.lLastY);
				}
			}
		}
		return DefWindowProc(WindowHandle, msg, wparam, lparam);
	}
	else if (msg == WM_PAINT)
	{
		PAINTSTRUCT paintStruct = {};
		PaintDC = BeginPaint(WindowHandle, &paintStruct);
		if (PaintDC)
		{
			WindowHost->OnWindowPaint();
			EndPaint(WindowHandle, &paintStruct);
			PaintDC = 0;
		}
		return 0;
	}
	else if (msg == WM_ACTIVATE)
	{
		WindowHost->OnWindowActivated();
	}
	else if (msg == WM_MOUSEACTIVATE)
	{
		// We don't want to activate the window on mouse clicks as that changes the focus from the popup owner to the popup itself
		if (PopupWindow)
			return MA_NOACTIVATE;
	}
	else if (msg == WM_MOUSEMOVE)
	{
		if (MouseLocked && GetFocus() != 0)
		{
			RECT box = {};
			GetClientRect(WindowHandle, &box);

			POINT center = {};
			center.x = box.right / 2;
			center.y = box.bottom / 2;
			ClientToScreen(WindowHandle, &center);

			SetCursorPos(center.x, center.y);
		}
		else
		{
			UpdateCursor();
		}

		if (!TrackMouseActive)
		{
			TRACKMOUSEEVENT eventTrack = {};
			eventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
			eventTrack.hwndTrack = WindowHandle;
			eventTrack.dwFlags = TME_LEAVE;
			if (TrackMouseEvent(&eventTrack))
				TrackMouseActive = true;
		}

		WindowHost->OnWindowMouseMove(GetLParamPos(lparam));
	}
	else if (msg == WM_MOUSELEAVE)
	{
		TrackMouseActive = false;
		WindowHost->OnWindowMouseLeave();
	}
	else if (msg == WM_LBUTTONDOWN)
	{
		WindowHost->OnWindowMouseDown(GetLParamPos(lparam), InputKey::LeftMouse);
	}
	else if (msg == WM_LBUTTONDBLCLK)
	{
		WindowHost->OnWindowMouseDoubleclick(GetLParamPos(lparam), InputKey::LeftMouse);
	}
	else if (msg == WM_LBUTTONUP)
	{
		WindowHost->OnWindowMouseUp(GetLParamPos(lparam), InputKey::LeftMouse);
	}
	else if (msg == WM_MBUTTONDOWN)
	{
		WindowHost->OnWindowMouseDown(GetLParamPos(lparam), InputKey::MiddleMouse);
	}
	else if (msg == WM_MBUTTONDBLCLK)
	{
		WindowHost->OnWindowMouseDoubleclick(GetLParamPos(lparam), InputKey::MiddleMouse);
	}
	else if (msg == WM_MBUTTONUP)
	{
		WindowHost->OnWindowMouseUp(GetLParamPos(lparam), InputKey::MiddleMouse);
	}
	else if (msg == WM_RBUTTONDOWN)
	{
		WindowHost->OnWindowMouseDown(GetLParamPos(lparam), InputKey::RightMouse);
	}
	else if (msg == WM_RBUTTONDBLCLK)
	{
		WindowHost->OnWindowMouseDoubleclick(GetLParamPos(lparam), InputKey::RightMouse);
	}
	else if (msg == WM_RBUTTONUP)
	{
		WindowHost->OnWindowMouseUp(GetLParamPos(lparam), InputKey::RightMouse);
	}
	else if (msg == WM_MOUSEWHEEL)
	{
		double delta = GET_WHEEL_DELTA_WPARAM(wparam) / (double)WHEEL_DELTA;

		// Note: WM_MOUSEWHEEL uses screen coordinates. GetLParamPos assumes client coordinates.
		double dpiscale = GetDpiScale();
		POINT pos;
		pos.x = GET_X_LPARAM(lparam);
		pos.y = GET_Y_LPARAM(lparam);
		ScreenToClient(WindowHandle, &pos);

		WindowHost->OnWindowMouseWheel(Point(pos.x / dpiscale, pos.y / dpiscale), delta < 0.0 ? InputKey::MouseWheelDown : InputKey::MouseWheelUp);
	}
	else if (msg == WM_CHAR)
	{
		wchar_t buf[2] = { (wchar_t)wparam, 0 };
		WindowHost->OnWindowKeyChar(from_utf16(buf));
	}
	else if (msg == WM_KEYDOWN)
	{
		WindowHost->OnWindowKeyDown((InputKey)wparam);
	}
	else if (msg == WM_KEYUP)
	{
		WindowHost->OnWindowKeyUp((InputKey)wparam);
	}
	else if (msg == WM_SETFOCUS)
	{
		if (MouseLocked)
		{
			::ShowCursor(FALSE);
		}
	}
	else if (msg == WM_KILLFOCUS)
	{
		if (MouseLocked)
		{
			::ShowCursor(TRUE);
		}
	}
	else if (msg == WM_CLOSE)
	{
		WindowHost->OnWindowClose();
		return 0;
	}
	else if (msg == WM_SIZE)
	{
		WindowHost->OnWindowGeometryChanged();
		return 0;
	}
	/*else if (msg == WM_NCCALCSIZE && wparam == TRUE) // calculate client area for the window
	{
		NCCALCSIZE_PARAMS* calcsize = (NCCALCSIZE_PARAMS*)lparam;
		return WVR_REDRAW;
	}*/

	return DefWindowProc(WindowHandle, msg, wparam, lparam);
}

void Win32DisplayWindow::UpdateCursor()
{
	LPCWSTR cursor = IDC_ARROW;
	switch (CurrentCursor)
	{
	case StandardCursor::arrow: cursor = IDC_ARROW; break;
	case StandardCursor::appstarting: cursor = IDC_APPSTARTING; break;
	case StandardCursor::cross: cursor = IDC_CROSS; break;
	case StandardCursor::hand: cursor = IDC_HAND; break;
	case StandardCursor::ibeam: cursor = IDC_IBEAM; break;
	case StandardCursor::no: cursor = IDC_NO; break;
	case StandardCursor::size_all: cursor = IDC_SIZEALL; break;
	case StandardCursor::size_nesw: cursor = IDC_SIZENESW; break;
	case StandardCursor::size_ns: cursor = IDC_SIZENS; break;
	case StandardCursor::size_nwse: cursor = IDC_SIZENWSE; break;
	case StandardCursor::size_we: cursor = IDC_SIZEWE; break;
	case StandardCursor::uparrow: cursor = IDC_UPARROW; break;
	case StandardCursor::wait: cursor = IDC_WAIT; break;
	default: break;
	}

	::SetCursor((HCURSOR)LoadImage(0, cursor, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED));
}

Point Win32DisplayWindow::GetLParamPos(LPARAM lparam) const
{
	double dpiscale = GetDpiScale();
	return Point(GET_X_LPARAM(lparam) / dpiscale, GET_Y_LPARAM(lparam) / dpiscale);
}

LRESULT Win32DisplayWindow::WndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_CREATE)
	{
		CREATESTRUCT* createstruct = (CREATESTRUCT*)lparam;
		Win32DisplayWindow* viewport = (Win32DisplayWindow*)createstruct->lpCreateParams;
		viewport->WindowHandle = windowhandle;
		SetWindowLongPtr(windowhandle, GWLP_USERDATA, (LONG_PTR)viewport);
		return viewport->OnWindowMessage(msg, wparam, lparam);
	}
	else
	{
		Win32DisplayWindow* viewport = (Win32DisplayWindow*)GetWindowLongPtr(windowhandle, GWLP_USERDATA);
		if (viewport)
		{
			LRESULT result = viewport->OnWindowMessage(msg, wparam, lparam);
			if (msg == WM_DESTROY)
			{
				SetWindowLongPtr(windowhandle, GWLP_USERDATA, 0);
				viewport->WindowHandle = 0;
			}
			return result;
		}
		else
		{
			return DefWindowProc(windowhandle, msg, wparam, lparam);
		}
	}
}

void Win32DisplayWindow::ProcessEvents()
{
	while (true)
	{
		MSG msg = {};
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) <= 0)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Win32DisplayWindow::RunLoop()
{
	while (!ExitRunLoop && !Windows.empty())
	{
		MSG msg = {};
		if (GetMessage(&msg, 0, 0, 0) <= 0)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ExitRunLoop = false;
}

void Win32DisplayWindow::ExitLoop()
{
	ExitRunLoop = true;
}

Size Win32DisplayWindow::GetScreenSize()
{
	HDC screenDC = GetDC(0);
	int screenWidth = GetDeviceCaps(screenDC, HORZRES);
	int screenHeight = GetDeviceCaps(screenDC, VERTRES);
	double dpiScale = GetDeviceCaps(screenDC, LOGPIXELSX) / 96.0;
	ReleaseDC(0, screenDC);

	return Size(screenWidth / dpiScale, screenHeight / dpiScale);
}

static void CALLBACK Win32TimerCallback(HWND handle, UINT message, UINT_PTR timerID, DWORD timestamp)
{
	auto it = Win32DisplayWindow::Timers.find(timerID);
	if (it != Win32DisplayWindow::Timers.end())
	{
		it->second();
	}
}

void* Win32DisplayWindow::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	UINT_PTR result = SetTimer(0, 0, timeoutMilliseconds, Win32TimerCallback);
	if (result == 0)
		throw std::runtime_error("Could not create timer");
	Timers[result] = std::move(onTimer);
	return (void*)result;
}

void Win32DisplayWindow::StopTimer(void* timerID)
{
	auto it = Timers.find((UINT_PTR)timerID);
	if (it != Timers.end())
	{
		Timers.erase(it);
		KillTimer(0, (UINT_PTR)timerID);
	}
}

std::list<Win32DisplayWindow*> Win32DisplayWindow::Windows;
bool Win32DisplayWindow::ExitRunLoop;

std::unordered_map<UINT_PTR, std::function<void()>> Win32DisplayWindow::Timers;
