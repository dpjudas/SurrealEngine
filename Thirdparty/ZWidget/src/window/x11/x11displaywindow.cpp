
#include "x11displaywindow.h"
#include <stdexcept>
#include <vector>
#include <cmath>
#include <map>
#include <string>
#include <cstring>
#include <dlfcn.h>
#include <unistd.h>
#include <iostream>

class X11Connection
{
public:
	X11Connection()
	{
		// If we ever want to support windows on multiple threads:
		// XInitThreads();

		display = XOpenDisplay(nullptr);
		if (!display)
			throw std::runtime_error("Could not open X11 display");

		// Make auto-repeat keys detectable
		Bool supports_detectable_autorepeat = {};
		XkbSetDetectableAutoRepeat(display, True, &supports_detectable_autorepeat);
	}

	~X11Connection()
	{
		XCloseDisplay(display);
	}

	Display* display = nullptr;
	std::map<std::string, Atom> atoms;
	std::map<Window, X11DisplayWindow*> windows;
	bool ExitRunLoop = false;
};

static X11Connection* GetX11Connection()
{
	static X11Connection connection;
	return &connection;
}

static Atom GetAtom(const std::string& name)
{
	auto connection = GetX11Connection();
	auto it = connection->atoms.find(name);
	if (it != connection->atoms.end())
		return it->second;
	
	Atom atom = XInternAtom(connection->display, name.c_str(), True);
	connection->atoms[name] = atom;
	return atom;
}

X11DisplayWindow::X11DisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, X11DisplayWindow* owner) : windowHost(windowHost), owner(owner)
{
	display = GetX11Connection()->display;

	screen = XDefaultScreen(display);
	depth = XDefaultDepth(display, screen);
	visual = XDefaultVisual(display, screen);
	colormap = XDefaultColormap(display, screen);

	int disp_width_px = XDisplayWidth(display, screen);
	int disp_height_px = XDisplayHeight(display, screen);
	int disp_width_mm = XDisplayWidthMM(display, screen);
	double ppi = (disp_width_mm < 24) ? 96.0 : (25.4 * static_cast<double>(disp_width_px) / static_cast<double>(disp_width_mm));
	dpiScale = ppi / 96.0;

	XSetWindowAttributes attributes = {};
	attributes.backing_store = Always;
	attributes.save_under = popupWindow ? True : False;
	attributes.colormap = colormap;
	attributes.event_mask =
		KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
		EnterWindowMask | LeaveWindowMask | PointerMotionMask | KeymapStateMask |
		ExposureMask | StructureNotifyMask | FocusChangeMask | PropertyChangeMask;
		
	unsigned long mask = CWBackingStore | CWSaveUnder | CWEventMask;

	window = XCreateWindow(display, XRootWindow(display, screen), 0, 0, 100, 100, 0, depth, InputOutput, visual, mask, &attributes);
	GetX11Connection()->windows[window] = this;

	if (owner)
	{
		XSetTransientForHint(display, window, owner->window);
	}

	// Tell window manager which process this window came from
	if (GetAtom("_NET_WM_PID") != None)
	{
		int32_t pid = getpid();
		if (pid != 0)
		{
			XChangeProperty(display, window, GetAtom("_NET_WM_PID"), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&pid, 1);
		}
	}

	// Tell window manager which machine this window came from
	if (GetAtom("WM_CLIENT_MACHINE") != None)
	{
		std::vector<char> hostname(256);
		if (gethostname(hostname.data(), hostname.size()) >= 0)
		{
			hostname.push_back(0);
			XChangeProperty(display, window, GetAtom("WM_CLIENT_MACHINE"), XA_STRING, 8, PropModeReplace, (unsigned char *)hostname.data(), strlen(hostname.data()));
		}
	}

	// Tell window manager we want to listen to close events
	if (GetAtom("WM_DELETE_WINDOW") != None)
	{
		Atom protocol = GetAtom("WM_DELETE_WINDOW");
		XSetWMProtocols(display, window, &protocol, 1);
	}

	// Tell window manager what type of window we are
	if (GetAtom("_NET_WM_WINDOW_TYPE") != None)
	{
		Atom type = None;
		if (popupWindow)
		{
			type = GetAtom("_NET_WM_WINDOW_TYPE_POPUP_MENU");
			if (type == None)
				type = GetAtom("_NET_WM_WINDOW_TYPE_COMBO");
			if (type == None)
				type =  GetAtom("_NET_WM_WINDOW_TYPE_DROPDOWN_MENU");
		}
		if (type == None)
			type = GetAtom("_NET_WM_WINDOW_TYPE_NORMAL");

		if (type != None)
		{
			XChangeProperty(display, window, GetAtom("_NET_WM_WINDOW_TYPE"), XA_ATOM, 32, PropModeReplace, (unsigned char *)&type, 1);
		}
	}
}

X11DisplayWindow::~X11DisplayWindow()
{
	DestroyBackbuffer();
	XDestroyWindow(display, window);
	GetX11Connection()->windows.erase(GetX11Connection()->windows.find(window));
}

void X11DisplayWindow::SetWindowTitle(const std::string& text)
{
	XSetStandardProperties(display, window, text.c_str(), text.c_str(), None, nullptr, 0, nullptr);
}

void X11DisplayWindow::SetWindowFrame(const Rect& box)
{
	// To do: this requires cooperation with the window manager

	SetClientFrame(box);
}	

void X11DisplayWindow::SetClientFrame(const Rect& box)
{
	double dpiscale = GetDpiScale();
	int x = (int)std::round(box.x * dpiscale);
	int y = (int)std::round(box.y * dpiscale);
	int width = (int)std::round(box.width * dpiscale);
	int height = (int)std::round(box.height * dpiscale);

	XWindowChanges changes = {};
	changes.x = x;
	changes.y = y;
	changes.width = width;
	changes.height = height;
	unsigned int mask = CWX | CWY | CWWidth | CWHeight;

	XConfigureWindow(display, window, mask, &changes);
}

void X11DisplayWindow::Show()
{
	if (!isMapped)
	{
		XMapRaised(display, window);
		isMapped = true;
	}
}

void X11DisplayWindow::ShowFullscreen()
{
	Show();

	if (GetAtom("_NET_WM_STATE") != None && GetAtom("_NET_WM_STATE_FULLSCREEN") != None)
	{
		Atom state = GetAtom("_NET_WM_STATE_FULLSCREEN");
		XChangeProperty(display, window, GetAtom("_NET_WM_STATE"), XA_ATOM, 32, PropModeReplace, (unsigned char *)&state, 1);
	}
}

void X11DisplayWindow::ShowMaximized()
{
	Show();
}

void X11DisplayWindow::ShowMinimized()
{
	if (!isMinimized)
	{
		Show(); // To do: can this be avoided? WMHints has an initial state that can make it show minimized
		XIconifyWindow(display, window, screen);
		isMinimized = true;
	}
}

void X11DisplayWindow::ShowNormal()
{
	Show();
}

void X11DisplayWindow::Hide()
{
	if (isMapped)
	{
		XUnmapWindow(display, window);
		isMapped = false;
	}
}

void X11DisplayWindow::Activate()
{
	XRaiseWindow(display, window);
}

void X11DisplayWindow::ShowCursor(bool enable)
{
	if (isCursorEnabled != enable)
	{
		isCursorEnabled = enable;
		UpdateCursor();
	}
}

void X11DisplayWindow::LockCursor()
{
}

void X11DisplayWindow::UnlockCursor()
{
}

void X11DisplayWindow::CaptureMouse()
{
}

void X11DisplayWindow::ReleaseMouseCapture()
{
}

void X11DisplayWindow::Update()
{
	needsUpdate = true;
}

bool X11DisplayWindow::GetKeyState(InputKey key)
{
	auto it = keyState.find(key);
	return it != keyState.end() ? it->second : false;
}

void X11DisplayWindow::SetCursor(StandardCursor newcursor)
{
	if (cursor != newcursor)
	{
		cursor = newcursor;
		UpdateCursor();
	}
}

void X11DisplayWindow::UpdateCursor()
{
	if (isCursorEnabled)
	{
		unsigned int index = XC_left_ptr;
		switch (cursor)
		{
		case StandardCursor::arrow: index = XC_left_ptr; break;
		case StandardCursor::appstarting: index = XC_watch; break;
		case StandardCursor::cross: index = XC_cross; break;
		case StandardCursor::hand: index = XC_hand2; break;
		case StandardCursor::ibeam: index = XC_xterm; break;
		case StandardCursor::size_all: index = XC_fleur; break;
		case StandardCursor::size_ns: index = XC_double_arrow; break;
		case StandardCursor::size_we: index = XC_sb_h_double_arrow; break;
		case StandardCursor::uparrow: index = XC_sb_up_arrow; break;
		case StandardCursor::wait: index = XC_watch; break;
		case StandardCursor::no: index = XC_X_cursor; break;
		case StandardCursor::size_nesw: break; // To do: need to map this
		case StandardCursor::size_nwse: break;
		default: break;
		}

		Cursor x11cursor =  XCreateFontCursor(display, index);
		XDefineCursor(display, window, x11cursor);
		XFreeCursor(display, x11cursor);
	}
	else
	{
		char data[64] = {};
		XColor black_color = {};
		Pixmap cursor_bitmap = XCreateBitmapFromData(display, window, data, 8, 8);
		Cursor hidden_cursor = XCreatePixmapCursor(display, cursor_bitmap, cursor_bitmap, &black_color, &black_color, 0, 0);
		XDefineCursor(display, window, hidden_cursor);
		XFreeCursor(display, hidden_cursor);
		XFreePixmap(display, cursor_bitmap);
	}
}

Rect X11DisplayWindow::GetWindowFrame() const
{
	// To do: this needs to include the window manager frame

	double dpiscale = GetDpiScale();

	Window root = {};
	int x = 0;
	int y = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int borderwidth = 0;
	unsigned int depth = 0;
	Status status = XGetGeometry(display, window, &root, &x, &y, &width, &height, &borderwidth, &depth);

	return Rect::xywh(x / dpiscale, y / dpiscale, width / dpiscale, height / dpiscale);
}

Size X11DisplayWindow::GetClientSize() const
{
	double dpiscale = GetDpiScale();

	Window root = {};
	int x = 0;
	int y = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int borderwidth = 0;
	unsigned int depth = 0;
	Status status = XGetGeometry(display, window, &root, &x, &y, &width, &height, &borderwidth, &depth);

	return Size(width / dpiscale, height / dpiscale);
}

int X11DisplayWindow::GetPixelWidth() const
{
	Window root = {};
	int x = 0;
	int y = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int borderwidth = 0;
	unsigned int depth = 0;
	Status status = XGetGeometry(display, window, &root, &x, &y, &width, &height, &borderwidth, &depth);
	return width;
}

int X11DisplayWindow::GetPixelHeight() const
{
	Window root = {};
	int x = 0;
	int y = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int borderwidth = 0;
	unsigned int depth = 0;
	Status status = XGetGeometry(display, window, &root, &x, &y, &width, &height, &borderwidth, &depth);
	return height;
}

double X11DisplayWindow::GetDpiScale() const
{
	return dpiScale;
}

void X11DisplayWindow::CreateBackbuffer(int width, int height)
{
	backbuffer.pixels = malloc(width * height * sizeof(uint32_t));
	backbuffer.image = XCreateImage(display, DefaultVisual(display, screen), depth, ZPixmap, 0, (char*)backbuffer.pixels, width, height, 32, 0);
	backbuffer.pixmap = XCreatePixmap(display, window, width, height, depth);
	backbuffer.width = width;
	backbuffer.height = height;
}

void X11DisplayWindow::DestroyBackbuffer()
{
	if (backbuffer.width > 0 && backbuffer.height > 0)
	{
		XDestroyImage(backbuffer.image);
		XFreePixmap(display, backbuffer.pixmap);
		backbuffer.width = 0;
		backbuffer.height = 0;
		backbuffer.pixmap = None;
		backbuffer.image = nullptr;
		backbuffer.pixels = nullptr;
	}
}

void X11DisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels)
{
	if (backbuffer.width != width || backbuffer.height != height)
	{
		DestroyBackbuffer();
		if (width > 0 && height > 0)
			CreateBackbuffer(width, height);
	}

	if (backbuffer.width == width && backbuffer.height == height)
	{
		memcpy(backbuffer.pixels, pixels, width * height * sizeof(uint32_t));
		GC gc = XDefaultGC(display, screen);
		XPutImage(display, backbuffer.pixmap, gc, backbuffer.image, 0, 0, 0, 0, width, height);
		XCopyArea(display, backbuffer.pixmap, window, gc, 0, 0, width, height, BlackPixel(display, screen), WhitePixel(display, screen));
	}
}

void X11DisplayWindow::SetBorderColor(uint32_t bgra8)
{
}

void X11DisplayWindow::SetCaptionColor(uint32_t bgra8)
{
}

void X11DisplayWindow::SetCaptionTextColor(uint32_t bgra8)
{
}

std::vector<uint8_t> X11DisplayWindow::GetWindowProperty(Atom property, Atom &actual_type, int &actual_format, unsigned long &item_count)
{
	long read_bytes = 0;
	Atom _actual_type = actual_type;
	int  _actual_format = actual_format;
	unsigned long _item_count = item_count;
	unsigned long bytes_remaining = 0;
	unsigned char *read_data = nullptr;
	do
	{
		int result = XGetWindowProperty(
			display, window, property, 0ul, read_bytes,
			False, AnyPropertyType, &actual_type, &actual_format,
			&_item_count, &bytes_remaining, &read_data);
		if (result != Success)
		{
			actual_type = None;
			actual_format = 0;
			item_count = 0;
			return {};
		}
	} while (bytes_remaining > 0);

	item_count = _item_count;
	if (!read_data)
		return {};
	std::vector<uint8_t> buffer(read_data, read_data + read_bytes);
	XFree(read_data);
	return buffer;
}

std::string X11DisplayWindow::GetClipboardText()
{
	Atom clipboard = GetAtom("CLIPBOARD");
	if (clipboard == None)
		return {};

	XConvertSelection(display, clipboard, XA_STRING, clipboard, window, CurrentTime);
	XFlush(display);

	// Wait 500 ms for a response
	XEvent event = {};
	while (true)
	{
		if (XCheckTypedWindowEvent(display, window, SelectionNotify, &event))
			break;
		if (!WaitForEvents(500))
			return {};
	}

	Atom type = None;
	int format = 0;
	unsigned long count = 0;
	std::vector<uint8_t> data = GetWindowProperty(clipboard, type, format, count);
	if (type != XA_STRING || format != 8 || count <= 0 || data.empty())
		return {};

	data.push_back(0);
	return (char*)data.data();
}

void X11DisplayWindow::SetClipboardText(const std::string& text)
{
	clipboardText = text;

	Atom clipboard = GetAtom("CLIPBOARD");
	if (clipboard == None)
		return;

	XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
	XSetSelectionOwner(display, clipboard, window, CurrentTime);
}

Point X11DisplayWindow::MapFromGlobal(const Point& pos) const
{
	double dpiscale = GetDpiScale();
	Window root = XRootWindow(display, screen);
	Window child = {};
	int srcx = (int)std::round(pos.x * dpiscale);
	int srcy = (int)std::round(pos.y * dpiscale);
	int destx = 0;
	int desty = 0;
	Bool result = XTranslateCoordinates(display, root, window, srcx, srcy, &destx, &desty, &child);
	return Point(destx / dpiscale, desty / dpiscale);
}

Point X11DisplayWindow::MapToGlobal(const Point& pos) const
{
	double dpiscale = GetDpiScale();
	Window root = XRootWindow(display, screen);
	Window child = {};
	int srcx = (int)std::round(pos.x * dpiscale);
	int srcy = (int)std::round(pos.y * dpiscale);
	int destx = 0;
	int desty = 0;
	Bool result = XTranslateCoordinates(display, window, root, srcx, srcy, &destx, &desty, &child);
	return Point(destx / dpiscale, desty / dpiscale);
}

void* X11DisplayWindow::GetNativeHandle()
{
	return reinterpret_cast<void*>(window);
}

bool X11DisplayWindow::WaitForEvents(int timeout)
{
	Display* display = GetX11Connection()->display;
	int fd = XConnectionNumber(display);

	struct timeval tv;
	if (timeout > 0)
	{
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) / 1000;
	}

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	int result = select(fd + 1, &rfds, nullptr, nullptr, timeout >= 0 ? &tv : nullptr);
	return result > 0 && FD_ISSET(fd, &rfds);
}

void X11DisplayWindow::CheckNeedsUpdate()
{
	for (auto& it : GetX11Connection()->windows)
	{
		if (it.second->needsUpdate)
		{
			it.second->needsUpdate = false;
			it.second->windowHost->OnWindowPaint();
		}
	}
}

void X11DisplayWindow::ProcessEvents()
{
	CheckNeedsUpdate();
	Display* display = GetX11Connection()->display;
	while (XPending(display) > 0)
	{
		XEvent event = {};
		XNextEvent(display, &event);
		DispatchEvent(&event);
	}
}

void X11DisplayWindow::RunLoop()
{
	X11Connection* connection = GetX11Connection();
	connection->ExitRunLoop = false;
	while (!connection->ExitRunLoop && !connection->windows.empty())
	{
		CheckNeedsUpdate();
		XEvent event = {};
		XNextEvent(connection->display, &event);
		DispatchEvent(&event);
	}
}

void X11DisplayWindow::ExitLoop()
{
	X11Connection* connection = GetX11Connection();
	connection->ExitRunLoop = true;
}

void X11DisplayWindow::DispatchEvent(XEvent* event)
{
	X11Connection* connection = GetX11Connection();
	auto it = connection->windows.find(event->xany.window);
	if (it != connection->windows.end())
	{
		X11DisplayWindow* window = it->second;
		window->OnEvent(event);
	}
}

void X11DisplayWindow::OnEvent(XEvent* event)
{
	if (event->type == ConfigureNotify)
		OnConfigureNotify(event);
	else if (event->type == ClientMessage)
		OnClientMessage(event);
	else if (event->type == Expose)
		OnExpose(event);
	else if (event->type == FocusIn)
		OnFocusIn(event);
	else if (event->type == FocusOut)
		OnFocusOut(event);
	else if (event->type == PropertyNotify)
		OnPropertyNotify(event);
	else if (event->type == KeyPress)
		OnKeyPress(event);
	else if (event->type == KeyRelease)
		OnKeyRelease(event);
	else if (event->type == ButtonPress)
		OnButtonPress(event);
	else if (event->type == ButtonRelease)
		OnButtonRelease(event);
	else if (event->type == MotionNotify)
		OnMotionNotify(event);
	else if (event->type == LeaveNotify)
		OnLeaveNotify(event);
	else if (event->type == SelectionClear)
		OnSelectionClear(event);
	else if (event->type == SelectionNotify)
		OnSelectionNotify(event);
	else if (event->type == SelectionRequest)
		OnSelectionRequest(event);
}

void X11DisplayWindow::OnConfigureNotify(XEvent* event)
{
	windowHost->OnWindowGeometryChanged();
}

void X11DisplayWindow::OnClientMessage(XEvent* event)
{
	Atom protocolsAtom = GetAtom("WM_PROTOCOLS");
	if (protocolsAtom != None && event->xclient.message_type == protocolsAtom)
	{
		Atom deleteAtom = GetAtom("WM_DELETE_WINDOW");
		Atom pingAtom = GetAtom("_NET_WM_PING");

		Atom protocol = event->xclient.data.l[0];
		if (deleteAtom != None && protocol == deleteAtom)
		{
			windowHost->OnWindowClose();
		}
		else if (pingAtom != None && protocol == pingAtom)
		{
			XSendEvent(display, RootWindow(display, screen), False, SubstructureNotifyMask | SubstructureRedirectMask, event);
		}
	}
}

void X11DisplayWindow::OnExpose(XEvent* event)
{
	windowHost->OnWindowPaint();
}

void X11DisplayWindow::OnFocusIn(XEvent* event)
{
	windowHost->OnWindowActivated();
}

void X11DisplayWindow::OnFocusOut(XEvent* event)
{
}

void X11DisplayWindow::OnPropertyNotify(XEvent* event)
{
	// Sent when window is minimized, maximized, etc.
}

InputKey X11DisplayWindow::GetInputKey(XEvent* event)
{
	if (event->type == KeyPress || event->type == KeyRelease)
	{

	}
	else if (event->type == ButtonPress || event->type == ButtonRelease)
	{
		switch (event->xbutton.button)
		{
		case 1: return InputKey::LeftMouse;
		case 2: return InputKey::MiddleMouse;
		case 3: return InputKey::RightMouse;
		case 4: return InputKey::MouseWheelUp;
		case 5: return InputKey::MouseWheelDown;
		// case 6: return InputKey::XButton1;
		// case 7: return InputKey::XButton2;
		default: break;
		}
	}
	return {};
}

Point X11DisplayWindow::GetMousePos(XEvent* event)
{
	double dpiScale = GetDpiScale();
	int x = event->xbutton.x;
	int y = event->xbutton.y;
	return Point(x / dpiScale, y / dpiScale);
}

void X11DisplayWindow::OnKeyPress(XEvent* event)
{
	InputKey key = GetInputKey(event);
	keyState[key] = true;
	windowHost->OnWindowKeyDown(key);
}

void X11DisplayWindow::OnKeyRelease(XEvent* event)
{
	InputKey key = GetInputKey(event);
	keyState[key] = false;
	windowHost->OnWindowKeyUp(key);
}

void X11DisplayWindow::OnButtonPress(XEvent* event)
{
	InputKey key = GetInputKey(event);
	keyState[key] = true;
	windowHost->OnWindowMouseDown(GetMousePos(event), key);
	// if (lastClickWithin400ms)
	//	windowHost->OnWindowMouseDoubleclick(GetMousePos(event), InputKey::LeftMouse);
}

void X11DisplayWindow::OnButtonRelease(XEvent* event)
{
	InputKey key = GetInputKey(event);
	keyState[key] = false;
	windowHost->OnWindowMouseUp(GetMousePos(event), key);
}

void X11DisplayWindow::OnMotionNotify(XEvent* event)
{
	double dpiScale = GetDpiScale();
	int x = event->xmotion.x;
	int y = event->xmotion.y;
	windowHost->OnWindowMouseMove(Point(x / dpiScale, y / dpiScale));
}

void X11DisplayWindow::OnLeaveNotify(XEvent* event)
{
	windowHost->OnWindowMouseLeave();
}

void X11DisplayWindow::OnSelectionClear(XEvent* event)
{
	clipboardText.clear();
}

void X11DisplayWindow::OnSelectionNotify(XEvent* event)
{
	// This is handled in GetClipboardText
}

void X11DisplayWindow::OnSelectionRequest(XEvent* event)
{
	Atom requestor = event->xselectionrequest.requestor;
	if (requestor == window)
		return;

	Atom targetsAtom = GetAtom("TARGETS");
	Atom multipleAtom = GetAtom("MULTIPLE");

	struct Request { Window target; Atom property; };
	std::vector<Request> requests;

	if (event->xselectionrequest.target == multipleAtom)
	{
		Atom actualType = None;
		int actualFormat = 0;
		unsigned long itemCount = 0;
		std::vector<uint8_t> data = GetWindowProperty(requestor, actualType, actualFormat, itemCount);
		if (data.size() < itemCount * sizeof(Atom))
			return;

		Atom* atoms = (Atom*)data.data();
		for (unsigned long i = 0; i + 1 < itemCount; i += 2)
		{
			requests.push_back({ atoms[i], atoms[i + 1]});
		}
	}
	else
	{
		requests.push_back({ event->xselectionrequest.target, event->xselectionrequest.property });
	}

	for (const Request& request : requests)
	{
		Window xtarget = request.target;
		Atom xproperty = request.property;

		XEvent response = {};
		response.xselection.type = SelectionNotify;
		response.xselection.display = event->xselectionrequest.display;
		response.xselection.requestor = event->xselectionrequest.requestor;
		response.xselection.selection = event->xselectionrequest.selection;
		response.xselection.target = event->xselectionrequest.target;
		response.xselection.property = xproperty;
		response.xselection.time = event->xselectionrequest.time;

		if (xtarget == targetsAtom)
		{
			Atom newTargets = XA_STRING;
			XChangeProperty(display, requestor, xproperty, targetsAtom, 32, PropModeReplace, (unsigned char *)&newTargets, 1);
		}
		else if (xtarget == XA_STRING)
		{
			XChangeProperty(display, requestor, xproperty, xtarget, 8, PropModeReplace, (const unsigned char*)clipboardText.c_str(), clipboardText.size());
		}
		else
		{
			response.xselection.property = None; // Is this correct?
		}

		XSendEvent(display, requestor, False, 0, &response);
	}
}

Size X11DisplayWindow::GetScreenSize()
{
	X11Connection* connection = GetX11Connection();
	Display* display = connection->display;
	int screen = XDefaultScreen(display);

	int disp_width_px = XDisplayWidth(display, screen);
	int disp_height_px = XDisplayHeight(display, screen);
	int disp_width_mm = XDisplayWidthMM(display, screen);
	double ppi = (disp_width_mm < 24) ? 96.0 : (25.4 * static_cast<double>(disp_width_px) / static_cast<double>(disp_width_mm));
	double dpiScale = ppi / 96.0;

	return Size(disp_width_px / dpiScale, disp_height_px / dpiScale);
}

void* X11DisplayWindow::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return nullptr;
}

void X11DisplayWindow::StopTimer(void* timerID)
{
}
