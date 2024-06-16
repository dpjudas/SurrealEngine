#include "wayland_display_window.h"

#include <cstring>

bool WaylandDisplayWindow::exitRunLoop = false;
Size WaylandDisplayWindow::m_ScreenSize = Size(0, 0);
wayland::display_t WaylandDisplayWindow::m_waylandDisplay = wayland::display_t();
std::list<WaylandDisplayWindow*> WaylandDisplayWindow::s_Windows;
std::list<WaylandDisplayWindow*>::iterator WaylandDisplayWindow::s_WindowsIterator;

WaylandDisplayWindow::WaylandDisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, WaylandDisplayWindow* owner)
    : windowHost(windowHost), m_PopupWindow(popupWindow)
{
    if (!m_waylandDisplay)
        throw std::runtime_error("Wayland Display initialization failed!");

    m_waylandRegistry = m_waylandDisplay.get_registry();

    m_waylandRegistry.on_global() = [&](uint32_t name, std::string interface, uint32_t version) {
        if (interface == wayland::compositor_t::interface_name)
            m_waylandRegistry.bind(name, m_waylandCompositor, 3);
        if (interface == wayland::shm_t::interface_name)
            m_waylandRegistry.bind(name, m_waylandSHM, version);
        if (interface == wayland::output_t::interface_name)
            m_waylandRegistry.bind(name, m_waylandOutput, version);
        if (interface == wayland::seat_t::interface_name)
            m_waylandRegistry.bind(name, m_waylandSeat, 8);
        if (interface == wayland::xdg_wm_base_t::interface_name)
            m_waylandRegistry.bind(name, m_XDGWMBase, 1);
        if (interface == wayland::zxdg_output_manager_v1_t::interface_name)
            m_waylandRegistry.bind(name, m_XDGOutputManager, version);
        if (interface == wayland::zxdg_exporter_v2_t::interface_name)
            m_waylandRegistry.bind(name, m_XDGExporter, 1);
        if (interface == wayland::zwp_pointer_constraints_v1_t::interface_name)
            m_waylandRegistry.bind(name, m_PointerConstraints, 1);
        if (interface == wayland::xdg_activation_v1_t::interface_name)
            m_waylandRegistry.bind(name, m_XDGActivation, 1);
        if (interface == wayland::zxdg_decoration_manager_v1_t::interface_name)
            m_waylandRegistry.bind(name, m_XDGDecorationManager, 1);
    };

    m_waylandDisplay.roundtrip();

    if (!m_XDGWMBase)
        throw std::runtime_error("WaylandDisplayWindow: XDG-Shell is required!");

    if (!m_XDGOutputManager)
        throw std::runtime_error("WaylandDisplayWindow: xdg-output-manager-v1 is required!");

    if (!m_XDGExporter)
        throw std::runtime_error("WaylandDisplayWindow: xdg-foreign-unstable-v2 is required!");

    if (!m_PointerConstraints)
        throw std::runtime_error("WaylandDisplayWindow: pointer-constrains-unstable-v1 is required!");

    m_AppSurface = m_waylandCompositor.create_surface();

    m_waylandOutput.on_mode() = [&] (wayland::output_mode flags, int32_t width, int32_t height, int32_t refresh) {
        m_ScreenSize = Size(width, height);
    };

    m_XDGWMBase.on_ping() = [&] (uint32_t serial) {
        m_XDGWMBase.pong(serial);
    };

    m_XDGSurface = m_XDGWMBase.get_xdg_surface(m_AppSurface);
    m_XDGSurface.on_configure() = [&] (uint32_t serial) {
        m_XDGSurface.ack_configure(serial);
    };

    if (popupWindow)
    {
        wayland::xdg_positioner_t popupPositioner;
    }

    m_XDGToplevel = m_XDGSurface.get_toplevel();
    m_XDGToplevel.set_title("ZWidget Window");

    if (owner)
        m_XDGToplevel.set_parent(owner->m_XDGToplevel);

    if (m_XDGToplevel && m_XDGDecorationManager)
    {
        // Force server side decorations if possible
        m_XDGToplevelDecoration = m_XDGDecorationManager.get_toplevel_decoration(m_XDGToplevel);
        m_XDGToplevelDecoration.set_mode(wayland::zxdg_toplevel_decoration_v1_mode::server_side);
    }

    m_XDGOutput = m_XDGOutputManager.get_xdg_output(m_waylandOutput);

    m_XDGOutput.on_logical_position() = [&] (int32_t x, int32_t y) {
        m_WindowGlobalPos = Point(x, y);
    };

    m_XDGOutput.on_logical_size() = [&] (int32_t width, int32_t height) {
        m_WindowSize = Size(width, height);
        m_NeedsUpdate = true;
        windowHost->OnWindowGeometryChanged();
    };

    m_waylandOutput.on_scale() = [&] (int32_t scale) {
        m_ScaleFactor = scale;
        m_NeedsUpdate = true;
        windowHost->OnWindowDpiScaleChanged();
    };

    m_waylandSeat.on_capabilities() = [&] (uint32_t capabilities) {
        hasKeyboard = capabilities & wayland::seat_capability::keyboard;
        hasPointer = capabilities & wayland::seat_capability::pointer;
    };

    m_AppSurface.commit();

    m_waylandDisplay.roundtrip();

    m_XDGToplevel.on_configure() = [&] (int32_t width, int32_t height, wayland::array_t states) {
        OnXDGToplevelConfigureEvent(width, height);
    };

    m_XDGToplevel.on_close() = [&] () {
        OnExitEvent();
    };

    if (!hasKeyboard)
        throw std::runtime_error("No keyboard detected!");
    if (!hasPointer)
        throw std::runtime_error("No pointer device detected!");

    m_waylandKeyboard = m_waylandSeat.get_keyboard();
    m_waylandPointer = m_waylandSeat.get_pointer();

    m_cursorSurface = m_waylandCompositor.create_surface();
    SetCursor(StandardCursor::arrow);

    m_waylandPointer.on_enter() = [&](uint32_t serial, wayland::surface_t surfaceEntered, double surfaceX, double surfaceY) {
        OnMouseEnterEvent(serial);
    };

    m_waylandPointer.on_leave() = [&](uint32_t serial, wayland::surface_t surfaceLeft) {
        OnMouseLeaveEvent();
    };

    m_waylandPointer.on_motion() = [&] (uint32_t serial, double surfaceX, double surfaceY) {
        OnMouseMoveEvent(Point(surfaceX, surfaceY));
    };

    m_waylandPointer.on_button() = [&] (uint32_t serial, uint32_t time, uint32_t button, wayland::pointer_button_state state) {
        if (state == wayland::pointer_button_state::pressed)
            OnMousePressEvent(LinuxInputEventCodeToInputKey(button));
        if (state == wayland::pointer_button_state::released)
            OnMouseReleaseEvent(LinuxInputEventCodeToInputKey(button));
    };

    m_waylandPointer.on_axis() = [&] (uint32_t serial, wayland::pointer_axis axis, double value) {
        if (axis == wayland::pointer_axis::vertical_scroll && value > 0)
            OnMouseWheelEvent(InputKey::MouseWheelDown);
        if (axis == wayland::pointer_axis::vertical_scroll && value < 0)
            OnMouseWheelEvent(InputKey::MouseWheelUp);
    };

    // High resolution scroll event
    m_waylandPointer.on_axis_value120() = [&] (wayland::pointer_axis axis, int32_t value) {
        if (axis == wayland::pointer_axis::vertical_scroll && value > 0)
            OnMouseWheelEvent(InputKey::MouseWheelDown);
        if (axis == wayland::pointer_axis::vertical_scroll && value < 0)
            OnMouseWheelEvent(InputKey::MouseWheelUp);
    };

    s_Windows.push_front(this);
    s_WindowsIterator = s_Windows.begin();

    this->DrawSurface();
}

WaylandDisplayWindow::~WaylandDisplayWindow()
{
    s_Windows.erase(s_WindowsIterator);
}

void WaylandDisplayWindow::SetWindowTitle(const std::string& text)
{
    if (m_XDGToplevel)
        m_XDGToplevel.set_title(text);
}

void WaylandDisplayWindow::SetWindowFrame(const Rect& box)
{
    m_XDGSurface.set_window_geometry((int32_t)box.left(), (int32_t)box.top(),
                                     (int32_t)box.width, (int32_t)box.height);
    // Resizing will be shown on the next commit
    CreateBuffers(box.width, box.height);
    windowHost->OnWindowGeometryChanged();
    m_NeedsUpdate = true;
    m_AppSurface.commit();
}

void WaylandDisplayWindow::SetClientFrame(const Rect& box)
{
    SetWindowFrame(box);
}

void WaylandDisplayWindow::Show()
{
    m_AppSurface.attach(m_AppSurfaceBuffer, 0, 0);
    m_AppSurface.damage(0, 0, m_WindowSize.width, m_WindowSize.height);
    m_AppSurface.commit();
}

void WaylandDisplayWindow::ShowFullscreen()
{
    if (m_XDGToplevel)
        m_XDGToplevel.set_fullscreen(m_waylandOutput);
}

void WaylandDisplayWindow::ShowMaximized()
{
    if (m_XDGToplevel)
        m_XDGToplevel.set_maximized();
}

void WaylandDisplayWindow::ShowMinimized()
{
    if (m_XDGToplevel)
        m_XDGToplevel.set_minimized();
}

void WaylandDisplayWindow::ShowNormal()
{
    if (m_XDGToplevel)
        m_XDGToplevel.unset_fullscreen();
}

void WaylandDisplayWindow::Hide()
{
    // Apparently this is how hiding a window works
    // By attaching a null buffer to the surface
    // See: https://lists.freedesktop.org/archives/wayland-devel/2017-November/035963.html
    m_AppSurface.attach(nullptr, 0, 0);
    m_AppSurface.commit();
}

void WaylandDisplayWindow::Activate()
{
    wayland::xdg_activation_token_v1_t xdgActivationToken = m_XDGActivation.get_activation_token();

    std::string tokenString;

    xdgActivationToken.on_done() = [&tokenString] (std::string obtainedString) {
        tokenString = obtainedString;
    };

    xdgActivationToken.set_surface(m_AppSurface);
    xdgActivationToken.commit();  // This will set our token string

    m_XDGActivation.activate(tokenString, m_AppSurface);
}

void WaylandDisplayWindow::ShowCursor(bool enable)
{
    m_cursorSurface.attach(enable ? m_cursorBuffer : nullptr, 0, 0);
    m_cursorSurface.commit();
}

void WaylandDisplayWindow::LockCursor()
{
    m_LockedPointer = m_PointerConstraints.lock_pointer(m_AppSurface, m_waylandPointer, nullptr,
                                                        wayland::zwp_pointer_constraints_v1_lifetime::persistent);
}

void WaylandDisplayWindow::UnlockCursor()
{
    if (m_LockedPointer)
        m_LockedPointer.proxy_release();
}

void WaylandDisplayWindow::CaptureMouse()
{

}

void WaylandDisplayWindow::ReleaseMouseCapture()
{

}

void WaylandDisplayWindow::Update()
{
    m_NeedsUpdate = true;
}

bool WaylandDisplayWindow::GetKeyState(InputKey key)
{
    return false;
}

void WaylandDisplayWindow::SetCursor(StandardCursor cursor)
{
    std::string cursorName = GetWaylandCursorName(cursor);

    // Perhaps the cursor size can be inferred from the user prefs as well?
    wayland::cursor_theme_t cursorTheme = wayland::cursor_theme_t("default", 16, m_waylandSHM);
    wayland::cursor_t obtainedCursor = cursorTheme.get_cursor(cursorName);
    m_cursorImage = obtainedCursor.image(0);
    m_cursorBuffer = m_cursorImage.get_buffer();
}

Rect WaylandDisplayWindow::GetWindowFrame() const
{
    return Rect(m_WindowGlobalPos.x, m_WindowGlobalPos.y, m_WindowSize.width, m_WindowSize.height);
}

Size WaylandDisplayWindow::GetClientSize() const
{
    return m_WindowSize;
}

int WaylandDisplayWindow::GetPixelWidth() const
{
    return m_WindowSize.width;
}

int WaylandDisplayWindow::GetPixelHeight() const
{
    return m_WindowSize.height;
}

double WaylandDisplayWindow::GetDpiScale() const
{
    return m_ScaleFactor;
}

void WaylandDisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels)
{
    // Make new buffers if the sizes don't match
    if (width != m_WindowSize.width || height != m_WindowSize.height)
        CreateBuffers(width, height);

    std::memcpy(shared_mem->get_mem(), (void*)pixels, width * height * 4);
}

void WaylandDisplayWindow::SetBorderColor(uint32_t bgra8)
{

}

void WaylandDisplayWindow::SetCaptionColor(uint32_t bgra8)
{

}

void WaylandDisplayWindow::SetCaptionTextColor(uint32_t bgra8)
{

}

std::string WaylandDisplayWindow::GetClipboardText()
{
    return "";
}

void WaylandDisplayWindow::SetClipboardText(const std::string& text)
{

}

Point WaylandDisplayWindow::MapFromGlobal(const Point& pos) const
{
    return pos - m_WindowGlobalPos;
}

Point WaylandDisplayWindow::MapToGlobal(const Point& pos) const
{
    return m_WindowGlobalPos + pos;
}

void WaylandDisplayWindow::ProcessEvents()
{
    while (m_waylandDisplay.dispatch() > 0 )
    {
    }
}

void WaylandDisplayWindow::RunLoop()
{
    while (!exitRunLoop)
    {
        CheckNeedsUpdate();
        if (m_waylandDisplay.dispatch() == -1)
            break;
    }
}

void WaylandDisplayWindow::ExitLoop()
{
    exitRunLoop = true;
}

Size WaylandDisplayWindow::GetScreenSize()
{
    return m_ScreenSize;
}

void * WaylandDisplayWindow::StartTimer(int timeoutMilliseconds, std::function<void ()> onTimer)
{
    return nullptr;
}

void WaylandDisplayWindow::StopTimer(void* timerID)
{
}

void WaylandDisplayWindow::CheckNeedsUpdate()
{
    for (auto window: s_Windows)
    {
        if (window->m_NeedsUpdate)
        {
            window->m_NeedsUpdate = false;
            window->windowHost->OnWindowPaint();
        }
    }
}

void WaylandDisplayWindow::OnXDGToplevelConfigureEvent(int32_t width, int32_t height)
{
    Rect rect = GetWindowFrame();
    rect.width = width;
    rect.height = height;
    SetWindowFrame(rect);
    windowHost->OnWindowGeometryChanged();
}

void WaylandDisplayWindow::OnMouseEnterEvent(uint32_t serial)
{
    m_cursorSurface.attach(m_cursorBuffer, 0, 0);
    m_cursorSurface.damage(0, 0, m_cursorImage.width(), m_cursorImage.height());
    m_cursorSurface.commit();
    m_waylandPointer.set_cursor(serial, m_cursorSurface, 0, 0);
}

void WaylandDisplayWindow::OnMouseLeaveEvent()
{
    windowHost->OnWindowMouseLeave();
}

void WaylandDisplayWindow::OnMousePressEvent(InputKey button)
{
    windowHost->OnWindowMouseDown(MapToGlobal(m_SurfaceMousePos), button);
}

void WaylandDisplayWindow::OnMouseReleaseEvent(InputKey button)
{
    windowHost->OnWindowMouseUp(MapToGlobal(m_SurfaceMousePos), button);
}

void WaylandDisplayWindow::OnMouseMoveEvent(Point surfacePos)
{
    m_SurfaceMousePos = surfacePos;
    windowHost->OnWindowMouseMove(MapToGlobal(m_SurfaceMousePos));
}

void WaylandDisplayWindow::OnMouseWheelEvent(InputKey button)
{
    windowHost->OnWindowMouseWheel(MapToGlobal(m_SurfaceMousePos), button);
}

void WaylandDisplayWindow::OnExitEvent()
{
    windowHost->OnWindowClose();
}

void WaylandDisplayWindow::DrawSurface(uint32_t serial)
{
    m_AppSurface.attach(m_AppSurfaceBuffer, 0, 0);
    m_AppSurface.damage(0, 0, m_WindowSize.width, m_WindowSize.height);

    m_FrameCallback = m_AppSurface.frame();

    m_FrameCallback.on_done() = bind_mem_fn(&WaylandDisplayWindow::DrawSurface, this);

    m_AppSurface.commit();
}

void WaylandDisplayWindow::CreateBuffers(int32_t width, int32_t height)
{
    if (shared_mem)
        shared_mem.reset();

    shared_mem = std::make_shared<SharedMemHelper>(width * height * 4);
    auto pool = m_waylandSHM.create_pool(shared_mem->get_fd(), width * height * 4);

    m_AppSurfaceBuffer = pool.create_buffer(0, width, height, width * 4, wayland::shm_format::xrgb8888);

    m_WindowSize = Size(width, height);
}

std::string WaylandDisplayWindow::GetWaylandCursorName(StandardCursor cursor)
{
    // Checked out Adwaita and Breeze cursors for the names.
    // Other cursor themes should adhere to the names these two have.
    switch (cursor)
    {
        case StandardCursor::arrow:
            return "default";
        case StandardCursor::appstarting:
            return "progress";
        case StandardCursor::cross:
            return "crosshair";
        case StandardCursor::hand:
            return "pointer";
        case StandardCursor::ibeam:
            return "text";
        case StandardCursor::no:
            return "not-allowed";
        case StandardCursor::size_all:
            return "fleur";
        case StandardCursor::size_nesw:
            return "nesw-resize";
        case StandardCursor::size_ns:
            return "ns-resize";
        case StandardCursor::size_nwse:
            return "nwse-resize";
        case StandardCursor::size_we:
            return "ew-resize";
        case StandardCursor::uparrow:
            // Breeze actually has an up-arrow cursor, but Adwaita doesn't, so the default cursor it is
            return "default";
        case StandardCursor::wait:
            return "wait";
        default:
            return "default";
    }
}

std::string WaylandDisplayWindow::GetWaylandWindowID()
{
    m_XDGExported.on_handle() = [&] (std::string handleStr) {
        m_windowID = handleStr;
    };

    m_XDGExported = m_XDGExporter.export_toplevel(m_AppSurface);

    return m_windowID;
}

InputKey WaylandDisplayWindow::LinuxInputEventCodeToInputKey(uint32_t inputCode)
{
    switch (inputCode)
    {
        // Keyboard
        case KEY_ESC:
            return InputKey::Escape;
        case KEY_1:
            return InputKey::_1;
        case KEY_2:
            return InputKey::_2;
        case KEY_3:
            return InputKey::_3;
        case KEY_4:
            return InputKey::_4;
        case KEY_5:
            return InputKey::_5;
        case KEY_6:
            return InputKey::_6;
        case KEY_7:
            return InputKey::_7;
        case KEY_8:
            return InputKey::_8;
        case KEY_9:
            return InputKey::_9;
        case KEY_0:
            return InputKey::_0;
        case KEY_KP1:
            return InputKey::NumPad1;
        case KEY_KP2:
            return InputKey::NumPad2;
        case KEY_KP3:
            return InputKey::NumPad3;
        case KEY_KP4:
            return InputKey::NumPad4;
        case KEY_KP5:
            return InputKey::NumPad5;
        case KEY_KP6:
            return InputKey::NumPad6;
        case KEY_KP7:
            return InputKey::NumPad7;
        case KEY_KP8:
            return InputKey::NumPad8;
        case KEY_KP9:
            return InputKey::NumPad9;
        case KEY_KP0:
            return InputKey::NumPad0;
        case KEY_F1:
            return InputKey::F1;
        case KEY_F2:
            return InputKey::F2;
        case KEY_F3:
            return InputKey::F3;
        case KEY_F4:
            return InputKey::F4;
        case KEY_F5:
            return InputKey::F5;
        case KEY_F6:
            return InputKey::F6;
        case KEY_F7:
            return InputKey::F7;
        case KEY_F8:
            return InputKey::F8;
        case KEY_F9:
            return InputKey::F9;
        case KEY_F10:
            return InputKey::F10;
        case KEY_F11:
            return InputKey::F11;
        case KEY_F12:
            return InputKey::F12;
        case KEY_F13:
            return InputKey::F13;
        case KEY_F14:
            return InputKey::F14;
        case KEY_F15:
            return InputKey::F15;
        case KEY_F16:
            return InputKey::F16;
        case KEY_F17:
            return InputKey::F17;
        case KEY_F18:
            return InputKey::F18;
        case KEY_F19:
            return InputKey::F19;
        case KEY_F20:
            return InputKey::F20;
        case KEY_F21:
            return InputKey::F21;
        case KEY_F22:
            return InputKey::F22;
        case KEY_F23:
            return InputKey::F23;
        case KEY_F24:
            return InputKey::F24;
        case KEY_MINUS:
        case KEY_KPMINUS:
            return InputKey::Minus;
        case KEY_EQUAL:
            return InputKey::Equals;
        case KEY_BACKSPACE:
            return InputKey::Backspace;
        case KEY_BACKSLASH:
            return InputKey::Backslash;
        case KEY_TAB:
            return InputKey::Tab;
        case KEY_LEFTBRACE:
            return InputKey::LeftBracket;
        case KEY_RIGHTBRACE:
            return InputKey::RightBracket;
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL:
            return InputKey::Ctrl;
        case KEY_LEFTALT:
        case KEY_RIGHTALT:
            return InputKey::Alt;
        case KEY_DELETE:
            return InputKey::Delete;
        case KEY_SEMICOLON:
            return InputKey::Semicolon;
        case KEY_COMMA:
            return InputKey::Comma;
        case KEY_DOT:
            return InputKey::Period;
        case KEY_NUMLOCK:
            return InputKey::NumLock;
        case KEY_CAPSLOCK:
            return InputKey::CapsLock;
        case KEY_SCROLLDOWN:
            return InputKey::ScrollLock;
        case KEY_LEFTSHIFT:
            return InputKey::LShift;
        case KEY_RIGHTSHIFT:
            return InputKey::RShift;
        case KEY_GRAVE:
            return InputKey::Tilde;
        case KEY_APOSTROPHE:
            return InputKey::SingleQuote;
        case KEY_A:
            return InputKey::A;
        case KEY_B:
            return InputKey::B;
        case KEY_C:
            return InputKey::C;
        case KEY_D:
            return InputKey::D;
        case KEY_E:
            return InputKey::E;
        case KEY_F:
            return InputKey::F;
        case KEY_G:
            return InputKey::G;
        case KEY_H:
            return InputKey::H;
        case KEY_I:
            return InputKey::I;
        case KEY_J:
            return InputKey::J;
        case KEY_K:
            return InputKey::K;
        case KEY_L:
            return InputKey::L;
        case KEY_M:
            return InputKey::M;
        case KEY_N:
            return InputKey::N;
        case KEY_O:
            return InputKey::O;
        case KEY_P:
            return InputKey::P;
        case KEY_Q:
            return InputKey::Q;
        case KEY_R:
            return InputKey::R;
        case KEY_S:
            return InputKey::S;
        case KEY_T:
            return InputKey::T;
        case KEY_U:
            return InputKey::U;
        case KEY_V:
            return InputKey::V;
        case KEY_W:
            return InputKey::W;
        case KEY_X:
            return InputKey::X;
        case KEY_Y:
            return InputKey::Y;
        case KEY_Z:
            return InputKey::Z;

        // Mouse
        case BTN_LEFT:
            return InputKey::LeftMouse;
        case BTN_RIGHT:
            return InputKey::RightMouse;
        case BTN_MIDDLE:
            return InputKey::MiddleMouse;
        default:
            return InputKey::None;
    }
}
