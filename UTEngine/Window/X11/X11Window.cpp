/*
**  Copyright (c) Mark Page, Chu Chin Kuan, Magnus Norddahl, Harry Storbacka
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
*/

#include "Precomp.h"
#include "X11Window.h"
#include "RenderDevice/RenderDevice.h"
#include "RenderDevice/Vulkan/VulkanDevice.h"
#include "Engine.h"
#include <dlfcn.h>
#include <string.h>
#include <algorithm>
#include <cstdio>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>

#ifndef _NET_WM_STATE_REMOVE
#define _NET_WM_STATE_REMOVE  0
#define _NET_WM_STATE_ADD     1
#define _NET_WM_STATE_TOGGLE  2
#endif

class X11Display
{
public:
	Display* display = nullptr;

	X11Display()
	{
		if (XInitThreads() == 0)
			throw std::runtime_error("XInitThreads failed");

		display = XOpenDisplay(nullptr);
		if (!display)
			throw std::runtime_error("Could not open X11 display connection");
	}

	~X11Display()
	{
		// XCloseDisplay(display);
	}

	static Display* GetDisplay()
	{
		static X11Display disp;
		return disp.display;
	}
};

X11Window::X11Window(Engine* engine) : engine(engine)
{
	display = X11Display::GetDisplay();
	screen = DefaultScreen(display);
	atoms = X11Atoms(display);
}

X11Window::~X11Window()
{
	CloseWindow();
}

void X11Window::OpenWindow(int width, int height, bool fullscreen)
{
	CloseWindow();

	auto createSurfaceWindow = [&](VkPhysicalDevice physDevice, uint32_t queueFamilyIndex) -> std::pair<Display*,Window>
	{
		XVisualInfo visualTemplate = { 0 };
		visualTemplate.screen = screen;
		visualTemplate.c_class = TrueColor;
		int numVisuals = 0;
		XVisualInfo* visuals = XGetVisualInfo(display, VisualScreenMask | VisualClassMask, &visualTemplate, &numVisuals);
		XVisualInfo* foundVisual = nullptr;
		for (int i = 0; i < numVisuals; i++)
		{
			if (vkGetPhysicalDeviceXlibPresentationSupportKHR(physDevice, queueFamilyIndex, display, visuals[i].visualid))
			{
				foundVisual = &visuals[i];
				break;
			}
		}
		XVisualInfo visualInfo;
		if (foundVisual)
			visualInfo = *foundVisual;
		XFree(visuals);
		if (!foundVisual)
			throw std::runtime_error("Could not find a visual compatible with the selected vulkan device");

		Status result;

		int disp_width_px = XDisplayWidth(display, screen);
		int disp_height_px = XDisplayHeight(display, screen);
		int disp_width_mm = XDisplayWidthMM(display, screen);

		// Get DPI of screen or use 96 if Xlib doesn't have a value
		double dpi = (disp_width_mm < 24) ? 96.0 : (25.4 * static_cast<double>(disp_width_px) / static_cast<double>(disp_width_mm));
		dpiscale = dpi / 96.0;

		int w = (int)std::round(width * dpiscale);
		int h = (int)std::round(height * dpiscale);

		if (fullscreen)
		{
			w = disp_width_px;
			h = disp_height_px;
		}

		int x = (disp_width_px - w) / 2;
		int y = (disp_height_px - h) / 2;

		SizeX = w;
		SizeY = h;

		size_hints = XAllocSizeHints();
		if (!size_hints)
			throw std::runtime_error("Failed to allocate X11 XSizeHints structure");

		size_hints->flags = PMinSize | PResizeInc | PBaseSize | PWinGravity | USSize | USPosition;
		size_hints->min_width   = 8;
		size_hints->min_height  = 8;
		size_hints->max_width   = 0;
		size_hints->max_height  = 0;
		size_hints->width_inc   = 1;
		size_hints->height_inc  = 1;
		size_hints->base_width  = w;
		size_hints->base_height = h;
		size_hints->win_gravity = NorthWestGravity;

		Window root_window = RootWindow(display, screen);
		colormap = XCreateColormap(display, root_window, visualInfo.visual, AllocNone);

		XSetWindowAttributes attr = {};
		attr.background_pixmap = None;
		attr.background_pixel = 0ul;
		attr.border_pixmap = CopyFromParent;
		attr.border_pixel = 0ul;
		attr.bit_gravity = ForgetGravity;
		attr.win_gravity = NorthWestGravity;
		attr.backing_store = NotUseful;
		attr.backing_planes = -1ul;
		attr.backing_pixel = 0ul;
		attr.save_under = False;
		attr.event_mask =
			KeyPressMask | KeyReleaseMask |
			ButtonPressMask | ButtonReleaseMask |
			EnterWindowMask | LeaveWindowMask | PointerMotionMask |
			KeymapStateMask | ExposureMask | StructureNotifyMask | FocusChangeMask | PropertyChangeMask;
		attr.do_not_propagate_mask = NoEventMask;
		attr.override_redirect = False;
		attr.colormap = colormap;
		attr.cursor = None;

		window = XCreateWindow(display, root_window, x, y, w, h, 0, visualInfo.depth, InputOutput, visualInfo.visual,
			CWBorderPixel | CWOverrideRedirect | CWSaveUnder | CWEventMask | CWColormap, &attr);
		if (!window)
			throw std::runtime_error("Could not create window");

		char data[8 * 8];
		memset(data, 0, 8 * 8);
		XColor black_color = { 0 };
		cursor_bitmap = XCreateBitmapFromData(display, window, data, 8, 8);
		hidden_cursor = XCreatePixmapCursor(display, cursor_bitmap, cursor_bitmap, &black_color, &black_color, 0,0);

		system_cursor = XCreateFontCursor(display, XC_left_ptr); // This is allowed to fail

		std::string title = "UTEngine";
		XSetStandardProperties(display, window, title.c_str(), title.c_str(), None, nullptr, 0, nullptr);

		{   // Inform the window manager who we are, so it can kill us if we're not good for its universe.
			Atom atom;
			int32_t pid = getpid();
			if (pid > 0)
			{
				atom = atoms.get_atom(display, "_NET_WM_PID", False);
				XChangeProperty(display, window, atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &pid, 1);
			}

			char hostname[256];
			if (gethostname(hostname, sizeof(hostname)) > -1)
			{
				hostname[255] = 0;
				atom = atoms.get_atom(display, "WM_CLIENT_MACHINE", False);
				XChangeProperty(display, window, atom, XA_STRING, 8, PropModeReplace, (unsigned char *) hostname, strlen(hostname));
			}
		}

		XSetWMNormalHints(display, window, size_hints);

		Atom protocol = atoms["WM_DELETE_WINDOW"];
		result = XSetWMProtocols(display, window, &protocol, 1);

		Bool supports_detectable_autorepeat = {};
		XkbSetDetectableAutoRepeat(display, True, &supports_detectable_autorepeat);

		if (atoms["_NET_WM_STATE"] == None && atoms["_NET_WM_STATE_FULLSCREEN"])
		{
			fullscreen = false; // Fullscreen not supported by WM
		}

		if (fullscreen)
		{
			Atom state = atoms["_NET_WM_STATE_FULLSCREEN"];
			XChangeProperty(display, window, atoms["_NET_WM_STATE"], XA_ATOM, 32, PropModeReplace, (unsigned char *)&state, 1);
			is_fullscreen = true;
		}

		return { display, window };
	};
	
	auto printLog = [](const char* type, const std::string& msg)
	{
		std::cout << "[" << type << "] " << msg.c_str() << std::endl;
	};

	int vk_device = 0;
	bool vk_debug = true;
	Device = std::make_unique<VulkanDevice>(createSurfaceWindow, vk_device, vk_debug, printLog);

	RendDevice = RenderDevice::Create(this);

	MapWindow();
	HideSystemCursor();
}

void X11Window::CloseWindow()
{
	RendDevice.reset();
	Device.reset();

	if (window)
	{
		XDestroyWindow(display, window);
		window = 0;
	}

	if (system_cursor)
	{
		XFreeCursor(display, system_cursor);
		system_cursor = 0;
	}

	if (hidden_cursor)
	{
		XFreeCursor(display, hidden_cursor);
		hidden_cursor = 0;
	}

	if (cursor_bitmap)
	{
		XFreePixmap(display, cursor_bitmap);
		cursor_bitmap = 0;
	}

	if (colormap)
	{
		XFreeColormap(display, colormap);
		colormap = 0;
	}

	if (size_hints)
	{
		XFree(size_hints);
		size_hints = nullptr;
	}
}

void* X11Window::GetDisplay()
{
	return display;
}

void* X11Window::GetWindow()
{
	return (void*)window;
}

void X11Window::Tick()
{
	XEvent event;
	while (XPending(display) > 0)
	{
		XNextEvent(display, &event);

		switch(event.type)
		{
		case ConfigureNotify:
		{
			// WindowX = event.xconfigure.x;
			// WindowY = event.xconfigure.y;
			// WindowSizeX = event.xconfigure.x + event.xconfigure.border_width * 2;
			// WindowSizeY = event.xconfigure.y + event.xconfigure.border_width * 2;
			SizeX = event.xconfigure.width;
			SizeY = event.xconfigure.height;
			// engine->OnGeometryChanged(this);
			break;
		}
		case ClientMessage:
		{	// handle window manager messages
			Atom WM_PROTOCOLS = atoms["WM_PROTOCOLS"];
			if (WM_PROTOCOLS == None)
			{
				break;
			}
			else if (event.xclient.message_type == WM_PROTOCOLS)
			{
				unsigned long protocol = event.xclient.data.l[0];
				if (protocol == None)
				{
					break;
				}

				Atom WM_DELETE_WINDOW = atoms["WM_DELETE_WINDOW"];
				Atom _NET_WM_PING = atoms["_NET_WM_PING"];

				if (protocol == WM_DELETE_WINDOW)
				{
					engine->WindowClose(this);
				}
				else if (protocol == _NET_WM_PING)
				{
					XSendEvent(display, RootWindow(display, screen), False, SubstructureNotifyMask | SubstructureRedirectMask, &event);
				}
			}
			break;
		}
		case Expose:
		{	// Window exposure
			// engine->OnPaint();
			break;
		}
		case FocusIn:
			//engine->WindowGotFocus(this);
			break;
		case FocusOut:
			//if (!HasFocus())	// For an unknown reason, FocusOut is called when clicking on title bar of window
			//  engine->WindowLostFocus(this);
			break;
		case PropertyNotify:
		{	// Iconify, Maximized, ...
			/*
			Atom _NET_WM_STATE = atoms["_NET_WM_STATE"];
			Atom WM_STATE = atoms["WM_STATE"]; // legacy.

			if (_NET_WM_STATE != None && event.xproperty.atom == _NET_WM_STATE && event.xproperty.state == PropertyNewValue)
			{
				if (is_minimized())
				{
					if (!minimized && site != nullptr)
						site->sig_window_minimized()();
					minimized = true;
					maximized = false;
				}
				else if (is_maximized())
				{
					if (!maximized && site != nullptr)
						site->sig_window_maximized()();
					if (minimized && site != nullptr)
					{
						// generate resize events for minimized -> maximized transition
						Rectf rectf = get_geometry();
						rectf.left   /= pixel_ratio;
						rectf.top    /= pixel_ratio;
						rectf.right  /= pixel_ratio;
						rectf.bottom /= pixel_ratio;

						site->sig_window_moved()();
						if (site->func_window_resize())
							site->func_window_resize()(rectf);

						if (callback_on_resized)
							callback_on_resized();

						site->sig_resize()(rectf.width(), rectf.height());
					}
					minimized = false;
					maximized = true;
				}
				else
				{
					if ((minimized || maximized) && site != nullptr)
						site->sig_window_restored()();
					minimized = false;
					maximized = false;
				}
			}
			else if (WM_STATE != None && event.xproperty.atom == WM_STATE && event.xproperty.state == PropertyNewValue)
			{
				if (is_minimized())
				{
					if (!minimized && site != nullptr)
						site->sig_window_minimized()();
					minimized = true;
				}
				else
				{
					if (minimized && site != nullptr)
						site->sig_window_restored()();
					minimized = false;
				}
			}
			*/
			break;
		}
		case KeyRelease:
		case KeyPress:
			OnKeyboardInput(event.xkey);
			break;
		case ButtonPress:
		case ButtonRelease:
			if (event.xany.send_event == 0)
			{
				OnMouseInput(event.xbutton);
			}
			break;
		case MotionNotify:
			if (event.xany.send_event == 0)
			{
				OnMouseMove(event.xmotion);
			}
			break;
		case SelectionClear: // New clipboard selection owner
			// clipboard.event_selection_clear(event.xselectionclear);
			break;
		case SelectionNotify:
			// clipboard.event_selection_notify();
			break;
		case SelectionRequest:	// Clipboard requests
			// clipboard.event_selection_request(event.xselectionrequest);
			break;
		default:
			break;
		}
	}

	// To do: use XGrabPointer to restrict the pointer to be within the window - or maybe there's a better way to track the mouse than this?
	XWarpPointer(display, window, window, 0, 0, SizeX, SizeY, SizeX / 2, SizeY / 2);

	bool Paused = false;
	if (Paused)
	{
		MouseMoveX = 0;
		MouseMoveY = 0;
	}
	else if (MouseMoveX != 0 || MouseMoveY != 0)
	{
		int DX = MouseMoveX;
		int DY = MouseMoveY;
		MouseMoveX = 0;
		MouseMoveY = 0;

		if (DX)
			engine->InputEvent(this, IK_MouseX, IST_Axis, +DX);
		if (DY)
			engine->InputEvent(this, IK_MouseY, IST_Axis, -DY);
	}
}

void X11Window::OnMouseInput(XButtonEvent &event)
{
	EInputKey id = IK_None;
	switch (event.button)
	{
	case 1: id = IK_LeftMouse; break;
	case 3: id = IK_RightMouse; break;
	case 2: id = IK_MiddleMouse; break;
	case 4: id = IK_MouseWheelUp; break;
	case 5: id = IK_MouseWheelDown; break;
	//case 6: id = IK_XButton1 break;
	//case 7: id = IK_XButton2; break;
	default: return;
	}

	MouseX = event.x;
	MouseY = event.y;

	if (event.type == ButtonPress)
	{
		engine->InputEvent(this, id, IST_Press);
	}
	else if (event.type == ButtonRelease)
	{
		engine->InputEvent(this, id, IST_Release);

		/*
		Time time_change = event.time - time_at_last_press;
		time_at_last_press = event.time;

		bool is_a_double_click_event = false;
		if (last_press_id == id)	// Same key pressed
		{
			if (time_change < 500)	// 500 ms is the default in Windows
			{
				is_a_double_click_event = true;
				last_press_id = IK_None;	// Reset to avoid "tripple clicks"
			}
		}
		else
		{
			last_press_id = id;
		}

		engine->InputEvent(this, id, IST_Click);
		if (is_a_double_click_event)
			engine->InputEvent(this, id, IST_DoubleClick);
		*/
	}
}

void X11Window::OnMouseMove(XMotionEvent &event)
{
	if (event.x == SizeX / 2 && event.y == SizeY / 2) // XWarpPointer moved the cursor to the center of the window
	{
		MouseX = SizeX / 2;
		MouseY = SizeY / 2;
	}

	if (MouseX != -1 && MouseY != -1)
	{
		MouseMoveX += event.x - MouseX;
		MouseMoveY += event.y - MouseY;
	}

	MouseX = event.x;
	MouseY = event.y;

	//engine->OnMouseMove(MouseX, MouseY);
}

void X11Window::OnKeyboardInput(XKeyEvent &event)
{
	bool keydown = (event.type == KeyPress);
	KeySym key_symbol = XkbKeycodeToKeysym(display, event.keycode, 0, 0);

	EInputType keytype = keydown ? IST_Press : IST_Release;

	if (keydown)
	{
		repeat_count[key_symbol]++;
	}
	else
	{
		repeat_count[key_symbol] = 0;
	}

	bool key_repeat = repeat_count[key_symbol] > 1;

	switch (key_symbol)
	{
	case XK_Control_L:
	case XK_Control_R:
		ctrl_down = keydown;
		break;
	case XK_Shift_L:
	case XK_Shift_R:
		shift_down = keydown;
		break;
	case XK_Alt_L:
	case XK_Alt_R:
		alt_down = keydown;
		break;
	}

	const int buff_size = 16;
	char buff[buff_size];
	int result = XLookupString(&event, buff, buff_size - 1, nullptr, nullptr);
	if (result < 0) result = 0;
	if (result > (buff_size - 1)) result = buff_size - 1;
	buff[result] = 0;
	std::string keystr(buff, result);
	if (!keystr.empty())
		engine->Key(this, keystr);

	engine->InputEvent(this, KeySymToInputKey(key_symbol), keytype);
}

EInputKey X11Window::KeySymToInputKey(KeySym keysym)
{
	switch (keysym)
	{
	case XK_BackSpace: return IK_Backspace;
	case XK_Tab: return IK_Tab;
	case XK_Clear: return IK_OEMClear;
	case XK_Return: return IK_Enter;
	case XK_Menu: return IK_Alt;
	case XK_Pause: return IK_Pause;
	// case XK_Kanji: return IK_Kanji;
	case XK_Escape: return IK_Escape;
	//case XK_Henkan_Mode: return IK_Convert;
	// case XK_Muhenkan: return IK_Muhenkan;
	case XK_space: return IK_Space;
	// case XK_Prior: return IK_Prior;
	// case XK_Next: return IK_Next;
	case XK_End: return IK_End;
	case XK_Home: return IK_Home;
	case XK_Left: return IK_Left;
	case XK_Up: return IK_Up;
	case XK_Right: return IK_Right;
	case XK_Down: return IK_Down;
	case XK_Select: return IK_Select;
	case XK_Print: return IK_Print;
	case XK_Execute: return IK_Execute;
	case XK_Insert: return IK_Insert;
	case XK_Delete: return IK_Delete;
	case XK_Help: return IK_Help;
	case XK_0: return IK_0;
	case XK_1: return IK_1;
	case XK_2: return IK_2;
	case XK_3: return IK_3;
	case XK_4: return IK_4;
	case XK_5: return IK_5;
	case XK_6: return IK_6;
	case XK_7: return IK_7;
	case XK_8: return IK_8;
	case XK_9: return IK_9;
	case XK_a: return IK_A;
	case XK_b: return IK_B;
	case XK_c: return IK_C;
	case XK_d: return IK_D;
	case XK_e: return IK_E;
	case XK_f: return IK_F;
	case XK_g: return IK_G;
	case XK_h: return IK_H;
	case XK_i: return IK_I;
	case XK_j: return IK_J;
	case XK_k: return IK_K;
	case XK_l: return IK_L;
	case XK_m: return IK_M;
	case XK_n: return IK_N;
	case XK_o: return IK_O;
	case XK_p: return IK_P;
	case XK_q: return IK_Q;
	case XK_r: return IK_R;
	case XK_s: return IK_S;
	case XK_t: return IK_T;
	case XK_u: return IK_U;
	case XK_v: return IK_V;
	case XK_w: return IK_W;
	case XK_x: return IK_X;
	case XK_y: return IK_Y;
	case XK_z: return IK_Z;
	//case XK_Super_L: return IK_LWin;
	//case XK_Multi_key: return IK_RWin;
	//case XK_Menu: return IK_Apps;
	case XK_KP_0: return IK_NumPad0;
	case XK_KP_1: return IK_NumPad1;
	case XK_KP_2: return IK_NumPad2;
	case XK_KP_3: return IK_NumPad3;
	case XK_KP_4: return IK_NumPad4;
	case XK_KP_5: return IK_NumPad5;
	case XK_KP_6: return IK_NumPad6;
	case XK_KP_7: return IK_NumPad7;
	case XK_KP_8: return IK_NumPad8;
	case XK_KP_9: return IK_NumPad9;
	//case XK_KP_Enter: return IK_NumPadEnter;
	//case XK_KP_Multiply: return IK_Multiply;
	//case XK_KP_Add: return IK_Add;
	case XK_KP_Separator: return IK_Separator;
	//case XK_KP_Subtract: return IK_Subtract;
	case XK_KP_Decimal: return IK_NumPadPeriod;
	//case XK_KP_Divide: return IK_Divide;
	case XK_F1: return IK_F1;
	case XK_F2: return IK_F2;
	case XK_F3: return IK_F3;
	case XK_F4: return IK_F4;
	case XK_F5: return IK_F5;
	case XK_F6: return IK_F6;
	case XK_F7: return IK_F7;
	case XK_F8: return IK_F8;
	case XK_F9: return IK_F9;
	case XK_F10: return IK_F10;
	case XK_F11: return IK_F11;
	case XK_F12: return IK_F12;
	case XK_F13: return IK_F13;
	case XK_F14: return IK_F14;
	case XK_F15: return IK_F15;
	case XK_F16: return IK_F16;
	case XK_F17: return IK_F17;
	case XK_F18: return IK_F18;
	case XK_F19: return IK_F19;
	case XK_F20: return IK_F20;
	case XK_F21: return IK_F21;
	case XK_F22: return IK_F22;
	case XK_F23: return IK_F23;
	case XK_F24: return IK_F24;
	case XK_Num_Lock: return IK_NumLock;
	case XK_Scroll_Lock: return IK_ScrollLock;
	case XK_Shift_L: return IK_LShift;
	case XK_Shift_R: return IK_RShift;
	case XK_Control_L: return IK_LControl;
	case XK_Control_R: return IK_RControl;
	//case XK_Meta_L: return IK_LMenu;
	//case XK_Meta_R: return IK_RMenu;
	default: return (EInputKey)(0x10000ul + keysym);
	}
}

void X11Window::MapWindow()
{
	// Clear exisitng StructureNofify events.
	XEvent event;
	while(XCheckMaskEvent(display, StructureNotifyMask, &event));

	if (is_window_mapped)
		return;

	int result = XMapWindow(display, window);
	if ((result == BadValue) || (result == BadWindow))
		throw std::runtime_error("Failed to map window");

	XFlush(display);

	do
	{
		XWindowEvent(display, window, StructureNotifyMask, &event);
	} while (event.type != MapNotify);

	XFlush(display);

	is_window_mapped = true;

	if (is_fullscreen)
	{
		XSetInputFocus(display, window, RevertToParent, CurrentTime);
		XFlush(display);
	}
}

void X11Window::UnmapWindow()
{
	// Clear exisitng StructureNofify events.
	XEvent event;
	while(XCheckMaskEvent(display, StructureNotifyMask, &event));

	if (!is_window_mapped)
		return;

	int result = XUnmapWindow(display, window);
	if (result == BadValue || result == BadWindow)
		throw std::runtime_error("Failed to unmap window.");

	XFlush(display);

	do
	{
		XWindowEvent(display, window, StructureNotifyMask, &event);
	} while (event.type != UnmapNotify);

	XFlush(display);

	is_window_mapped = false;
}

bool X11Window::HasFocus() const
{
	Window focus_window;
	int    focus_state;
	XGetInputFocus(display, &focus_window, &focus_state);
	return (focus_window == window);
}

bool X11Window::IsMinimized() const
{
	// Check FreeDeskop specified _NET_WM_STATE first.
	if (atoms["_NET_WM_STATE"] != None)
	{
		if (atoms["_NET_WM_STATE_HIDDEN"] != None)
		{
			auto ret = atoms.check_net_wm_state(window, { "_NET_WM_STATE_HIDDEN" } );
			return ret.front();
		}
	}

	// If not available, check legacy WM_STATE property
	if (atoms["WM_STATE"] != None)
	{
		unsigned long  item_count;
		unsigned char *data = atoms.get_property(window, "WM_STATE", item_count);
		if (data != NULL)
		{
			long state = *(long *)data;
			XFree(data);
			return state == IconicState;
		}
	}

	return false;
}

bool X11Window::IsMaximized() const
{
	auto ret = atoms.check_net_wm_state(window, { "_NET_WM_STATE_MAXIMIZED_HORZ", "_NET_WM_STATE_MAXIMIZED_VERT" });
	return ret[0] && ret[1];
}

bool X11Window::IsVisible() const
{
	XWindowAttributes attr;
	XGetWindowAttributes(display, window, &attr);
	if (attr.map_state == IsViewable) return false;
	return true;
}

void X11Window::ShowSystemCursor()
{
	XDefineCursor(display, window, system_cursor);
}

void X11Window::HideSystemCursor()
{
	XDefineCursor(display, window, hidden_cursor);
}

void X11Window::SetCursor(StandardCursor type)
{
	if (system_cursor)
	{
		XFreeCursor(display, system_cursor);
		system_cursor = 0;
	}
	unsigned int index = XC_left_ptr;
	switch (type)
	{
		case StandardCursor::arrow:
			index = XC_left_ptr;
			break;
		case StandardCursor::appstarting:
			index = XC_watch;
			break;
		case StandardCursor::cross:
			index = XC_cross;
			break;
		case StandardCursor::hand:
			index = XC_hand2;
			break;
		case StandardCursor::ibeam:
			index = XC_xterm;
			break;
		case StandardCursor::size_all:
			index = XC_fleur;
			break;
		case StandardCursor::size_ns:
			index = XC_double_arrow;
			break;
		case StandardCursor::size_we:
			index = XC_sb_h_double_arrow;
			break;
		case StandardCursor::uparrow:
			index = XC_sb_up_arrow;
			break;
		case StandardCursor::wait:
			index = XC_watch;
			break;
		case StandardCursor::no:
			index = XC_X_cursor;
			break;
		case StandardCursor::size_nesw:
		case StandardCursor::size_nwse:
		default:
			break;
	}
	system_cursor = XCreateFontCursor(display, index);
	XDefineCursor(display, window, system_cursor);
}
