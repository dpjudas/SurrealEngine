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
            m_waylandRegistry.bind(name, m_waylandSeat, version);
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
        m_windowGlobalX = x;
        m_windowGlobalY = y;
    };

    m_XDGOutput.on_logical_size() = [&] (int32_t width, int32_t height) {
        m_windowWidth = width;
        m_windowHeight = height;
        windowHost->OnWindowGeometryChanged();
    };

    m_waylandOutput.on_scale() = [&] (int32_t scale) {
        m_ScaleFactor = scale;
        windowHost->OnWindowDpiScaleChanged();
    };

    m_waylandSeat.on_capabilities() = [&] (uint32_t capabilities) {
        hasKeyboard = capabilities & wayland::seat_capability::keyboard;
        hasPointer = capabilities & wayland::seat_capability::pointer;
    };

    m_AppSurface.commit();

    m_waylandDisplay.roundtrip();

    m_XDGToplevel.on_configure() = [&] (int32_t width, int32_t height, wayland::array_t states) {
        Rect rect = GetWindowFrame();
        rect.width = width;
        rect.height = height;
        SetWindowFrame(rect);
    };

    m_XDGToplevel.on_close() = [&] () {
        windowHost->OnWindowClose();
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
        m_cursorSurface.attach(m_cursorBuffer, 0, 0);
        m_cursorSurface.damage(0, 0, m_cursorImage.width(), m_cursorImage.height());
        m_cursorSurface.commit();
        m_waylandPointer.set_cursor(serial, m_cursorSurface, 0, 0);
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
    m_AppSurface.commit();
}

void WaylandDisplayWindow::SetClientFrame(const Rect& box)
{
    SetWindowFrame(box);
}

void WaylandDisplayWindow::Show()
{
    m_AppSurface.attach(m_AppSurfaceBuffer, 0, 0);
    m_AppSurface.damage(0, 0, m_windowWidth, m_windowHeight);
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
    {
        m_XDGToplevel.set_maximized();
    }

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
    return Rect(m_windowGlobalX, m_windowGlobalY, m_windowWidth, m_windowHeight);
}

Size WaylandDisplayWindow::GetClientSize() const
{
    return Size(m_windowWidth, m_windowHeight);
}

int WaylandDisplayWindow::GetPixelWidth() const
{
    return m_windowWidth;
}

int WaylandDisplayWindow::GetPixelHeight() const
{
    return m_windowHeight;
}

double WaylandDisplayWindow::GetDpiScale() const
{
    return m_ScaleFactor;
}

void WaylandDisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels)
{
    // Make new buffers if the sizes don't match
    if (width != m_windowWidth || height != m_windowHeight)
    {
        CreateBuffers(width, height);
        m_windowWidth = width;
        m_windowHeight = height;
    }

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
    return Point(0.0, 0.0);
}

Point WaylandDisplayWindow::MapToGlobal(const Point& pos) const
{
    return Point(0.0, 0.0);
}

void WaylandDisplayWindow::ProcessEvents()
{
    //TODO: Implement (???)
    while (m_waylandDisplay.dispatch() > 0 )
    {
    }
}

void WaylandDisplayWindow::RunLoop()
{
    //TODO: Implement (???)
    while (!exitRunLoop)
    {
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

void WaylandDisplayWindow::DrawSurface(uint32_t serial)
{
    m_AppSurface.attach(m_AppSurfaceBuffer, 0, 0);
    m_AppSurface.damage(0, 0, m_windowWidth, m_windowHeight);

    m_FrameCallback = m_AppSurface.frame();

    m_FrameCallback.on_done() = bind_mem_fn(&WaylandDisplayWindow::DrawSurface, this);

    m_AppSurface.commit();
}

void WaylandDisplayWindow::CreateBuffers(int32_t width, int32_t height)
{
    if (shared_mem)
        shared_mem.reset();

    shared_mem = std::make_shared<SharedMemHelper>(2 * width * height * 4);
    auto pool = m_waylandSHM.create_pool(shared_mem->get_fd(), 2 * width * height * 4);

    m_AppSurfaceBuffer = pool.create_buffer(width * height * 4, width, height, width * 4, wayland::shm_format::xrgb8888);

    m_windowWidth = width;
    m_windowHeight = height;
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


