
#include "opengl_creation_helper.h"
#include <commctrl.h>
#include <stdexcept>
#include <vector>

#pragma comment(lib, "Opengl32.lib")

OpenGLCreationHelper::OpenGLCreationHelper(HWND window, HDC hdc) : window(window), hdc(hdc)
{
	WINDOWINFO window_info = {};
	window_info.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(window, &window_info);

	query_window = CreateWindowEx(
		0,
		WC_STATIC,
		TEXT(""),
		WS_CHILD,
		window_info.rcWindow.left,
		window_info.rcWindow.top,
		window_info.rcWindow.right - window_info.rcWindow.left,
		window_info.rcWindow.bottom - window_info.rcWindow.top,
		window, 0, GetModuleHandle(0), 0);
	if (query_window == 0)
		throw std::runtime_error("Unable to create OpenGL creation query window");

	query_dc = GetDC(query_window);
	if (query_dc == 0)
	{
		DestroyWindow(query_window);
		throw std::runtime_error("Unable to retrieve OpenGL creation query device context");
	}

	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;

	int pixelformat = ChoosePixelFormat(query_dc, &pfd);
	SetPixelFormat(query_dc, pixelformat, &pfd);

	query_context = wglCreateContext(query_dc);
	if (query_context == 0)
	{
		DeleteDC(query_dc);
		DestroyWindow(query_window);
		throw std::runtime_error("Unable to create OpenGL context for creation query window");
	}
}

OpenGLCreationHelper::~OpenGLCreationHelper()
{
	wglDeleteContext(query_context);
	DeleteDC(query_dc);
	DestroyWindow(query_window);
}

void OpenGLCreationHelper::set_pixel_format()
{
	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags |= PFD_DOUBLEBUFFER;
	pfd.cColorBits = 24;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 0;
	pfd.cDepthBits = 0;
	pfd.cStencilBits = 0;

	int pixelformat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pixelformat, &pfd);
}

HGLRC OpenGLCreationHelper::create_opengl3_context(HGLRC share_context, int major_version, int minor_version)
{
	set_active();
	ptr_wglCreateContextAttribsARB wglCreateContextAttribsARB = (ptr_wglCreateContextAttribsARB) wglGetProcAddress("wglCreateContextAttribsARB");
	reset_active();

	HGLRC opengl3_context = 0;
	if (wglCreateContextAttribsARB)
	{
		std::vector<int> int_attributes;

		int_attributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
		int_attributes.push_back(major_version);
		int_attributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
		int_attributes.push_back(minor_version);

		int_attributes.push_back(0x2094);	// WGL_CONTEXT_FLAGS_ARB
		int flags = 0;
		// flags |= 0x1; // WGL_CONTEXT_DEBUG_BIT_ARB
		flags |= 0x2; // WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
		int_attributes.push_back(flags);

		int_attributes.push_back(0x9126);	// WGL_CONTEXT_PROFILE_MASK_ARB
		flags = 0;
		flags |= 0x1; // WGL_CONTEXT_CORE_PROFILE_BIT_ARB
		// flags |= 0x2; // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
		int_attributes.push_back(flags);

		int_attributes.push_back(0);

		opengl3_context = wglCreateContextAttribsARB(hdc, share_context, &int_attributes[0]);
	}
	return opengl3_context;
}

void OpenGLCreationHelper::set_active()
{
	wglMakeCurrent(query_dc, query_context);
}

void OpenGLCreationHelper::reset_active()
{
	wglMakeCurrent(0, 0);
}
