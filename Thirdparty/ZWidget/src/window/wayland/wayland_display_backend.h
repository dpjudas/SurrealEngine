#pragma once

#include "window/window.h"
#include "window/ztimer/ztimer.h"

#include <wayland-client.hpp>
#include <wayland-client-protocol-extra.hpp>
#include <wayland-client-protocol-unstable.hpp>
#include <wayland-cursor.hpp>
#include "wl_fractional_scaling_protocol.hpp"
#include <linux/input.h>
#include <map>
#include <xkbcommon/xkbcommon.h>

class WaylandDisplayWindow;

class WaylandDisplayBackend : public DisplayBackend
{
public:
	WaylandDisplayBackend();
    ~WaylandDisplayBackend();

	std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner, RenderAPI renderAPI) override;
	void ProcessEvents() override;
	void RunLoop() override;
	void ExitLoop() override;

	void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
	void StopTimer(void* timerID) override;

	Size GetScreenSize() override;

	bool IsWayland() override { return true; }

	void OnWindowCreated(WaylandDisplayWindow* window);
	void OnWindowDestroyed(WaylandDisplayWindow* window);

	void SetCursor(StandardCursor cursor);
    void ShowCursor(bool enable);
	bool GetKeyState(InputKey key);

#ifdef USE_DBUS
	std::unique_ptr<OpenFileDialog> CreateOpenFileDialog(DisplayWindow* owner) override;
	std::unique_ptr<SaveFileDialog> CreateSaveFileDialog(DisplayWindow* owner) override;
	std::unique_ptr<OpenFolderDialog> CreateOpenFolderDialog(DisplayWindow* owner) override;
#endif

	bool exitRunLoop = false;
	Size s_ScreenSize = Size(0, 0);
	wayland::display_t s_waylandDisplay = wayland::display_t();
	wayland::registry_t s_waylandRegistry;
	std::vector<WaylandDisplayWindow*> s_Windows;
	WaylandDisplayWindow* m_FocusWindow = nullptr;

    wayland::compositor_t m_waylandCompositor;
    wayland::shm_t m_waylandSHM;
    wayland::seat_t m_waylandSeat;
    wayland::output_t m_waylandOutput;
    wayland::data_device_manager_t m_DataDeviceManager;
    wayland::xdg_wm_base_t m_XDGWMBase;
    wayland::zwp_pointer_constraints_v1_t m_PointerConstraints;
    wayland::xdg_activation_v1_t m_XDGActivation;
    wayland::zxdg_decoration_manager_v1_t m_XDGDecorationManager;
    wayland::fractional_scale_manager_v1_t m_FractionalScaleManager;
    wayland::zxdg_output_manager_v1_t m_XDGOutputManager;
    wayland::zxdg_output_v1_t m_XDGOutput;
    wayland::zxdg_exporter_v2_t m_XDGExporter;

    wayland::keyboard_t m_waylandKeyboard;
    wayland::pointer_t m_waylandPointer;

    wayland::cursor_image_t m_cursorImage;
    wayland::surface_t m_cursorSurface;
    wayland::buffer_t m_cursorBuffer;

    std::map<InputKey, bool> inputKeyStates; // True when the key is pressed, false when isn't

private:
    void CheckNeedsUpdate();
	void UpdateTimers();
    void ConnectDeviceEvents();
    void OnKeyboardKeyEvent(xkb_keysym_t xkbKeySym, wayland::keyboard_key_state state);
    void OnKeyboardCharEvent(const char* ch, wayland::keyboard_key_state state);
    void OnKeyboardDelayEnd();
    void OnKeyboardRepeat();
    void OnMouseEnterEvent(uint32_t serial);
    void OnMouseLeaveEvent();
    void OnMousePressEvent(InputKey button);
    void OnMouseReleaseEvent(InputKey button);
    void OnMouseMoveEvent(Point surfacePos);
    void OnMouseWheelEvent(InputKey button);

    InputKey XKBKeySymToInputKey(xkb_keysym_t keySym);
    InputKey LinuxInputEventCodeToInputKey(uint32_t inputCode);

    std::string GetWaylandCursorName(StandardCursor cursor);

    bool hasKeyboard = false;
    bool hasPointer = false;

    ZTimer::TimePoint m_previousTime;
    ZTimer::TimePoint m_currentTime;

    ZTimer m_keyboardDelayTimer;
    ZTimer m_keyboardRepeatTimer;

    InputKey previousKey = {};
    std::string previousChars;

    uint32_t m_KeyboardSerial = 0;

    xkb_context* m_KeymapContext = nullptr;
    xkb_keymap* m_Keymap = nullptr;
    xkb_state* m_KeyboardState = nullptr;
};
