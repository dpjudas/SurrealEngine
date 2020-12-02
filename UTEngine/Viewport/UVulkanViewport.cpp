
#include "Precomp.h"
#include "UVulkanViewport.h"
#include "RenderDevice/UVulkanRenderDevice.h"
#include "Engine.h"

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

struct ReceivedWindowMessage
{
	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
};

class UVulkanViewport : public UViewport
{
public:
	UVulkanViewport(Engine* engine);
	~UVulkanViewport();

	// UViewport interface.
	void OpenWindow(int width, int height, bool fullscreen) override;
	void CloseWindow() override;
	void* GetWindow() override;
	URenderDevice* GetRenderDevice() override { return RenderDevice.get(); }
	void Tick() override;

	void PauseGame();
	void ResumeGame();

	LRESULT OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK WndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam);

	Engine* engine = nullptr;

	// Variables.
	HWND WindowHandle = 0;
	bool Fullscreen = false;
	bool Paused = false;

	// Mouse.
	int MouseMoveX = 0;
	int MouseMoveY = 0;

	std::vector<ReceivedWindowMessage> ReceivedMessages;

	std::unique_ptr<URenderDevice> RenderDevice;
};

std::unique_ptr<UViewport> UViewport::Create(Engine* engine)
{
	return std::make_unique<UVulkanViewport>(engine);
}

UVulkanViewport::UVulkanViewport(Engine* engine) : engine(engine)
{
	WNDCLASSEX classdesc = {};
	classdesc.cbSize = sizeof(WNDCLASSEX);
	classdesc.hInstance = GetModuleHandle(0);
	classdesc.style = CS_VREDRAW | CS_HREDRAW;
	classdesc.lpszClassName = L"UVulkanViewport";
	classdesc.lpfnWndProc = &UVulkanViewport::WndProc;
	RegisterClassEx(&classdesc);

	CreateWindowEx(WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW, L"UVulkanViewport", L"Unreal Tournament", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, 0, 0, GetModuleHandle(0), this);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
	rid.usUsage = HID_USAGE_GENERIC_MOUSE;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = WindowHandle;
	BOOL result = RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
}

UVulkanViewport::~UVulkanViewport()
{
	RenderDevice.reset();
	if (WindowHandle)
	{
		DestroyWindow(WindowHandle);
		WindowHandle = 0;
	}
}

void UVulkanViewport::OpenWindow(int width, int height, bool fullscreen)
{
	if (!WindowHandle)
		return;

	if (fullscreen)
	{
		bool visible = IsWindowVisible(WindowHandle);
		SetWindowLongPtr(WindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		SetWindowLongPtr(WindowHandle, GWL_STYLE, WS_OVERLAPPED | (GetWindowLongPtr(WindowHandle, GWL_STYLE) & WS_VISIBLE));
		SetWindowPos(WindowHandle, 0, 0, 0, width, height, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowLongPtr(WindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW);
		SetWindowLongPtr(WindowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW | (GetWindowLongPtr(WindowHandle, GWL_STYLE) & WS_VISIBLE));
		SetWindowPos(WindowHandle, 0, 0, 0, width, height, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
	}

	Fullscreen = fullscreen;
	SizeX = width;
	SizeY = height;

	// Create rendering device.
	if (!RenderDevice)
		RenderDevice = URenderDevice::Create(this);

	if (!IsWindowVisible(WindowHandle))
	{
		ShowWindow(WindowHandle, SW_SHOW);
		SetActiveWindow(WindowHandle);
	}
}

void UVulkanViewport::CloseWindow()
{
	if (WindowHandle && IsWindowVisible(WindowHandle))
		ShowWindow(WindowHandle, SW_HIDE);
}

void* UVulkanViewport::GetWindow()
{
	return (void*)WindowHandle;
}

void UVulkanViewport::Tick()
{
	if (!WindowHandle)
		return;

	MSG msg;
	while (WindowHandle && PeekMessage(&msg, WindowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	std::vector<ReceivedWindowMessage> messages;
	messages.swap(ReceivedMessages);

	for (const ReceivedWindowMessage& e : messages)
	{
		switch (e.msg)
		{
		case WM_CHAR:
			engine->Key(this, std::string(1, (char)e.wparam));
			break;
		case WM_KEYDOWN:
			engine->InputEvent(this, (EInputKey)e.wparam, IST_Press);
			break;
		case WM_KEYUP:
			engine->InputEvent(this, (EInputKey)e.wparam, IST_Release);
			break;
		case WM_LBUTTONDOWN:
			engine->InputEvent(this, IK_LeftMouse, IST_Press);
			break;
		case WM_MBUTTONDOWN:
			engine->InputEvent(this, IK_MiddleMouse, IST_Press);
			break;
		case WM_RBUTTONDOWN:
			engine->InputEvent(this, IK_RightMouse, IST_Press);
			break;
		case WM_LBUTTONUP:
			engine->InputEvent(this, IK_LeftMouse, IST_Release);
			break;
		case WM_MBUTTONUP:
			engine->InputEvent(this, IK_MiddleMouse, IST_Release);
			break;
		case WM_RBUTTONUP:
			engine->InputEvent(this, IK_RightMouse, IST_Release);
			break;
		case WM_MOUSEWHEEL:
			if (GET_WHEEL_DELTA_WPARAM(msg.wParam) != 0)
			{
				EInputKey key = GET_WHEEL_DELTA_WPARAM(msg.wParam) > 0 ? IK_MouseWheelUp : IK_MouseWheelDown;
				engine->InputEvent(this, key, IST_Press);
				engine->InputEvent(this, key, IST_Release);
			}
			break;
		case WM_SETFOCUS:
			ResumeGame();
			break;
		case WM_KILLFOCUS:
			PauseGame();
			break;
		}
	}

	if (Paused)
	{
		MouseMoveX = 0;
		MouseMoveY = 0;
		return;
	}

	// Deliver mouse behavior to the engine.
	if (MouseMoveX != 0 || MouseMoveY != 0)
	{
		int DX = MouseMoveX;
		int DY = MouseMoveY;
		MouseMoveX = 0;
		MouseMoveY = 0;

		// Send to input subsystem.
		if (DX)
			engine->InputEvent(this, IK_MouseX, IST_Axis, +DX);
		if (DY)
			engine->InputEvent(this, IK_MouseY, IST_Axis, -DY);
	}
}

void UVulkanViewport::PauseGame()
{
	if (!Paused)
	{
		Paused = true;
		engine->SetPause(true);
	}
}

void UVulkanViewport::ResumeGame()
{
	if (Paused)
	{
		Paused = false;
		engine->SetPause(false);
	}
}

LRESULT UVulkanViewport::OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_CREATE ||
		msg == WM_DESTROY ||
		msg == WM_SHOWWINDOW ||
		msg == WM_KILLFOCUS ||
		msg == WM_LBUTTONDOWN ||
		msg == WM_LBUTTONUP ||
		msg == WM_MBUTTONDOWN ||
		msg == WM_MBUTTONUP ||
		msg == WM_RBUTTONDOWN ||
		msg == WM_RBUTTONUP ||
		msg == WM_MOUSEWHEEL ||
		msg == WM_CHAR ||
		msg == WM_KEYDOWN ||
		msg == WM_KEYUP ||
		msg == WM_SETFOCUS ||
		msg == WM_KILLFOCUS)
	{
		ReceivedMessages.push_back({ msg, wparam, lparam });
	}

	if (msg == WM_INPUT)
	{
		HRAWINPUT handle = (HRAWINPUT)lparam;
		UINT size = 0;
		UINT result = GetRawInputData(handle, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));
		if (result == 0 && size > 0)
		{
			std::vector<uint8_t*> buffer(size);
			result = GetRawInputData(handle, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));
			if (result >= 0)
			{
				RAWINPUT* rawinput = (RAWINPUT*)buffer.data();
				if (rawinput->header.dwType == RIM_TYPEMOUSE)
				{
					MouseMoveX += rawinput->data.mouse.lLastX;
					MouseMoveY += rawinput->data.mouse.lLastY;
				}
			}
		}
		return DefWindowProc(WindowHandle, msg, wparam, lparam);
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
		engine->WindowClose(this);
		return 0;
	}

	return DefWindowProc(WindowHandle, msg, wparam, lparam);
}

LRESULT UVulkanViewport::WndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_CREATE)
	{
		CREATESTRUCT* createstruct = (CREATESTRUCT*)lparam;
		UVulkanViewport* viewport = (UVulkanViewport*)createstruct->lpCreateParams;
		viewport->WindowHandle = windowhandle;
		SetWindowLongPtr(windowhandle, GWLP_USERDATA, (LONG_PTR)viewport);
		return viewport->OnWindowMessage(msg, wparam, lparam);
	}
	else
	{
		UVulkanViewport* viewport = (UVulkanViewport*)GetWindowLongPtr(windowhandle, GWLP_USERDATA);
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
