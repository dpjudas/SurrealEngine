#pragma once

#include "wayland_display_backend.h"

#include <stdexcept>
#include <array>
#include <memory>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <random>
#include <map>
#include <list>

#include <wayland-client.hpp>
#include <wayland-client-protocol-extra.hpp>
#include <wayland-client-protocol-unstable.hpp>
#include <wayland-cursor.hpp>
#include <linux/input.h>

#include <xkbcommon/xkbcommon.h>

#include "zwidget/window/window.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

template <typename R, typename T, typename... Args>
std::function<R(Args...)> bind_mem_fn(R(T::* func)(Args...), T *t)
{
  return [func, t] (Args... args)
    {
      return (t->*func)(args...);
    };
}

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
            throw std::runtime_error("mmap failed with len " + std::to_string(len) + ".");
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
    static void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer);
    static void StopTimer(void* timerID);
private:
    // Event handlers as otherwise linking DisplayWindowHost On...() functions with Wayland events directly crashes the app
    // Alternatively to avoid crashes one can capture by value ([=]) instead of reference ([&])
    void OnXDGToplevelConfigureEvent(int32_t width, int32_t height);
    void OnKeyboardKeyEvent(xkb_keysym_t xkbKeySym, wayland::keyboard_key_state state);
    void OnKeyboardCharEvent(const char* ch);
    void OnMouseEnterEvent(uint32_t serial);
    void OnMouseLeaveEvent();
    void OnMousePressEvent(InputKey button);
    void OnMouseReleaseEvent(InputKey button);
    void OnMouseMoveEvent(Point surfacePos);
    void OnMouseWheelEvent(InputKey button);
    void OnExitEvent();

    void DrawSurface(uint32_t serial = 0);

    DisplayWindowHost* windowHost = nullptr;
    bool m_PopupWindow = false;

    bool m_NeedsUpdate = true;
    static void CheckNeedsUpdate();

    static bool exitRunLoop;

    Point m_WindowGlobalPos = Point(0, 0);
    Size m_WindowSize = Size(0, 0);
    double m_ScaleFactor = 1.0;

    Point m_SurfaceMousePos = Point(0, 0);

    static Size m_ScreenSize;
    static std::list<WaylandDisplayWindow*> s_Windows;
    static std::list<WaylandDisplayWindow*>::iterator s_WindowsIterator;

    static wayland::display_t m_waylandDisplay;
    wayland::registry_t m_waylandRegistry;
    wayland::compositor_t m_waylandCompositor;
    wayland::shm_t m_waylandSHM;
    wayland::seat_t m_waylandSeat;
    wayland::output_t m_waylandOutput;
    wayland::data_device_manager_t m_DataDeviceManager;
    wayland::xdg_wm_base_t m_XDGWMBase;
    wayland::zwp_pointer_constraints_v1_t m_PointerConstraints;
    wayland::xdg_activation_v1_t m_XDGActivation;

    wayland::data_device_t m_DataDevice;
    wayland::data_source_t m_DataSource;

    wayland::zxdg_decoration_manager_v1_t m_XDGDecorationManager;
    wayland::zxdg_toplevel_decoration_v1_t m_XDGToplevelDecoration;

    wayland::surface_t m_AppSurface;
    wayland::buffer_t m_AppSurfaceBuffer;

    wayland::xdg_surface_t m_XDGSurface;
    wayland::xdg_toplevel_t m_XDGToplevel;
    wayland::xdg_popup_t m_XDGPopup;

    bool hasKeyboard = false;
    bool hasPointer = false;

    wayland::keyboard_t m_waylandKeyboard;
    wayland::pointer_t m_waylandPointer;

    uint32_t m_KeyboardSerial = 0;

    xkb_context* m_KeymapContext = nullptr;
    xkb_keymap* m_Keymap = nullptr;
    xkb_state* m_KeyboardState = nullptr;

    std::map<InputKey, bool> inputKeyStates; // True when the key is pressed, false when isn't

    wayland::zxdg_output_manager_v1_t m_XDGOutputManager;
    wayland::zxdg_output_v1_t m_XDGOutput;

    wayland::zxdg_exporter_v2_t m_XDGExporter;
    wayland::zxdg_exported_v2_t m_XDGExported;

    wayland::cursor_image_t m_cursorImage;
    wayland::surface_t m_cursorSurface;
    wayland::buffer_t m_cursorBuffer;

    wayland::zwp_locked_pointer_v1_t m_LockedPointer;

    wayland::callback_t m_FrameCallback;

    std::string m_windowID;
    std::string m_ClipboardContents = "";

    std::shared_ptr<SharedMemHelper> shared_mem;

    // Helper functions
    void CreateBuffers(int32_t width, int32_t height);
    std::string GetWaylandCursorName(StandardCursor cursor);
    std::string GetWaylandWindowID();

    InputKey XKBKeySymToInputKey(xkb_keysym_t keySym);
    InputKey LinuxInputEventCodeToInputKey(uint32_t inputCode);

    friend WaylandDisplayBackend;
};
