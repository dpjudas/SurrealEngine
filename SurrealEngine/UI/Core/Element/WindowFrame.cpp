
#include "Precomp.h"
#include "WindowFrame.h"
#include "Canvas.h"
#include "../View.h"
#include "../../../UTF16.h"
#include <cmath>

#ifdef WIN32
#include <windowsx.h>

#ifdef _MSC_VER
#pragma comment(lib, "Gdiplus.lib")
#endif

struct InitGdiPlus
{
	InitGdiPlus()
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput = {};
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
	}
	~InitGdiPlus()
	{
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
	ULONG_PTR gdiplusToken = 0;
};

class GdiplusCanvas : public Canvas
{
public:
	GdiplusCanvas(HDC dc) : graphics(dc)
	{
		dpiscale = GetDeviceCaps(dc, LOGPIXELSX) / 96.0;

		double fontSize = 12.0;
		LOGFONT desc = {};
		desc.lfHeight = -(int)std::round(dpiscale * fontSize);
		desc.lfWeight = FW_NORMAL;
		desc.lfCharSet = DEFAULT_CHARSET;
		desc.lfOutPrecision = OUT_DEFAULT_PRECIS;
		desc.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		desc.lfQuality = DEFAULT_QUALITY;
		desc.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy_s(desc.lfFaceName, L"Segoe UI");
		font.reset(new Gdiplus::Font(dc, &desc));
	}

	Point getOrigin() override { return origin; }
	void setOrigin(const Point& newOrigin) override { origin = newOrigin; }

	void pushClip(const Rect& box)
	{
		graphics.SetClip(Gdiplus::Rect(toScreen(origin.x + box.x), toScreen(origin.y + box.y), toScreen(box.width), toScreen(box.height)));
		//graphics.IntersectClip(Gdiplus::Rect(toScreen(origin.x + box.x), toScreen(origin.y + box.y), toScreen(box.width), toScreen(box.height)));
	}

	void popClip()
	{
		// This isn't really correct. Cba to code this properly. Too boring! Plus gdiplus shouldn't be used anyway!
		graphics.ResetClip();
	}

	void fillRect(const Rect& box, const Colorf& color) override
	{
		Gdiplus::SolidBrush brush(toGdiColor(color));
		graphics.FillRectangle(&brush, Gdiplus::Rect(toScreen(origin.x + box.x), toScreen(origin.y + box.y), toScreen(box.width), toScreen(box.height)));
	}

	void drawText(const Point& pos, const Colorf& color, const std::string& text) override
	{
		Gdiplus::SolidBrush brush(toGdiColor(color));
		std::wstring text16 = to_utf16(text);
		graphics.DrawString(text16.data(), (int)text16.size(), font.get(), Gdiplus::PointF((float)toScreen(origin.x + pos.x), (float)toScreen(origin.y + pos.y)), &brush);
	}

	Rect measureText(const std::string& text) override
	{
		std::wstring text16 = to_utf16(text);
		Gdiplus::RectF box;
		graphics.MeasureString(text16.data(), (int)text16.size(), font.get(), Gdiplus::PointF(0.0f, 0.0f), &box);
		return Rect(box.X / dpiscale, box.Y / dpiscale, box.Width / dpiscale, box.Height / dpiscale);
	}

	int toScreen(double v) const
	{
		return (int)std::round(dpiscale * v);
	}

	static float clamp(float v, float minval, float maxval)
	{
		return std::max(std::min(v, maxval), minval);
	}

	static Gdiplus::Color toGdiColor(const Colorf& color)
	{
		return Gdiplus::Color(
			(int)clamp(color.a * 255.0f, 0.0f, 255.0f),
			(int)clamp(color.r * 255.0f, 0.0f, 255.0f),
			(int)clamp(color.g * 255.0f, 0.0f, 255.0f),
			(int)clamp(color.b * 255.0f, 0.0f, 255.0f)
		);
	}

	double dpiscale = 1.0f;
	Gdiplus::Graphics graphics;
	Point origin;

	std::unique_ptr<Gdiplus::Font> font;
};

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
				classdesc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
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

	InitGdiPlus initGdiplus;

	WindowFrame* viewwindow = nullptr;
	std::unique_ptr<View> root;
	std::map<std::set<std::string>, std::unique_ptr<ElementStyle>> styleCache;

	Element* focusElement = nullptr;

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

	void dispatchMouseButtonEvent(std::string name, int button, WPARAM wparam, LPARAM lparam, int detail)
	{
		double dpiscale = getDpiScale();
		double x = GET_X_LPARAM(lparam) / dpiscale;
		double y = GET_Y_LPARAM(lparam) / dpiscale;
		Element* element = root->element->findElementAt({ x, y });
		if (element)
		{
			Event e;
			e.clientX = x;
			e.clientY = y;
			e.button = button;
			e.detail = detail;
			element->dispatchEvent(name, &e);
		}
	}

	void dispatchMouseWheelEvent(std::string name, int dx, int dy, WPARAM wparam, LPARAM lparam)
	{
		POINT pt;
		pt.x = GET_X_LPARAM(lparam);
		pt.y = GET_Y_LPARAM(lparam);
		ScreenToClient(windowHandle, &pt);
		double dpiscale = getDpiScale();
		double x = pt.x / dpiscale;
		double y = pt.y / dpiscale;
		Element* element = root->element->findElementAt({ x, y });
		if (element)
		{
			Event e;
			e.clientX = x;
			e.clientY = y;
			e.deltaX = -dx;
			e.deltaY = -dy;
			element->dispatchEvent(name, &e);
			if (!e.preventDefaultFlag)
			{
				while (element)
				{
					if (element->overflow())
					{
						element->scrollBy(-dx, -dy);
						break;
					}
					element = element->parent();
				}
			}
		}
	}

	LRESULT onWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg == WM_MOUSEMOVE)
		{
			if (root)
			{
				double dpiscale = getDpiScale();
				int x = GET_X_LPARAM(lparam);
				int y = GET_Y_LPARAM(lparam);
				Element* element = root->element->findElementAt({ x / dpiscale, y / dpiscale });
				if (element)
				{
					// To do: element->computedCursor()
					// To do: apply hover pseudo class
					SetCursor(LoadCursor(0, IDC_ARROW));
				}
			}
			return 0;
		}
		else if (msg == WM_PAINT)
		{
			PAINTSTRUCT paintStruct = {};
			HDC dc = BeginPaint(windowHandle, &paintStruct);
			if (dc)
			{
				RECT box = {};
				GetClientRect(windowHandle, &box);
				HBITMAP backbuffer = CreateCompatibleBitmap(dc, box.right, box.bottom);
				HDC backbufferDC = CreateCompatibleDC(dc);
				HBITMAP oldBitmap = (HBITMAP)SelectObject(backbufferDC, backbuffer);

				if (root)
				{
					GdiplusCanvas canvas(backbufferDC);
					root->element->render(&canvas);
				}

				BitBlt(dc, 0, 0, box.right, box.bottom, backbufferDC, 0, 0, SRCCOPY);
				SelectObject(backbufferDC, oldBitmap);
				DeleteDC(backbufferDC);
				DeleteObject(backbuffer);
				EndPaint(windowHandle, &paintStruct);
			}
			return 0;
		}
		else if (msg == WM_SETFOCUS)
		{
			return 0;
		}
		else if (msg == WM_KILLFOCUS)
		{
			return 0;
		}
		else if (msg == WM_CHAR)
		{
			return 0;
		}
		else if (msg == WM_KEYDOWN)
		{
			if (focusElement)
			{
				Event e;
				e.keyCode = (int)wparam;
				focusElement->dispatchEvent("keydown", &e);
			}
			return 0;
		}
		else if (msg == WM_KEYUP)
		{
			if (focusElement)
			{
				Event e;
				e.keyCode = (int)wparam;
				focusElement->dispatchEvent("keyup", &e);
			}
			return 0;
		}
		else if (msg == WM_LBUTTONDOWN)
		{
			dispatchMouseButtonEvent("mousedown", 0, wparam, lparam, 2);
			return 0;
		}
		else if (msg == WM_MBUTTONDOWN)
		{
			dispatchMouseButtonEvent("mousedown", 1, wparam, lparam, 2);
			return 0;
		}
		else if (msg == WM_RBUTTONDOWN)
		{
			dispatchMouseButtonEvent("mousedown", 2, wparam, lparam, 2);
			return 0;
		}
		else if (msg == WM_LBUTTONUP)
		{
			dispatchMouseButtonEvent("mouseup", 0, wparam, lparam, 2);
			dispatchMouseButtonEvent("click", 0, wparam, lparam, 1);
			return 0;
		}
		else if (msg == WM_MBUTTONUP)
		{
			dispatchMouseButtonEvent("mouseup", 1, wparam, lparam, 2);
			dispatchMouseButtonEvent("click", 1, wparam, lparam, 1);
			return 0;
		}
		else if (msg == WM_RBUTTONUP)
		{
			dispatchMouseButtonEvent("mouseup", 2, wparam, lparam, 2);
			dispatchMouseButtonEvent("click", 2, wparam, lparam, 1);
			return 0;
		}
		else if (msg == WM_LBUTTONDBLCLK)
		{
			dispatchMouseButtonEvent("click", 0, wparam, lparam, 2);
			dispatchMouseButtonEvent("dblclick", 0, wparam, lparam, 2);
			return 0;
		}
		else if (msg == WM_MBUTTONDBLCLK)
		{
			dispatchMouseButtonEvent("click", 1, wparam, lparam, 2);
			dispatchMouseButtonEvent("dblclick", 1, wparam, lparam, 2);
			return 0;
		}
		else if (msg == WM_RBUTTONDBLCLK)
		{
			dispatchMouseButtonEvent("click", 2, wparam, lparam, 2);
			dispatchMouseButtonEvent("dblclick", 2, wparam, lparam, 2);
			return 0;
		}
		else if (msg == WM_MOUSEWHEEL)
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wparam);
			dispatchMouseWheelEvent("wheel", 0, delta, wparam, lparam);
			return 0;
		}
		else if (msg == WM_MOUSEHWHEEL)
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wparam);
			dispatchMouseWheelEvent("wheel", delta, 0, wparam, lparam);
			return 0;
		}
		else if (msg == WM_SIZE)
		{
			RECT box = {};
			if (GetWindowRect(windowHandle, &box))
			{
				double dpiscale = getDpiScale();
				width = (box.right - box.left) / dpiscale;
				height = (box.bottom - box.top) / dpiscale;
				if (root)
				{
					GetClientRect(windowHandle, &box);
					ElementGeometry g;
					g.contentWidth = box.right / dpiscale;
					g.contentHeight = box.bottom / dpiscale;
					root->element->setGeometry(g);
				}
			}
		}
		else if (msg == WM_CLOSE)
		{
			viewwindow->onClose();
			return 0;
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
	if (impl->root)
		impl->root->element->viewwindow = nullptr;
	impl->root = std::move(view);
	impl->root->element->viewwindow = this;
	if (impl->root)
	{
		ElementGeometry g;
		g.contentWidth = impl->width;
		g.contentHeight = impl->height;
		impl->root->element->setGeometry(g);
	}
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

ElementStyle* WindowFrame::getStyle(Element* element)
{
	auto& style = impl->styleCache[element->classes];
	if (!style)
		style = std::make_unique<ElementStyle>(element->classes);
	return style.get();
}

void WindowFrame::setFocus(Element* element)
{
	impl->focusElement = element;
}

void WindowFrame::killFocus(Element* element)
{
	if (impl->focusElement == element)
		impl->focusElement = nullptr;
}

#else

class WindowFrameImpl
{
public:
	WindowFrameImpl(WindowFrame *viewwindow) : viewwindow(viewwindow)
	{
	}

	WindowFrame* viewwindow = nullptr;
	std::unique_ptr<View> root;

	std::string icon;
	std::string title;
	double width = 640;
	double height = 480;
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
}

void WindowFrame::setSize(double width, double height)
{
	impl->width = width;
	impl->height = height;
}

void WindowFrame::show()
{
}

void WindowFrame::hide()
{
}

void WindowFrame::setContentView(std::unique_ptr<View> view)
{
	if (impl->root)
		impl->root->element->viewwindow = nullptr;
	impl->root = std::move(view);
	impl->root->element->viewwindow = this;
	if (impl->root)
	{
		ElementGeometry g;
		g.contentWidth = impl->width;
		g.contentHeight = impl->height;
		impl->root->element->setGeometry(g);
	}
}

View* WindowFrame::contentView()
{
	return impl->root.get();
}

void WindowFrame::setNeedsRender()
{
}

ElementStyle* WindowFrame::getStyle(Element* element)
{
	return nullptr;
}

void WindowFrame::setFocus(Element* element)
{
}

void WindowFrame::killFocus(Element* element)
{
}

#endif
