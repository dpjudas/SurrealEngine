#include "wayland_display_backend.h"
#include "wayland_display_window.h"

#ifdef USE_DBUS
#include "window/dbus/dbus_open_file_dialog.h"
#include "window/dbus/dbus_save_file_dialog.h"
#include "window/dbus/dbus_open_folder_dialog.h"
#endif

std::unique_ptr<DisplayWindow> WaylandDisplayBackend::Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner)
{
	return std::make_unique<WaylandDisplayWindow>(windowHost, popupWindow, static_cast<WaylandDisplayWindow*>(owner));
}

void WaylandDisplayBackend::ProcessEvents()
{
	WaylandDisplayWindow::ProcessEvents();
}

void WaylandDisplayBackend::RunLoop()
{
	WaylandDisplayWindow::RunLoop();
}

void WaylandDisplayBackend::ExitLoop()
{
	WaylandDisplayWindow::ExitLoop();
}

Size WaylandDisplayBackend::GetScreenSize()
{
	return WaylandDisplayWindow::GetScreenSize();
}

void* WaylandDisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return WaylandDisplayWindow::StartTimer(timeoutMilliseconds, std::move(onTimer));
}

void WaylandDisplayBackend::StopTimer(void* timerID)
{
	WaylandDisplayWindow::StopTimer(timerID);
}

#ifdef USE_DBUS
std::unique_ptr<OpenFileDialog> WaylandDisplayBackend::CreateOpenFileDialog(DisplayWindow* owner)
{
	std::string ownerHandle;
	if (owner)
		ownerHandle = "wayland:" + static_cast<WaylandDisplayWindow*>(owner)->GetWaylandWindowID();
	return std::make_unique<DBusOpenFileDialog>(ownerHandle);
}

std::unique_ptr<SaveFileDialog> WaylandDisplayBackend::CreateSaveFileDialog(DisplayWindow* owner)
{
	std::string ownerHandle;
	if (owner)
		ownerHandle = "wayland:" + static_cast<WaylandDisplayWindow*>(owner)->GetWaylandWindowID();
	return std::make_unique<DBusSaveFileDialog>(ownerHandle);
}

std::unique_ptr<OpenFolderDialog> WaylandDisplayBackend::CreateOpenFolderDialog(DisplayWindow* owner)
{
	std::string ownerHandle;
	if (owner)
		ownerHandle = "wayland:" + static_cast<WaylandDisplayWindow*>(owner)->GetWaylandWindowID();
	return std::make_unique<DBusOpenFolderDialog>(ownerHandle);
}
#endif
