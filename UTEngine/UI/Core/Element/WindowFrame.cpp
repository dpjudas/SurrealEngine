
#include "Precomp.h"
#include "WindowFrame.h"
#include "../View.h"
#include <cmath>

namespace
{
	std::wstring to_utf16(const std::string& str)
	{
		if (str.empty()) return {};
		int needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		std::wstring result;
		result.resize(needed);
		needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size());
		if (needed == 0)
			throw std::runtime_error("MultiByteToWideChar failed");
		return result;
	}
}

class WindowFrameImpl
{
public:
	WindowFrameImpl(WindowFrame *viewwindow) : viewwindow(viewwindow)
	{
		static struct RegisterWindowClass
		{
			RegisterWindowClass()
			{
				WNDCLASSEX classdesc = {};
				classdesc.cbSize = sizeof(WNDCLASSEX);
				classdesc.hInstance = GetModuleHandle(0);
				classdesc.style = CS_VREDRAW | CS_HREDRAW;
				classdesc.lpszClassName = L"WindowFrame";
				classdesc.lpfnWndProc = &WindowFrameImpl::wndProc;
				RegisterClassEx(&classdesc);
			}
		} registerClass;
	}

	~WindowFrameImpl()
	{
		destroyWindow();
	}

	double getDpiScale()
	{
		HDC dc = GetDC(windowHandle);
		double dpiscale = GetDeviceCaps(dc, LOGPIXELSX) / 96.0;
		ReleaseDC(windowHandle, dc);
		return dpiscale;
	}

	WindowFrame* viewwindow = nullptr;
	std::unique_ptr<View> root;

	DWORD exstyle = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW;
	DWORD style = WS_OVERLAPPEDWINDOW;
	std::string icon;
	std::string title;
	double width = 640;
	double height = 480;

	HWND windowHandle = 0;

	void createWindow()
	{
		if (!windowHandle)
		{
			HDC screenDC = GetDC(0);
			double dpiscale = GetDeviceCaps(screenDC, LOGPIXELSX) / 96.0;
			int screenWidth = GetDeviceCaps(screenDC, HORZRES);
			int screenHeight = GetDeviceCaps(screenDC, VERTRES);
			ReleaseDC(0, screenDC);
			int scaledWidth = (int)std::round(dpiscale * width);
			int scaledHeight = (int)std::round(dpiscale * height);
			int x = (screenWidth - scaledWidth) / 2;
			int y = (screenHeight - scaledHeight) / 2;

			if (CreateWindowEx(exstyle, L"WindowFrame", to_utf16(title).c_str(), style, x, y, scaledWidth, scaledHeight, 0, 0, GetModuleHandle(0), this) == 0)
				throw std::runtime_error("CreateWindowEx failed!");
		}
	}

	void destroyWindow()
	{
		if (windowHandle)
			DestroyWindow(windowHandle);
	}

	LRESULT onWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg == WM_SIZE)
		{
			RECT box = {};
			if (GetWindowRect(windowHandle, &box))
			{
				double dpiscale = getDpiScale();
				width = (box.right - box.left) / dpiscale;
				height = (box.bottom - box.top) / dpiscale;
			}
		}
		return DefWindowProc(windowHandle, msg, wparam, lparam);
	}

	static LRESULT CALLBACK wndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg == WM_CREATE)
		{
			CREATESTRUCT* createstruct = (CREATESTRUCT*)lparam;
			WindowFrameImpl* viewport = (WindowFrameImpl*)createstruct->lpCreateParams;
			viewport->windowHandle = windowhandle;
			SetWindowLongPtr(windowhandle, GWLP_USERDATA, (LONG_PTR)viewport);
			return viewport->onWindowMessage(msg, wparam, lparam);
		}
		else
		{
			WindowFrameImpl* viewport = (WindowFrameImpl*)GetWindowLongPtr(windowhandle, GWLP_USERDATA);
			if (viewport)
			{
				LRESULT result = viewport->onWindowMessage(msg, wparam, lparam);
				if (msg == WM_DESTROY)
				{
					SetWindowLongPtr(windowhandle, GWLP_USERDATA, 0);
					viewport->windowHandle = 0;
				}
				return result;
			}
			else
			{
				return DefWindowProc(windowhandle, msg, wparam, lparam);
			}
		}
	}
};

WindowFrame::WindowFrame() : impl(std::make_unique<WindowFrameImpl>(this))
{
}

WindowFrame::~WindowFrame()
{
}

void WindowFrame::setIcon(const std::string& src)
{
	impl->icon = src;
}

void WindowFrame::setTitle(const std::string& title)
{
	impl->title = title;
	if (impl->windowHandle)
	{
		SetWindowText(impl->windowHandle, to_utf16(title).c_str());
	}
}

void WindowFrame::setSize(double width, double height)
{
	if (impl->windowHandle)
	{
		double dpiscale = impl->getDpiScale();
		SetWindowPos(impl->windowHandle, 0, 0, 0, (int)std::round(width * dpiscale), (int)std::round(height * dpiscale), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
	else
	{
		impl->width = width;
		impl->height = height;
	}
}

void WindowFrame::show()
{
	if (!impl->windowHandle)
	{
		impl->createWindow();
		ShowWindow(impl->windowHandle, SW_SHOW);
	}
}

void WindowFrame::hide()
{
	if (impl->windowHandle)
	{
		ShowWindow(impl->windowHandle, SW_HIDE);
	}
}

void WindowFrame::setContentView(std::unique_ptr<View> view)
{
	impl->root = std::move(view);
}

View* WindowFrame::contentView()
{
	return impl->root.get();
}

void WindowFrame::setNeedsRender()
{
	if (impl->windowHandle)
		InvalidateRect(impl->windowHandle, nullptr, FALSE);
}
