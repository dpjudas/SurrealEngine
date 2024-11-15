#include "wayland_display_window.h"

#include <cstring>


WaylandDisplayWindow::WaylandDisplayWindow(WaylandDisplayBackend* backend, DisplayWindowHost* windowHost, bool popupWindow, WaylandDisplayWindow* owner, RenderAPI renderAPI)
    : backend(backend), m_owner(owner), windowHost(windowHost), m_PopupWindow(popupWindow), m_renderAPI(renderAPI)
{
    m_AppSurface = backend->m_waylandCompositor.create_surface();

    m_NativeHandle.display = backend->s_waylandDisplay;
    m_NativeHandle.surface = m_AppSurface;

    if (backend->m_FractionalScaleManager)
    {
        m_FractionalScale = backend->m_FractionalScaleManager.get_fractional_scale(m_AppSurface);

        m_FractionalScale.on_preferred_scale() = [&](uint32_t scale_numerator) {
            // parameter is the numerator of a fraction with the denominator of 120
            m_ScaleFactor = scale_numerator / 120.0;

            m_NeedsUpdate = true;
            windowHost->OnWindowDpiScaleChanged();
        };
    }

    m_XDGSurface = backend->m_XDGWMBase.get_xdg_surface(m_AppSurface);
    m_XDGSurface.on_configure() = [&] (uint32_t serial) {
        m_XDGSurface.ack_configure(serial);
    };

    if (popupWindow)
    {
        InitializePopup();
    }
    else
    {
        InitializeToplevel();
    }

    backend->OnWindowCreated(this);

    this->DrawSurface();
}

WaylandDisplayWindow::~WaylandDisplayWindow()
{
    backend->OnWindowDestroyed(this);
}

void WaylandDisplayWindow::InitializeToplevel()
{
    m_XDGToplevel = m_XDGSurface.get_toplevel();
    m_XDGToplevel.set_title("ZWidget Window");

    if (m_owner)
        m_XDGToplevel.set_parent(m_owner->m_XDGToplevel);

    if (backend->m_XDGDecorationManager)
    {
        // Force server side decorations if possible
        m_XDGToplevelDecoration = backend->m_XDGDecorationManager.get_toplevel_decoration(m_XDGToplevel);
        m_XDGToplevelDecoration.set_mode(wayland::zxdg_toplevel_decoration_v1_mode::server_side);
    }

    m_AppSurface.commit();

    backend->s_waylandDisplay.roundtrip();

    // These have to be added after the roundtrip
    m_XDGToplevel.on_configure() = [&] (int32_t width, int32_t height, wayland::array_t states) {
        OnXDGToplevelConfigureEvent(width, height);
    };

    m_XDGToplevel.on_close() = [&] () {
        OnExitEvent();
    };

    m_XDGToplevel.on_configure_bounds() = [this] (int32_t width, int32_t height)
    {

    };

    m_XDGExported = backend->m_XDGExporter.export_toplevel(m_AppSurface);

    m_XDGExported.on_handle() = [&] (std::string handleStr) {
        OnExportHandleEvent(handleStr);
    };
}

void WaylandDisplayWindow::InitializePopup()
{
    if (!m_owner)
        throw std::runtime_error("Popup window must have an owner!");

    wayland::xdg_positioner_t popupPositioner = backend->m_XDGWMBase.create_positioner();

    popupPositioner.set_anchor(wayland::xdg_positioner_anchor::bottom);
    popupPositioner.set_anchor_rect(0, 0, 1, 30);
    popupPositioner.set_size(1, 1);

    m_XDGPopup = m_XDGSurface.get_popup(m_owner->m_XDGSurface, popupPositioner);

    m_XDGPopup.on_configure() = [&] (int32_t x, int32_t y, int32_t width, int32_t height) {
        SetClientFrame(Rect::xywh(x, y, width, height));
    };

    //m_XDGPopup.on_repositioned()

    m_XDGPopup.on_popup_done() = [&] () {
        OnExitEvent();
    };

    m_AppSurface.commit();

    backend->s_waylandDisplay.roundtrip();
}


void WaylandDisplayWindow::SetWindowTitle(const std::string& text)
{
    if (m_XDGToplevel)
        m_XDGToplevel.set_title(text);
}

void WaylandDisplayWindow::SetWindowFrame(const Rect& box)
{
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
        m_XDGToplevel.set_fullscreen(backend->m_waylandOutput);
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
    // To do: this needs to be in the backend instance if all windows share the activation token
    wayland::xdg_activation_token_v1_t xdgActivationToken = backend->m_XDGActivation.get_activation_token();

    std::string tokenString;

    xdgActivationToken.on_done() = [&tokenString] (std::string obtainedString) {
        tokenString = obtainedString;
    };

    xdgActivationToken.set_surface(m_AppSurface);
    xdgActivationToken.commit();  // This will set our token string

    backend->m_XDGActivation.activate(tokenString, m_AppSurface);
    backend->m_FocusWindow = this;
    backend->m_MouseFocusWindow = this;
    windowHost->OnWindowActivated();
}

void WaylandDisplayWindow::ShowCursor(bool enable)
{
    backend->ShowCursor(enable);
}

void WaylandDisplayWindow::LockCursor()
{
    m_LockedPointer = backend->m_PointerConstraints.lock_pointer(m_AppSurface, backend->m_waylandPointer, nullptr, wayland::zwp_pointer_constraints_v1_lifetime::persistent);
    backend->SetMouseLocked(true);
    ShowCursor(false);
}

void WaylandDisplayWindow::UnlockCursor()
{
    if (m_LockedPointer)
        m_LockedPointer.proxy_release();
    backend->SetMouseLocked(false);
    ShowCursor(true);
}

void WaylandDisplayWindow::CaptureMouse()
{
    m_ConfinedPointer = backend->m_PointerConstraints.confine_pointer(GetWindowSurface(), backend->m_waylandPointer, nullptr, wayland::zwp_pointer_constraints_v1_lifetime::persistent);
    ShowCursor(false);
}

void WaylandDisplayWindow::ReleaseMouseCapture()
{
    if (m_ConfinedPointer)
        m_ConfinedPointer.proxy_release();
    ShowCursor(true);
}

void WaylandDisplayWindow::Update()
{
    m_NeedsUpdate = true;
}

bool WaylandDisplayWindow::GetKeyState(InputKey key)
{
    return backend->GetKeyState(key);
}

void WaylandDisplayWindow::SetCursor(StandardCursor cursor)
{
    backend->SetCursor(cursor);
}

Rect WaylandDisplayWindow::GetWindowFrame() const
{
    return Rect(m_WindowGlobalPos.x, m_WindowGlobalPos.y, m_WindowSize.width, m_WindowSize.height);
}

Size WaylandDisplayWindow::GetClientSize() const
{
    return m_WindowSize / m_ScaleFactor;
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
    return m_ClipboardContents;
}

void WaylandDisplayWindow::SetClipboardText(const std::string& text)
{
    m_ClipboardContents = text;
}

Point WaylandDisplayWindow::MapFromGlobal(const Point& pos) const
{
    return (pos - m_WindowGlobalPos) / m_ScaleFactor;
}

Point WaylandDisplayWindow::MapToGlobal(const Point& pos) const
{
    return (m_WindowGlobalPos + pos) / m_ScaleFactor;
}

void WaylandDisplayWindow::OnXDGToplevelConfigureEvent(int32_t width, int32_t height)
{
    Rect rect = GetWindowFrame();
    rect.width = width / m_ScaleFactor;
    rect.height = height / m_ScaleFactor;
    SetWindowFrame(rect);
    windowHost->OnWindowGeometryChanged();
}

void WaylandDisplayWindow::OnExportHandleEvent(std::string exportedHandle)
{
    m_windowID = exportedHandle;
}

void WaylandDisplayWindow::OnExitEvent()
{
    windowHost->OnWindowClose();
}

void WaylandDisplayWindow::DrawSurface(uint32_t serial)
{
    m_AppSurface.attach(m_AppSurfaceBuffer, 0, 0);
    m_AppSurface.damage(0, 0, m_WindowSize.width, m_WindowSize.height);

    if (m_renderAPI == RenderAPI::Unspecified || m_renderAPI == RenderAPI::Bitmap)
    {
        m_FrameCallback = m_AppSurface.frame();

        m_FrameCallback.on_done() = bind_mem_fn(&WaylandDisplayWindow::DrawSurface, this);
    }
    m_AppSurface.commit();
}

void WaylandDisplayWindow::CreateBuffers(int32_t width, int32_t height)
{
    if (width == 0 || height == 0)
        return;

    if (shared_mem)
        shared_mem.reset();

    int scaled_width = width * m_ScaleFactor;
    int scaled_height  = height * m_ScaleFactor;

    shared_mem = std::make_shared<SharedMemHelper>(scaled_width * scaled_height * 4);
    auto pool = backend->m_waylandSHM.create_pool(shared_mem->get_fd(), scaled_width * scaled_height * 4);

    m_AppSurfaceBuffer = pool.create_buffer(0, scaled_width, scaled_height, scaled_width * 4, wayland::shm_format::xrgb8888);

    m_WindowSize = Size(scaled_width, scaled_height);
}

std::string WaylandDisplayWindow::GetWaylandWindowID()
{
    return m_windowID;
}
