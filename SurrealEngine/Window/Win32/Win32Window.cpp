
#include "Precomp.h"
#include "Win32Window.h"
#include "UTF16.h"
#include "RenderDevice/RenderDevice.h"
#include <zvulkan/vulkansurface.h>
#include <zvulkan/vulkancompatibledevice.h>
#include <zvulkan/vulkanbuilders.h>
#include <windowsx.h>

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

Win32Window::Win32Window(DisplayWindowHost* windowHost) : WindowHost(windowHost)
{
	Windows.push_front(this);
	WindowsIterator = Windows.begin();

	WNDCLASSEX classdesc = {};
	classdesc.cbSize = sizeof(WNDCLASSEX);
	classdesc.hInstance = GetModuleHandle(0);
	classdesc.style = CS_VREDRAW | CS_HREDRAW;
	classdesc.lpszClassName = L"Win32Window";
	classdesc.lpfnWndProc = &Win32Window::WndProc;
	RegisterClassEx(&classdesc);

	CreateWindowEx(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW, L"Win32Window", L"", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, 0, 0, GetModuleHandle(0), this);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid.usUsage = HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = WindowHandle;
	BOOL result = RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));

	auto instance = VulkanInstanceBuilder()
		.RequireSurfaceExtensions()
		.DebugLayer(false)
		.Create();

	auto surface = std::make_shared<VulkanSurface>(instance, WindowHandle);

	Device = RenderDevice::Create(this, surface);
}

Win32Window::~Win32Window()
{
	Device.reset();
	if (WindowHandle)
	{
		DestroyWindow(WindowHandle);
		WindowHandle = 0;
	}

	Windows.erase(WindowsIterator);
}

void Win32Window::SetWindowTitle(const std::string& text)
{
	SetWindowText(WindowHandle, to_utf16(text).c_str());
}

void Win32Window::SetWindowFrame(const Rect& box)
{
	double dpiscale = GetDpiScale();
	SetWindowPos(WindowHandle, nullptr, (int)std::round(box.x * dpiscale), (int)std::round(box.y * dpiscale), (int)std::round(box.width * dpiscale), (int)std::round(box.height * dpiscale), SWP_NOACTIVATE | SWP_NOZORDER);
}

void Win32Window::SetClientFrame(const Rect& box)
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

void Win32Window::Show()
{
	ShowWindow(WindowHandle, SW_SHOW);
}

void Win32Window::ShowFullscreen()
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

void Win32Window::ShowMaximized()
{
	ShowWindow(WindowHandle, SW_SHOWMAXIMIZED);
}

void Win32Window::ShowMinimized()
{
	ShowWindow(WindowHandle, SW_SHOWMINIMIZED);
}

void Win32Window::ShowNormal()
{
	ShowWindow(WindowHandle, SW_NORMAL);
}

void Win32Window::Hide()
{
	ShowWindow(WindowHandle, SW_HIDE);
}

void Win32Window::Activate()
{
	SetFocus(WindowHandle);
}

void Win32Window::ShowCursor(bool enable)
{
}

void Win32Window::LockCursor()
{
	if (!MouseLocked)
	{
		MouseLocked = true;
		GetCursorPos(&MouseLockPos);
		::ShowCursor(FALSE);
	}
}

void Win32Window::UnlockCursor()
{
	if (MouseLocked)
	{
		MouseLocked = false;
		SetCursorPos(MouseLockPos.x, MouseLockPos.y);
		::ShowCursor(TRUE);
	}
}

void Win32Window::Update()
{
	InvalidateRect(WindowHandle, nullptr, FALSE);
}

bool Win32Window::GetKeyState(EInputKey key)
{
	return ::GetKeyState((int)key) & 0x8000; // High bit (0x8000) means key is down, Low bit (0x0001) means key is sticky on (like Caps Lock, Num Lock, etc.)
}

Rect Win32Window::GetWindowFrame() const
{
	RECT box = {};
	GetWindowRect(WindowHandle, &box);
	double dpiscale = GetDpiScale();
	return Rect(box.left / dpiscale, box.top / dpiscale, box.right / dpiscale, box.bottom / dpiscale);
}

Size Win32Window::GetClientSize() const
{
	RECT box = {};
	GetClientRect(WindowHandle, &box);
	double dpiscale = GetDpiScale();
	return Size(box.right / dpiscale, box.bottom / dpiscale);
}

int Win32Window::GetPixelWidth() const
{
	RECT box = {};
	GetClientRect(WindowHandle, &box);
	return box.right;
}

int Win32Window::GetPixelHeight() const
{
	RECT box = {};
	GetClientRect(WindowHandle, &box);
	return box.bottom;
}

double Win32Window::GetDpiScale() const
{
	return GetDpiForWindow(WindowHandle) / 96.0;
}

std::string Win32Window::GetAvailableResolutions() const
{
	string result = "";
	std::vector<std::string> availableResolutions{};

	int modeNum = 0;
	DEVMODE deviceMode = {};
	deviceMode.dmSize = sizeof(DEVMODE);
	deviceMode.dmDriverExtra = 0;

	while (EnumDisplaySettings(nullptr, modeNum, &deviceMode) != 0)
	{
		std::string resolution = std::to_string(deviceMode.dmPelsWidth) + "x" + std::to_string(deviceMode.dmPelsHeight);

		// Skip over the current resolution if it is already inserted
        // (in case of multiple refresh rates being available for the display)
		bool resolutionAlreadyAdded = false;
		for (auto res : availableResolutions)
		{
			if (resolution.compare(res) == 0)
			{
				resolutionAlreadyAdded = true;
				break;
			}
		}
		if (resolutionAlreadyAdded)
			continue;

		// Add the resolution, as it is not added before
		availableResolutions.push_back(resolution);
		modeNum++;
	}

	// "Flatten" the resolutions list into a single string
	for (int i = 0 ; i < availableResolutions.size() ; i++)
	{
		result += availableResolutions[i];
		if (i < availableResolutions.size() - 1)
			result += " ";
	}

	return result;
}

LRESULT Win32Window::OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_INPUT)
	{
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
					WindowHost->OnWindowRawMouseMove(rawinput->data.mouse.lLastX, rawinput->data.mouse.lLastY);
				}
			}
		}
		return DefWindowProc(WindowHandle, msg, wparam, lparam);
	}
	else if (msg == WM_PAINT)
	{
		ValidateRect(WindowHandle, nullptr);
		WindowHost->OnWindowPaint();
	}
	else if (msg == WM_ACTIVATE)
	{
		WindowHost->OnWindowActivated();
	}
	else if (msg == WM_MOUSEMOVE)
	{
		if (MouseLocked)
		{
			RECT box = {};
			GetClientRect(WindowHandle, &box);

			POINT center = {};
			center.x = box.right / 2;
			center.y = box.bottom / 2;
			ClientToScreen(WindowHandle, &center);

			SetCursorPos(center.x, center.y);
		}

		WindowHost->OnWindowMouseMove(GetLParamPos(lparam));
	}
	else if (msg == WM_LBUTTONDOWN)
	{
		WindowHost->OnWindowMouseDown(GetLParamPos(lparam), IK_LeftMouse);
	}
	else if (msg == WM_LBUTTONUP)
	{
		WindowHost->OnWindowMouseUp(GetLParamPos(lparam), IK_LeftMouse);
	}
	else if (msg == WM_MBUTTONDOWN)
	{
		WindowHost->OnWindowMouseDown(GetLParamPos(lparam), IK_MiddleMouse);
	}
	else if (msg == WM_MBUTTONUP)
	{
		WindowHost->OnWindowMouseUp(GetLParamPos(lparam), IK_MiddleMouse);
	}
	else if (msg == WM_RBUTTONDOWN)
	{
		WindowHost->OnWindowMouseDown(GetLParamPos(lparam), IK_RightMouse);
	}
	else if (msg == WM_RBUTTONUP)
	{
		WindowHost->OnWindowMouseUp(GetLParamPos(lparam), IK_RightMouse);
	}
	else if (msg == WM_MOUSEWHEEL)
	{
		double delta = GET_WHEEL_DELTA_WPARAM(wparam) / (double)WHEEL_DELTA;
		WindowHost->OnWindowMouseWheel(GetLParamPos(lparam), delta < 0.0 ? IK_MouseWheelDown : IK_MouseWheelUp);
	}
	else if (msg == WM_CHAR)
	{
		wchar_t buf[2] = { (wchar_t)wparam, 0 };
		WindowHost->OnWindowKeyChar(from_utf16(buf));
	}
	else if (msg == WM_KEYDOWN)
	{
		WindowHost->OnWindowKeyDown((EInputKey)wparam);
	}
	else if (msg == WM_KEYUP)
	{
		WindowHost->OnWindowKeyUp((EInputKey)wparam);
	}
	else if (msg == WM_SETFOCUS)
	{
		ShowCursor(FALSE);
	}
	else if (msg == WM_KILLFOCUS)
	{
		ShowCursor(TRUE);
	}
	else if (msg == WM_CLOSE)
	{
		WindowHost->OnWindowClose();
		return 0;
	}
	else if (msg == WM_SIZE)
	{
		WindowHost->OnWindowGeometryChanged();
	}

	return DefWindowProc(WindowHandle, msg, wparam, lparam);
}

Point Win32Window::GetLParamPos(LPARAM lparam) const
{
	double dpiscale = GetDpiScale();
	return Point(GET_X_LPARAM(lparam) / dpiscale, GET_Y_LPARAM(lparam) / dpiscale);
}

LRESULT Win32Window::WndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_CREATE)
	{
		CREATESTRUCT* createstruct = (CREATESTRUCT*)lparam;
		Win32Window* viewport = (Win32Window*)createstruct->lpCreateParams;
		viewport->WindowHandle = windowhandle;
		SetWindowLongPtr(windowhandle, GWLP_USERDATA, (LONG_PTR)viewport);
		return viewport->OnWindowMessage(msg, wparam, lparam);
	}
	else
	{
		Win32Window* viewport = (Win32Window*)GetWindowLongPtr(windowhandle, GWLP_USERDATA);
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

void Win32Window::ProcessEvents()
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

void Win32Window::RunLoop()
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

void Win32Window::ExitLoop()
{
	ExitRunLoop = true;
}

std::list<Win32Window*> Win32Window::Windows;
bool Win32Window::ExitRunLoop;
