
#pragma once

#include "win32_util.h"

class OpenGLCreationHelper
{
public:
	OpenGLCreationHelper(HWND window, HDC hdc);
	~OpenGLCreationHelper();

	void set_pixel_format();
	HGLRC create_opengl3_context(HGLRC share_context, int major_version, int minor_version);

private:
	void set_active();
	void reset_active();

	HWND window = 0;
	HDC hdc = 0;
	HWND query_window = 0;
	HDC query_dc = 0;
	HGLRC query_context = 0;

	typedef HGLRC (WINAPI *ptr_wglCreateContextAttribsARB)(HDC, HGLRC, const int *);

	typedef BOOL (WINAPI *ptr_wglGetPixelFormatAttribivEXT)(HDC, int, int, UINT, int *, int *);
	typedef BOOL (WINAPI *ptr_wglGetPixelFormatAttribfvEXT)(HDC, int, int, UINT, int *, FLOAT *);
	typedef BOOL (WINAPI *ptr_wglChoosePixelFormatEXT)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);
};

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define ERROR_INVALID_VERSION_ARB               0x2095

#define WGL_NUMBER_PIXEL_FORMATS             0x2000
#define WGL_DRAW_TO_WINDOW                   0x2001
#define WGL_DRAW_TO_BITMAP                   0x2002
#define WGL_ACCELERATION                     0x2003
#define WGL_NEED_PALETTE                     0x2004
#define WGL_NEED_SYSTEM_PALETTE              0x2005
#define WGL_SWAP_LAYER_BUFFERS               0x2006
#define WGL_SWAP_METHOD                      0x2007
#define WGL_NUMBER_OVERLAYS                  0x2008
#define WGL_NUMBER_UNDERLAYS                 0x2009
#define WGL_TRANSPARENT                      0x200A
#define WGL_TRANSPARENT_VALUE                0x200B
#define WGL_SHARE_DEPTH                      0x200C
#define WGL_SHARE_STENCIL                    0x200D
#define WGL_SHARE_ACCUM                      0x200E
#define WGL_SUPPORT_GDI                      0x200F
#define WGL_SUPPORT_OPENGL                   0x2010
#define WGL_DOUBLE_BUFFER                    0x2011
#define WGL_STEREO                           0x2012
#define WGL_PIXEL_TYPE                       0x2013
#define WGL_COLOR_BITS                       0x2014
#define WGL_RED_BITS                         0x2015
#define WGL_RED_SHIFT                        0x2016
#define WGL_GREEN_BITS                       0x2017
#define WGL_GREEN_SHIFT                      0x2018
#define WGL_BLUE_BITS                        0x2019
#define WGL_BLUE_SHIFT                       0x201A
#define WGL_ALPHA_BITS                       0x201B
#define WGL_ALPHA_SHIFT                      0x201C
#define WGL_ACCUM_BITS                       0x201D
#define WGL_ACCUM_RED_BITS                   0x201E
#define WGL_ACCUM_GREEN_BITS                 0x201F
#define WGL_ACCUM_BLUE_BITS                  0x2020
#define WGL_ACCUM_ALPHA_BITS                 0x2021
#define WGL_DEPTH_BITS                       0x2022
#define WGL_STENCIL_BITS                     0x2023
#define WGL_AUX_BUFFERS                      0x2024
#define WGL_NO_ACCELERATION                  0x2025
#define WGL_GENERIC_ACCELERATION             0x2026
#define WGL_FULL_ACCELERATION                0x2027
#define WGL_SWAP_EXCHANGE                    0x2028
#define WGL_SWAP_COPY                        0x2029
#define WGL_SWAP_UNDEFINED                   0x202A
#define WGL_TYPE_RGBA                        0x202B
#define WGL_TYPE_COLORINDEX                  0x202C
#define WGL_SAMPLE_BUFFERS                   0x2041
#define WGL_SAMPLES                          0x2042
