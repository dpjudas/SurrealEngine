#include "wayland_display_window.h"

#include <cstring>

bool WaylandDisplayWindow::exitRunLoop = false;
WaylandClientState WaylandDisplayWindow::clientState;

WaylandDisplayWindow::WaylandDisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, WaylandDisplayWindow* owner)
    : windowHost(windowHost), m_PopupWindow(popupWindow)
{
    clientState.waylandDisplay = wl_display_connect(nullptr);

    if (!clientState.waylandDisplay)
        throw std::runtime_error("WaylandDisplayWindow: Failed to connect display!");

    clientState.waylandRegistry = wl_display_get_registry(clientState.waylandDisplay);

    if (!clientState.waylandRegistry)
        throw std::runtime_error("WaylandDisplayWindow: Failed to obtain global registry!");

    wl_registry_listener registryListener = {
        .global = RegistryGlobalHandler
    };

    wl_registry_add_listener(clientState.waylandRegistry, &registryListener, &clientState);

    wl_display_roundtrip(clientState.waylandDisplay);

    wl_output_listener outputListener = {
            .geometry = OnOutputGeometryEvent,
            .mode = OnOutputModeEvent,
            .done = OnOutputDoneEvent,
            .scale = OnOutputScaleEvent,
            .name = OnOutputNameEvent,
            .description = OnOutputDescriptionEvent
    };

    wl_output_add_listener(clientState.waylandOutput, &outputListener, &clientState.outputState);

    if (!clientState.xdgWMBase)
        throw std::runtime_error("WaylandDisplayWindow: xdg_wm_base protocol is required!");

    if (!clientState.xdgOutputManager)
        throw std::runtime_error("WaylandDisplayWindow: zxdg_output_manager_v1 protocol is required!");

    wl_seat_listener seatListener = {
        .capabilities = RegisterSeatCapabilities
    };

    wl_seat_add_listener(clientState.waylandSeat, &seatListener, &clientState);

    m_AppSurface = wl_compositor_create_surface(clientState.waylandCompositor);

    xdg_wm_base_listener wmBaseListener = {
        .ping = OnXDGWMBasePingEvent
    };

    xdg_wm_base_add_listener(clientState.xdgWMBase, &wmBaseListener, nullptr);

    m_XDGSurface = xdg_wm_base_get_xdg_surface(clientState.xdgWMBase, m_AppSurface);

    xdg_surface_listener xdgSurfaceListener = {
        .configure = OnXDGSurfaceConfigureEvent
    };

    xdg_surface_add_listener(m_XDGSurface, &xdgSurfaceListener, nullptr);


    if (popupWindow)
    {
        // Create an xdg_popup instead of an xdg_toplevel
        // Also needs an xdg_positioner
        // m_XGDPopup = m_XDGSurface.get_popup(owner ? owner->GetNativeHandle() : nullptr);
    }
    else
    {
        m_XDGToplevel = xdg_surface_get_toplevel(m_XDGSurface);

        xdg_toplevel_set_title(m_XDGToplevel, "ZWidget Window");

        if (owner)
            xdg_toplevel_set_parent(m_XDGToplevel, (xdg_toplevel*)owner->GetNativeHandle());
    }

    m_XDGOutput = zxdg_output_manager_v1_get_xdg_output(clientState.xdgOutputManager, clientState.waylandOutput);

    zxdg_output_v1_listener xdgOutputListener = {
        .logical_position = OnXDGOutputLogicalPositionEvent,
        .logical_size = OnXDGOutputLogicalSizeEvent,
        .done = OnXDGOutputDoneEvent,
        .name = OnXDGOutputNameEvent,
        .description = OnXDGOutputDescriptionEvent
    };

    zxdg_output_v1_add_listener(m_XDGOutput, &xdgOutputListener, &m_GlobalPosInfo);

    wl_surface_commit(m_AppSurface);

    wl_display_roundtrip(clientState.waylandDisplay);

    if(!clientState.hasKeyboard)
      throw std::runtime_error("No keyboard found.");
    if(!clientState.hasPointer)
      throw std::runtime_error("No pointer found.");

    m_WaylandKeyboard = wl_seat_get_keyboard(clientState.waylandSeat);
    m_WaylandPointer = wl_seat_get_pointer(clientState.waylandSeat);

    CreateBuffers(640, 480);

    SetCursor(StandardCursor::arrow);

    // create cursor surface
    // P.S. KDevelop is stupid trying to convert the . to -> for no reason
    m_CursorInfo.cursorSurface = wl_compositor_create_surface(clientState.waylandCompositor);

    wl_pointer_listener pointerListener = {
        .enter = OnPointerEnterEvent
    };

    wl_pointer_add_listener(m_WaylandPointer, &pointerListener, &m_CursorInfo);
}

WaylandDisplayWindow::~WaylandDisplayWindow()
{
}

void WaylandDisplayWindow::SetWindowTitle(const std::string& text)
{
    if (m_XDGToplevel)
        xdg_toplevel_set_title(m_XDGToplevel, text.c_str());
}

void WaylandDisplayWindow::SetWindowFrame(const Rect& box)
{

}

void WaylandDisplayWindow::SetClientFrame(const Rect& box)
{

}

void WaylandDisplayWindow::Show()
{
    wl_surface_attach(m_AppSurface, m_AppSurfaceBuffer, 0, 0);
    wl_surface_commit(m_AppSurface);
}

void WaylandDisplayWindow::ShowFullscreen()
{
    if (m_XDGToplevel)
        xdg_toplevel_set_fullscreen(m_XDGToplevel, clientState.waylandOutput);
}

void WaylandDisplayWindow::ShowMaximized()
{
    if (m_XDGToplevel)
        xdg_toplevel_set_maximized(m_XDGToplevel);
}

void WaylandDisplayWindow::ShowMinimized()
{
    if (m_XDGToplevel)
        xdg_toplevel_set_minimized(m_XDGToplevel);
}

void WaylandDisplayWindow::ShowNormal()
{
    if (m_XDGToplevel)
        xdg_toplevel_unset_fullscreen(m_XDGToplevel);
}

void WaylandDisplayWindow::Hide()
{
    // Apparently this is how hiding a window works
    // By attaching a null buffer to the surface
    // See: https://lists.freedesktop.org/archives/wayland-devel/2017-November/035963.html
    wl_surface_attach(m_AppSurface, nullptr, 0, 0);
    wl_surface_commit(m_AppSurface);
}

void WaylandDisplayWindow::Activate()
{

}

void WaylandDisplayWindow::ShowCursor(bool enable)
{

}

void WaylandDisplayWindow::LockCursor()
{

}

void WaylandDisplayWindow::UnlockCursor()
{

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
    wl_cursor_theme* cursorTheme = wl_cursor_theme_load("default", 16, clientState.waylandSHM);
    wl_cursor* obtainedCursor = wl_cursor_theme_get_cursor(cursorTheme, cursorName.c_str());
    m_CursorInfo.cursorImage = obtainedCursor->images[0];
    m_CursorInfo.cursorBuffer = wl_cursor_image_get_buffer(m_CursorInfo.cursorImage);
}

Rect WaylandDisplayWindow::GetWindowFrame() const
{
    return Rect(m_GlobalPosInfo.xPos, m_GlobalPosInfo.yPos, m_windowWidth, m_windowHeight);
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
    return clientState.outputState.scaleFactor;
}

void WaylandDisplayWindow::PresentBitmap(int width, int height, const uint32_t* pixels)
{
    // Make new buffers if the sizes don't match
    if (width != m_windowWidth || height != m_windowHeight)
        CreateBuffers(width, height);

    std::memcpy(shared_mem->get_mem(), (void*)pixels, width * height * 4);

    wl_surface_attach(m_AppSurface, m_AppSurfaceBuffer, 0, 0);
    wl_surface_commit(m_AppSurface);
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
    //TODO: Implement
}

void WaylandDisplayWindow::RunLoop()
{
    //TODO: Implement
    while (!exitRunLoop)
    {
        while (wl_display_dispatch(clientState.waylandDisplay) != -1)
        {

        }
    }
}

void WaylandDisplayWindow::ExitLoop()
{
    exitRunLoop = true;
}

Size WaylandDisplayWindow::GetScreenSize()
{
    // TODO: Implement
    return Size(0, 0);
}

void WaylandDisplayWindow::RegistryGlobalHandler(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WaylandClientState* state = (WaylandClientState*) data;
    if (strcmp(interface, wl_compositor_interface.name) == 0)
        state->waylandCompositor = (wl_compositor*) wl_registry_bind(registry, name, &wl_compositor_interface, 3);
    if (strcmp(interface, wl_shell_interface.name) == 0)
        state->waylandShell =  (wl_shell*) wl_registry_bind(registry, name, &wl_shell_interface, 1);
    if (strcmp(interface, wl_seat_interface.name) == 0)
        state->waylandSeat = (wl_seat*) wl_registry_bind(registry, name, &wl_seat_interface, 1);
    if (strcmp(interface, wl_shm_interface.name) == 0)
        state->waylandSHM = (wl_shm*) wl_registry_bind(registry, name, &wl_shm_interface, 1);
    if (strcmp(interface, xdg_wm_base_interface.name) == 0)
        state->xdgWMBase = (xdg_wm_base*) wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    if (strcmp(interface, wl_output_interface.name) == 0)
        state->waylandOutput = (wl_output*) wl_registry_bind(registry, name, &wl_output_interface, 4);
    if (strcmp(interface, zxdg_exporter_v2_interface.name) == 0)
        state->xdgExporter = (zxdg_exporter_v2*) wl_registry_bind(registry, name, &zxdg_exporter_v2_interface, 1);
    if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0)
        state->xdgOutputManager = (zxdg_output_manager_v1*) wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 1);
}

void WaylandDisplayWindow::RegisterSeatCapabilities(void *data, wl_seat* seat, uint32_t capabilities)
{
    WaylandClientState* state = (WaylandClientState*)data;
    state->hasKeyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
    state->hasPointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
}

void WaylandDisplayWindow::OnOutputScaleEvent(void* data, wl_output* output, int32_t newScaleFactor)
{
    // We default to 1.0 on scale, but if this event gets sent, we change the scale to the factor provided instead
    WaylandOutputState* outputState = (WaylandOutputState*) data;

    outputState->scaleFactor = (double)newScaleFactor;
}

void WaylandDisplayWindow::OnOutputGeometryEvent(void* data, wl_output* output, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight, int32_t subpixel, const char* make, const char* model, int32_t transform)
{
    WaylandOutputState* outputState = (WaylandOutputState*) data;

    outputState->physicalWidth = physicalWidth;
    outputState->physicalHeight = physicalHeight;
}

void WaylandDisplayWindow::OnOutputModeEvent(void* data, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    // no-op (but might be useful?)
}

void WaylandDisplayWindow::OnOutputDoneEvent(void* data, wl_output* output)
{
    // no-op
}

void WaylandDisplayWindow::OnOutputNameEvent(void* data, wl_output* output, const char* name)
{
    // no-op
}

void WaylandDisplayWindow::OnOutputDescriptionEvent(void* data, wl_output* output, const char* description)
{
    // no-op
}

void WaylandDisplayWindow::OnXDGWMBasePingEvent(void* data, xdg_wm_base* wmBase, uint32_t serial)
{
    xdg_wm_base_pong(wmBase, serial);
}

void WaylandDisplayWindow::OnXDGSurfaceConfigureEvent(void* data, xdg_surface* xdgSurface, uint32_t serial)
{
    xdg_surface_ack_configure(xdgSurface, serial);
}

void WaylandDisplayWindow::OnXDGOutputLogicalPositionEvent(void* data, zxdg_output_v1* xdgOutput, int32_t xPos, int32_t yPos)
{
    XDGOutputPositionInfo* posInfo = (XDGOutputPositionInfo*) data;

    posInfo->xPos = xPos;
    posInfo->yPos = yPos;
}

void WaylandDisplayWindow::OnXDGOutputLogicalSizeEvent(void* data, zxdg_output_v1* xdgOutput, int32_t width, int32_t height)
{
    // no-op (might be useful?)
}

void WaylandDisplayWindow::OnXDGOutputDoneEvent(void* data, zxdg_output_v1* xdgOutput)
{
    // no-op
}

void WaylandDisplayWindow::OnXDGOutputNameEvent(void* data, zxdg_output_v1* output, const char* name)
{
    // no-op
}

void WaylandDisplayWindow::OnXDGOutputDescriptionEvent(void* data, zxdg_output_v1* output, const char* description)
{
    // no-op
}

void WaylandDisplayWindow::OnPointerEnterEvent(void* data, wl_pointer* pointer, uint32_t serial,
                                          wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    WaylandCursorInfo* cursorInfo = (WaylandCursorInfo*) data;

    wl_surface_attach(cursorInfo->cursorSurface, cursorInfo->cursorBuffer, 0, 0);
    wl_surface_damage(cursorInfo->cursorSurface, 0, 0, cursorInfo->cursorImage->width, cursorInfo->cursorImage->height);
    wl_surface_commit(cursorInfo->cursorSurface);
    wl_pointer_set_cursor(pointer, serial, cursorInfo->cursorSurface, 0, 0);
}

void WaylandDisplayWindow::OnXDGExportedHandleEvent(void* data, zxdg_exported_v2* exportedSurface, const char* handleName)
{
    std::string* dataStr = (std::string*) data;

    dataStr->assign(handleName);
}


void WaylandDisplayWindow::CreateBuffers(int32_t width, int32_t height)
{
    if (shared_mem)
        shared_mem.reset();

    shared_mem = std::make_shared<SharedMemHelper>(2 * width * height * 4);
    auto pool = wl_shm_create_pool(clientState.waylandSHM, shared_mem->get_fd(), 2 * width * height * 4);

    m_AppSurfaceBuffer = wl_shm_pool_create_buffer(pool, width * height * 4, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);

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
    zxdg_exported_v2* exportedSurface = nullptr;

    std::string exportHandle;

    zxdg_exported_v2_listener listener = {
        .handle = OnXDGExportedHandleEvent
    };

    zxdg_exported_v2_add_listener(exportedSurface, &listener, &exportHandle);

    // export_toplevel() will cause a handle event to happen
    exportedSurface = zxdg_exporter_v2_export_toplevel(clientState.xdgExporter, m_AppSurface);

    return exportHandle;
}


