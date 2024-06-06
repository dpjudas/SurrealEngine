#pragma once

#include "wayland_display_backend.h"

#include <stdexcept>
#include <array>
#include <memory>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <random>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include "xdg-shell.h"
#include "xdg-output-unstable.h"
#include "xdg-foreign-unstable-v2.h" // Mainly for sending owner window ID over D-Bus
#include <linux/input.h>

#include "zwidget/window/window.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class SharedMemHelper
{
public:
    SharedMemHelper(size_t size)
        : len(size)
    {
        std::stringstream ss;
        std::random_device device;
        std::default_random_engine engine(device());
        std::uniform_int_distribution<unsigned int> distribution(0, std::numeric_limits<unsigned int>::max());
        ss << distribution(engine);
        name = ss.str();

        fd = memfd_create(name.c_str(), 0);
        if(fd < 0)
            throw std::runtime_error("shm_open failed.");

        if(ftruncate(fd, size) < 0)
            throw std::runtime_error("ftruncate failed.");

        mem = mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(mem == MAP_FAILED) // NOLINT
            throw std::runtime_error("mmap failed.");
    }

    ~SharedMemHelper() noexcept
    {
        if(fd)
        {
            munmap(mem, len);
            close(fd);
            shm_unlink(name.c_str());
        }
    }

    int get_fd() const
    {
        return fd;
    }

    void *get_mem()
    {
        return mem;
    }

private:
    std::string name;
    int fd = 0;
    size_t len = 0;
    void *mem = nullptr;
};

struct WaylandOutputState {
    int32_t physicalWidth = 0;
    int32_t physicalHeight = 0;
    double scaleFactor = 1.0;
};

struct WaylandClientState {
    wl_display* waylandDisplay = nullptr;
    wl_registry* waylandRegistry = nullptr;
    wl_compositor* waylandCompositor = nullptr;
    wl_output* waylandOutput = nullptr;
    wl_shell* waylandShell = nullptr;
    wl_seat* waylandSeat = nullptr;
    wl_shm* waylandSHM = nullptr;
    xdg_wm_base* xdgWMBase = nullptr;
    zxdg_output_manager_v1* xdgOutputManager = nullptr;
    zxdg_exporter_v2* xdgExporter = nullptr;
    bool hasKeyboard = false;
    bool hasPointer = false;
    WaylandOutputState outputState;
};

struct XDGOutputPositionInfo {
    int32_t xPos;
    int32_t yPos;
};

struct WaylandCursorInfo {
    wl_cursor_image* cursorImage;
    wl_buffer* cursorBuffer;
    wl_surface* cursorSurface;
};

class WaylandDisplayWindow : public DisplayWindow
{
public:
    WaylandDisplayWindow(DisplayWindowHost* windowHost, bool popupWindow, WaylandDisplayWindow* owner);
    ~WaylandDisplayWindow();

    void SetWindowTitle(const std::string& text) override;
	void SetWindowFrame(const Rect& box) override;
	void SetClientFrame(const Rect& box) override;
	void Show() override;
	void ShowFullscreen() override;
	void ShowMaximized() override;
	void ShowMinimized() override;
	void ShowNormal() override;
	void Hide() override;
	void Activate() override;
	void ShowCursor(bool enable) override;
	void LockCursor() override;
	void UnlockCursor() override;
	void CaptureMouse() override;
	void ReleaseMouseCapture() override;
	void Update() override;
	bool GetKeyState(InputKey key) override;
	void SetCursor(StandardCursor cursor) override;

	Rect GetWindowFrame() const override;
	Size GetClientSize() const override;
	int GetPixelWidth() const override;
	int GetPixelHeight() const override;
	double GetDpiScale() const override;

	void PresentBitmap(int width, int height, const uint32_t* pixels) override;

	void SetBorderColor(uint32_t bgra8) override;
	void SetCaptionColor(uint32_t bgra8) override;
	void SetCaptionTextColor(uint32_t bgra8) override;

	std::string GetClipboardText() override;
	void SetClipboardText(const std::string& text) override;

	Point MapFromGlobal(const Point& pos) const override;
	Point MapToGlobal(const Point& pos) const override;

    void* GetNativeHandle() override { return (void*)&m_XDGToplevel; }

    static void ProcessEvents();
    static void RunLoop();
    static void ExitLoop();
    static Size GetScreenSize();
private:
    DisplayWindowHost* windowHost = nullptr;
    bool m_PopupWindow = false;

    static bool exitRunLoop;
    static WaylandClientState clientState;

    XDGOutputPositionInfo m_GlobalPosInfo = {0};
    uint32_t m_windowWidth = 0;
    uint32_t m_windowHeight = 0;

    Size m_ScreenSize = Size{0, 0};

    // Local Wayland objects
    wl_surface* m_AppSurface = nullptr;
    wl_shell_surface* m_ShellSurface = nullptr;
    wl_buffer* m_AppSurfaceBuffer = nullptr;
    xdg_surface* m_XDGSurface = nullptr;
    xdg_toplevel* m_XDGToplevel = nullptr;
    xdg_popup* m_XDGPopup = nullptr; // This will be set when m_PopupWindow is true
    wl_keyboard* m_WaylandKeyboard = nullptr;
    wl_pointer* m_WaylandPointer = nullptr;
    zxdg_output_v1* m_XDGOutput = nullptr;

    WaylandCursorInfo m_CursorInfo = {0};

    std::shared_ptr<SharedMemHelper> shared_mem;

    // Wayland event handler functions
    static void RegistryGlobalHandler(void* data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void RegisterSeatCapabilities(void* data, wl_seat* seat, uint32_t capabilities);
    static void OnOutputScaleEvent(void* data, wl_output* output, int32_t newScaleFactor);
    static void OnOutputGeometryEvent(void* data, wl_output* output, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight,
			 int32_t subpixel, const char *make, const char *model,	 int32_t transform);
    static void OnOutputModeEvent(void* data, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
    static void OnOutputDoneEvent(void* data, wl_output* output);
    static void OnOutputNameEvent(void* data, wl_output* output, const char* name);
    static void OnOutputDescriptionEvent(void* data, wl_output* output, const char* description);
    static void OnXDGWMBasePingEvent(void* data, xdg_wm_base* wmBase, uint32_t serial);
    static void OnXDGSurfaceConfigureEvent(void* data, xdg_surface* xdgSurface, uint32_t serial);
    static void OnXDGOutputLogicalPositionEvent(void* data, zxdg_output_v1* xdgOutput, int32_t xPos, int32_t yPos);
    static void OnXDGOutputLogicalSizeEvent(void* data, zxdg_output_v1* xdgOutput, int32_t width, int32_t height);
    static void OnXDGOutputDoneEvent(void* data, zxdg_output_v1* xdgOutput);
    static void OnXDGOutputNameEvent(void* data, zxdg_output_v1* output, const char* name);
    static void OnXDGOutputDescriptionEvent(void* data, zxdg_output_v1* output, const char* description);
    static void OnPointerEnterEvent(void* data, wl_pointer* pointer, uint32_t serial,
                               wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
    static void OnXDGExportedHandleEvent(void* data, zxdg_exported_v2* exportedSurface, const char* handleName);

    // Helper functions
    void CreateBuffers(int32_t width, int32_t height);
    std::string GetWaylandCursorName(StandardCursor cursor);
    std::string GetWaylandWindowID();

    friend WaylandDisplayBackend;
};
